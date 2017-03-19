#pragma once

#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/exception.hpp"
#include "util/time.hpp"

#include "search/admissible_abtsearch.hpp"
#include "search/admissible_abtsearch_util.hpp"


namespace mjon661 { namespace algorithm {

	enum struct BugsySearchMode {
		Delay, BranchingFactor
	};
	
	std::string BugsySearchModeStr(BugsySearchMode m) {
		if(m == BugsySearchMode::Delay)
			return "Delay";
		else if(m == BugsySearchMode::BranchingFactor)
			return "BranchingFactor";
		
		gen_assert(false);
		return "";
	}
	
	
	template<typename, BugsySearchMode>
	struct CompRemExpansions;
	
	
	
	template<typename D>
	struct CompRemExpansions<D, BugsySearchMode::Delay> {
		
		
		void reset() {
			mLog_nextExpDelayAcc = 0;
			mLog_curExpDelay = 1;
			mLog_pastExpDelays.clear();
			mLog_pastExpDelays.push_back(mLog_curExpDelay);
		}
		
		template<typename Node>
		void informExpansion(Node* n, unsigned pExpDelay) {
			mLog_nextExpDelayAcc += pExpDelay;
		}
		
		void update(unsigned pExpSinceLast) {
			mLog_curExpDelay = (double)mLog_nextExpDelayAcc / pExpSinceLast;
			mLog_nextExpDelayAcc = 0;
			mLog_pastExpDelays.push_back(mLog_curExpDelay);
		}
		
		template<typename Cost>
		double eval(Cost d) {
			return d * mLog_curExpDelay;
		}
		
		Json report() {
			Json j;
			j["past_delays"] = mLog_pastExpDelays;
			return j;
		}

		unsigned mLog_nextExpDelayAcc;
		double mLog_curExpDelay;
		
		std::vector<double> mLog_pastExpDelays;
	};
	
	
	
	template<typename D>
	struct CompRemExpansions<D, BugsySearchMode::BranchingFactor> {
		
		using Cost = typename D::template Domain<0>::Cost;
		
		void reset() {
			mLog_fexp.clear();
			mLog_curBF = 1;
			mLog_pastBFs.clear();
			mLog_pastBFs.push_back(mLog_curBF);
		}
		
		template<typename Node>
		void informExpansion(Node* n, unsigned) {
			mLog_fexp[n->f]++;
		}
		
		void update(unsigned pExpSinceLast) {
			logDebug("update bf start");//.........
			logDebug(std::to_string(mLog_fexp.size()));
			if(mLog_fexp.size() < 2)
				return;
		
			std::vector<double> bfsamples;
						
			auto it = mLog_fexp.begin(), itprev = mLog_fexp.begin();
			++it;
			
			for(; it!=mLog_fexp.end(); ++it, ++itprev)
				bfsamples.push_back((double)it->second/itprev->second);
			
			std::sort(bfsamples.begin(), bfsamples.end());
			
			double acc = 0;
			unsigned nused = 0;
			
			if(bfsamples.size() >= 3) {
				for(unsigned i=1; i<bfsamples.size()-1; i++)
					acc += bfsamples[i];
				nused = bfsamples.size()-2;
			}
			else if(bfsamples.size() == 2) {
				acc = bfsamples[0] + bfsamples[1];
				nused = 2;
			}
			else {
				acc = bfsamples[0];
				nused = 1;
			}
			
			acc /= nused;
			
			if(acc > 5)
				acc = 5;
			
			mLog_curBF = acc;
			mLog_pastBFs.push_back(mLog_curBF);
			logDebugStream() << "update. new bf=" << mLog_curBF << "\n";
		}

		double eval(Cost d) {
			return std::pow(mLog_curBF, d);
		}
		
		Json report() {
			Json j;
			j["used_bf"] = mLog_pastBFs;
			return j;
		}
		

