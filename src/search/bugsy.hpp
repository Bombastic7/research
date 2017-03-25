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

#include "search/hr_module.hpp"



namespace mjon661 { namespace algorithm {


	struct BugsyConstants {
		enum {
			Delay=0, RollingBf=1
		};
		
		enum {
			depth=0, f=1, uRound=2,
		};
		
		
		static std::string optionStr(std::vector<unsigned> const& ops) {
			if(ops.size() == 1) {
				fast_assert(ops[0] == Delay);
				return "Delay";
			}
			
			if(ops.size() == 5) {
				fast_assert(ops[0] == RollingBf);
				std::string s = "RollingBf_";
				s += ops[1] == depth ? "depth_" : (ops[1] == f ? "f_" : (ops[1] == uRound ? "uRound_" : "error"));
				s += ops[2] == 0 ? "dropcounts_" : ( ops[2] == 1 ? "keepcounts_" : "error");
				s += ops[3] == 0 ? "nopr_" : (ops[3] == 1 ? "pr_" : "error");
				s += ops[4] == 0 ? "nok" : (ops[4] == 1 ? "kopensz" : "error");
				return s;
			}
			
			
			gen_assert(false);
			return "";
		}
		
	};


	template<typename D, typename Node, unsigned... Vp>
	struct CompRemExp;
	
	
	
	

	template<typename D, typename Node>
	struct CompRemExp<D, Node, BugsyConstants::Delay> {
		using Cost = typename D::template Domain<0>::Cost;
		
		void reset() {
			mLog_nextExpDelayAcc = 0;
			mLog_curExpDelay = 1;
			mLog_pastExpDelays.clear();
			mLog_pastExpDelays.push_back(mLog_curExpDelay);
			mExpSinceLast = 0;
		}

		void informExpansion(Node* n, unsigned pExpDelay) {
			mLog_nextExpDelayAcc += pExpDelay;
			mExpSinceLast++;
		}
		
		template<typename Alg_t>
		void update(Alg_t const&) {
			mLog_curExpDelay = (double)mLog_nextExpDelayAcc / mExpSinceLast;
			mLog_nextExpDelayAcc = 0;
			mLog_pastExpDelays.push_back(mLog_curExpDelay);
			mExpSinceLast = 0;
		}

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
		unsigned mExpSinceLast;
		
		std::vector<double> mLog_pastExpDelays;
	};
	
	
	
	

	template<	typename D, 
				typename Node, 
				unsigned Use_Prop, 
				unsigned Do_Keep_Counts, 
				unsigned Do_Prune_Outliers,
				unsigned Use_Open_List_Size>
	struct CompRemExp<D, Node, BugsyConstants::RollingBf, Use_Prop, Do_Keep_Counts, Do_Prune_Outliers, Use_Open_List_Size> {
		using Cost = typename D::template Domain<0>::Cost;
		
		void reset() {
			mAvgBf = 1;
			mExpCountMap.clear();
			mLog_pastBf.push_back(mAvgBf);
			mK = 1;
		}

		void informExpansion(Node* n, unsigned pExpDelay) {
			Cost k;
			if(Use_Prop == BugsyConstants::f)
				k = n->f;
			else if(Use_Prop == BugsyConstants::depth)
				k = n->depth;
			else if(Use_Prop == BugsyConstants::uRound) {
				k = std::round(n->u);
			}
			else
				gen_assert(false);

			mExpCountMap[k] += 1;
		}
		
		
		template<typename Alg_t>
		void update(Alg_t const& pAlg) {
			if(mExpCountMap.size() < 2)
				return;

			auto it = mExpCountMap.begin(), itprev = mExpCountMap.begin();
			++it;
			
			std::vector<double> bfsamples;
			
			for(; it!=mExpCountMap.end(); ++it, ++itprev) {
				bfsamples.push_back((double)it->second / itprev->second);
			}
			
			if(Do_Prune_Outliers == 1 && bfsamples.size() >= 3) {
				std::sort(bfsamples.begin(), bfsamples.end());
				bfsamples.erase(bfsamples.begin());
				bfsamples.pop_back();
				fast_assert(bfsamples.size() >= 1);
			}
			
			double acc = 0;
			
			for(auto& i : bfsamples)
				acc += i;
			
			mAvgBf = acc / (mExpCountMap.size() - 1);
			mLog_pastBf.push_back(mAvgBf);
			
			if(Do_Keep_Counts == 0)
				mExpCountMap.clear();
			
			if(Use_Open_List_Size) {
				mK = pAlg.mOpenList.size();
			}
		}
		