		std::map<Cost, unsigned> mLog_fexp;
		double mLog_curBF;
		
		std::vector<double> mLog_pastBFs;
	};
	


	template<typename D, bool Use_Exp_Time, BugsySearchMode Search_Mode>
	class BugsyImpl {
		public:

		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		

		struct Node {
			PackedState pkd;
			Cost g;
			double u;
			Cost f; //......
			
			Node *parent;
			
			unsigned expdGen;
		};
		


		struct ClosedOps {
			ClosedOps(Domain const& pDomain) :
				mDomain(pDomain)
			{}
			
			bool operator()(Node * const n, PackedState const& pkd) const {
				return mDomain.compare(n->pkd, pkd);
			}
			
			size_t operator()(Node * const n) const {
				return mDomain.hash(n->pkd);
			}
			
			size_t operator()(PackedState const& pkd) const {
				return mDomain.hash(pkd);
			}
			
			private:
			Domain const& mDomain;
		};
		
		struct OpenOps {
			bool operator()(Node * const a, Node * const b) const {
				if(a->u != b->u)
					return a->u < b->u;
				return a->g > b->g;
			}
		};
		
		
		using OpenList_t = OpenList<Node, Node, OpenOps>;
		
		using ClosedList_t = ClosedList<Node, 
										typename OpenList_t::Wrapped_t, 
										PackedState, 
										ClosedOps,
										ClosedOps,
										Domain::Is_Perfect_Hash>;
									  
		using NodePool_t = NodePool<Node, typename ClosedList_t::Wrapped_t>;
		
		
		
		
		
		
		BugsyImpl(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mParams_wf			(jConfig.at("wf")),
			mParams_wt			(jConfig.at("wt"))
		{}

		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mCompRemExp.reset();

			mLog_expd = mLog_gend = mLog_dups = mLog_reopnd = 0;
			mLog_curExpTime = Use_Exp_Time ? 0 : 1;
			mLog_curDistFact = mLog_curExpTime * mParams_wt;
			
			mGoalNode = nullptr;
			
			mResort_next = 16;
			mResort_n = 0;
			mTimer.start();
		}

		
		void execute(State const& s0) {
			doSearch(s0);
		}
		
		void doSearch(State const& s0) {
			reset();
			{
				Node* n0 = mNodePool.construct();

				n0->g = 		Cost(0);
				n0->parent = 	nullptr;
				n0->expdGen =	0;
				
				evalHr(n0, s0);

				mDomain.packState(s0, n0->pkd);
				
				mOpenList.push(n0);
				mClosedList.add(n0);
			}			
			

			while(true) {
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s)) {
					mGoalNode = n;
					break;
				}
				
				expand(n, s);
				