		double eval(Cost d) {
			return std::pow(mAvgBf, d);
		}
		
		Json report() {
			Json j;
			j["method"] = "RollingBf";
			j["Do_Keep_Counts"] = Do_Keep_Counts;
			j["past_bf"] = mLog_pastBf;
			return j;
		}
		
		double mAvgBf, mK;
		std::map<Cost, unsigned> mExpCountMap;
		std::vector<double> mLog_pastBf;
	};
	


	
	
	
	


	template<typename D, bool Use_Fixed_Exp_Time, unsigned... Comp_Rem_Exp_Ops>
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
			unsigned depth;
			
			Node *parent;
			
			unsigned expdGen;
			unsigned expdN;
			unsigned remexp;
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
			mParams_wt			(jConfig.at("wt")),
			mParams_fixedExpTime(Use_Fixed_Exp_Time ? jConfig.at("fixed_exptime").get<double>() : 0),
			mParams_expdLimit	(jConfig.count("expd_limit") ? jConfig.at("expd_limit").get<unsigned>() : 0)
		{}

		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mCompRemExp.reset();

			mLog_expd = mLog_gend = mLog_dups = mLog_reopnd = 0;
			
			if(Use_Fixed_Exp_Time)
				mLog_curExpTime = mParams_fixedExpTime;
			else
				mLog_curExpTime = 0;

			mLog_curDistFact = mLog_curExpTime * mParams_wt;
			
			mGoalNode = nullptr;
			
			mResort_next = 16;
			mResort_n = 0;
			
			mTest_exp_f.clear();
			mTest_exp_u.clear();
			mTest_exp_uh.clear();
			mTest_exp_ucorr.clear();
			mTest_exp_depth.clear();
			mTest_exp_delay.clear();
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
				n0->depth =		0;
				n0->expdN =		0;
				
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
					mTest_exp_f.push_back(n->f);
					n->expdN = mLog_expd;
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
			j["use_fixed_exp_time"] = Use_Fixed_Exp_Time;
			j["resort_next"] = mResort_next;
			j["resort_n"] = mResort_n;
			j["curExpTime"] = mLog_curExpTime;
			j["wf"] = mParams_wf;
			j["wt"] = mParams_wt;
			j["fixed_exp_time"] = mParams_fixedExpTime;
			j["expd_limit"] = mParams_expdLimit;
			
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
			
			j["exp_f_raw"] = mTest_exp_f;
			j["exp_u_raw"] = mTest_exp_u;
			j["exp_ucorr_raw"] = mTest_exp_ucorr;
			j["exp_uh_raw"] = mTest_exp_uh;
			j["exp_depth_raw"] = mTest_exp_depth;
			j["exp_delay_raw"] = mTest_exp_delay;
			j["exp_disthr_raw"] = mTest_exp_distHr;
			
			std::map<Cost, unsigned> flevel_exp;
			
			for(auto cst : mTest_exp_f)
				flevel_exp[cst]++;
			
			std::vector<Cost> flevel_cost;
			std::vector<unsigned> flevel_n;
			std::vector<double> flevel_bf;

			for(auto it = flevel_exp.begin(); it!=flevel_exp.end(); ++it) {
				flevel_cost.push_back(it->first);
				flevel_n.push_back(it->second);
			}
			
			if(flevel_n.size() > 1)
				for(unsigned i=0; i<flevel_n.size()-1; i++)
					flevel_bf.push_back((double)flevel_n[i+1] / flevel_n[i]);
			
			j["exp_f"] = flevel_cost;
			j["exp_n"] = flevel_n;
			j["exp_bf"] = flevel_bf;
			
			j["bf_count_map"] = countTrueBf();
			
			std::vector<double> expectedBf, expectedBf_hr;
			
			for(Node* n = mGoalNode; n; n=n->parent) {
				unsigned trueRemExp = mLog_expd - n->expdN;
				unsigned trueDistToGo = mGoalNode->depth - n->depth;
				double bf = std::pow(trueRemExp, 1.0/trueDistToGo);
				expectedBf.push_back(bf);
			}
			for(Node* n = mGoalNode; n; n=n->parent) {
				State s;
				mDomain.unpackState(s, n->pkd);
				unsigned trueRemExp = mLog_expd - n->expdN;
				double bf = std::pow(trueRemExp, 1.0/mDomain.distanceHeuristic(s));
				expectedBf_hr.push_back(bf);
			}
			
			
			
			j["expected_bf"] = expectedBf;
			j["expected_bf_hr"] = expectedBf_hr;
			
			
			
			std::vector<double> absRemExpError, ratRemExpError;
			
			for(Node* n = mGoalNode; n; n=n->parent) {
				unsigned trueRemExp = mLog_expd - n->expdN;
				absRemExpError.push_back((double)trueRemExp - n->remexp);
				ratRemExpError.push_back((double)n->remexp / trueRemExp);
			}
			j["true_remexp_error_abs"] = absRemExpError;
			j["true_remexp_error_rat"] = ratRemExpError;
			
			
			
			return j;
		}

		
		void expand(Node* n, State& s) {
			mLog_expd++;
			//mTest_exp_f.push_back(n->f);
			//mTest_exp_u.push_back(n->u);
			//mTest_exp_uh.push_back(n->u - mParams_wf * n->g);
			//mTest_exp_ucorr.push_back(n->u + mParams_wt * mLog_curExpTime * mLog_expd);
			//mTest_exp_depth.push_back(n->depth);
			//mTest_exp_delay.push_back(mLog_expd - n->expdGen);
			//mTest_exp_distHr.push_back(n->depth + mDomain.distanceHeuristic(s));
			n->expdN = mLog_expd;
			
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
						kid_dup->depth		= n->depth + 1;
						
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
					kid_node->depth		= n->depth + 1;
					
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
			n->remexp = mCompRemExp.eval(hrvals.second);
		}
		
		void informExpansion(Node* n) {
			mCompRemExp.informExpansion(n, mLog_expd - n->expdGen);
		}
		
		void doResort() {
			unsigned expThisPhase = mResort_n == 0 ? 16 : mLog_expd / 2;
			
			if(!Use_Fixed_Exp_Time) {
				mTimer.stop();
				mTimer.start();
				mLog_curExpTime = mTimer.seconds() / expThisPhase;
			}

			mCompRemExp.update(*this);
			
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
		
		
		std::vector<unsigned> countTrueBf() {
			std::map<Node*, unsigned> childCountMap;
			std::map<unsigned, unsigned> bfCountMap;
			
			for(auto it=mClosedList.begin(); it!=mClosedList.end(); ++it) {
				if(mOpenList.contains(*it))
					continue;
				childCountMap[(*it)->parent] += 1;
			}
			
			for(auto it=childCountMap.begin(); it!=childCountMap.end(); ++it) {
				bfCountMap[it->second]++;
			}
			
			std::vector<unsigned> countPairs;
			
			for(auto it=bfCountMap.begin(); it!=bfCountMap.end(); ++it) {
				if(countPairs.size() < it->first+1)
					countPairs.resize(it->first + 1, 0);
				countPairs.at(it->first) = it->second;
			}
			
			return countPairs;
		}
		

		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		
		Node* mGoalNode;
		Timer mTimer; //Should this be walltime or cputime ??
		CompRemExp<D, Node, Comp_Rem_Exp_Ops...> mCompRemExp;
		
		const double mParams_wf, mParams_wt, mParams_fixedExpTime;
		const unsigned mParams_expdLimit;
		
		unsigned mLog_expd, mLog_gend, mLog_dups, mLog_reopnd;

		unsigned mResort_next, mResort_n;
		
		double mLog_curExpTime, mLog_curDistFact;
		
		
		
		std::vector<Cost> mTest_exp_f;
		std::vector<double> mTest_exp_u;
		std::vector<double> mTest_exp_uh;
		std::vector<double> mTest_exp_ucorr;
		std::vector<double> mTest_exp_depth;
		std::vector<double> mTest_exp_delay;
		std::vector<double> mTest_exp_distHr;
	};
}}