				if(mLog_expd == mResort_next)
					doResort();
			}
		}
		
		
		Json report() {
			Json j;
			j["expd"] = mLog_expd;
			j["gend"] = mLog_gend;
			j["reopnd"] = mLog_reopnd;
			j["dups"] = mLog_dups;
			j["use_exp_time"] = Use_Exp_Time;
			j["resort_next"] = mResort_next;
			j["resort_n"] = mResort_n;
			j["curExpTime"] = mLog_curExpTime;
			j["wf"] = mParams_wf;
			j["wt"] = mParams_wt;
			j["comp_remexp"] = mCompRemExp.report();
			
			if(mGoalNode) {
				j["goal_g"] = mGoalNode->g;
				j["goal_f"] = mGoalNode->f;
				
				unsigned goal_depth = 0;
				for(Node* m = mGoalNode->parent; m; m=m->parent) {
					goal_depth++;
				}
				
				j["goal_depth"] = goal_depth;
			}
			
			std::vector<Cost> flevel_cost;
			std::vector<unsigned> flevel_n;
			std::vector<double> flevel_bf;

			for(auto it = mTest_exp_f.begin(); it!=mTest_exp_f.end(); ++it) {
				flevel_cost.push_back(it->first);
				flevel_n.push_back(it->second);
			}
			

			if(mTest_exp_f.size() >= 1) {
				auto it = mTest_exp_f.begin(), itprev = mTest_exp_f.begin();

				++it;
					
				for(; it!=mTest_exp_f.end(); ++it, ++itprev)
					flevel_bf.push_back((double)it->second / itprev->second);
				
				fast_assert(flevel_bf.size() + 1 == mTest_exp_f.size());
			}

			j["f_exp_n"] = flevel_n;
			j["f_exp_cost"] = flevel_cost;
			j["f_exp_bf"] = flevel_bf;
			
			return j;
		}

		
		void expand(Node* n, State& s) {
			mLog_expd++;
			mTest_exp_f[n->f]++;
			informExpansion(n);
			
			typename Domain::AdjEdgeIterator edgeIt = mDomain.getAdjEdges(s);
			
			for(; !edgeIt.finished(); edgeIt.next()) {
				
				PackedState kid_pkd;
				mDomain.packState(edgeIt.state(), kid_pkd);
								
				if(n->pkd == kid_pkd)
					continue;
				
				mLog_gend++;
				
				Cost kid_g = n->g + edgeIt.cost();

				Node* kid_dup = mClosedList.find(kid_pkd);

				if(kid_dup) {
					mLog_dups++;
					if(kid_dup->g > kid_g) {
						kid_dup->g			= kid_g;
						kid_dup->parent		= n;
						kid_dup->expdGen	= mLog_expd;
						
						evalHr(kid_dup, edgeIt.state());
						
						if(!mOpenList.contains(kid_dup)) {
							mLog_reopnd++;
						}

						mOpenList.pushOrUpdate(kid_dup);
					}
				}
				else {
					Node* kid_node 		= mNodePool.construct();

					kid_node->g 		= kid_g;
					kid_node->pkd 		= kid_pkd;
					kid_node->parent	= n;
					kid_node->expdGen	= mLog_expd;
					
					evalHr(kid_node, edgeIt.state());

					mOpenList.push(kid_node);
					mClosedList.add(kid_node);
				}
			}
		}
		

		void evalHr(Node* n, State const& s) {
			std::pair<Cost, Cost> hrvals = mDomain.pairHeuristics(s);

			n->f = n->g + hrvals.first;
			n->u = mParams_wf * n->f + mLog_curDistFact * mCompRemExp.eval(hrvals.second);
		}
		
		void informExpansion(Node* n) {
			mCompRemExp.informExpansion(n, mLog_expd - n->expdGen);
		}
		
		void doResort() {
			unsigned expThisPhase = mResort_n == 0 ? 16 : mLog_expd / 2;
			
			if(Use_Exp_Time) {
				mTimer.stop();
				mTimer.start();
				mLog_curExpTime = mTimer.seconds() / expThisPhase;
			}
			
			logDebug(std::to_string(expThisPhase));//.......
			mCompRemExp.update(expThisPhase);
			
			for(unsigned i=0; i<mOpenList.size(); i++) {
				State s;
				Node* n = mOpenList.at(i);
				mDomain.unpackState(s, n->pkd);
				evalHr(n, s);
			}
			mOpenList.reinit();
			
			mResort_n++;
			mResort_next *= 2;
			
			mLog_curDistFact = mLog_curExpTime * mParams_wt;
		}
		

		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		Node* mGoalNode;
		Timer mTimer; //Should this be walltime or cputime ??
		CompRemExpansions<D, Search_Mode> mCompRemExp;
		
		const double mParams_wf, mParams_wt;
		
		unsigned mLog_expd, mLog_gend, mLog_dups, mLog_reopnd;

		unsigned mResort_next, mResort_n;
		
		double mLog_curExpTime, mLog_curDistFact;
		
		std::map<Cost, unsigned> mTest_exp_f;
	};
}}
