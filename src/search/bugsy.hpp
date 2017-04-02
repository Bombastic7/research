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



namespace mjon661 { namespace algorithm { namespace bugsy {


	

	struct C_RollingBf {
		enum struct E_TgtProp {
			depth, f, uRound, n
		};
		static std::string Str_TgtProp(E_TgtProp i) {return std::vector<std::string>{"depth", "f", "uRound"}.at((unsigned)i);}
		
		enum struct E_KeepCounts {
			keepcounts, dropcounts, n
		};
		static std::string Str_KeepCounts(E_KeepCounts i) {return std::vector<std::string>{"keepcounts", "dropcounts"}.at((unsigned)i);}
		
		enum struct E_PruneOutliers {
			prune, nopr, n
		};
		static std::string Str_PruneOutliers(E_PruneOutliers i) {return std::vector<std::string>{"prune", "nopr"}.at((unsigned)i);}
		
		enum struct E_Kfactor {
			none, openlistsz, n
		};
		static std::string Str_Kfactor(E_Kfactor i) {return std::vector<std::string>{"nok", "openlistsz"}.at((unsigned)i);}
		
		enum struct E_EvalProp {
			dist, distAndDepth, dist_powsum, distAndDepth_powsum, exponentialDelay_powsum, n
		};
		static std::string Str_EvalProp(E_EvalProp i) {
			return std::vector<std::string>{"dist", "distAndDepth", "dist_powsum", "distAndDepth_powsum", "exponentialDelay_powsum"}.at((unsigned)i);
		}

		enum E_BfAvgMethod {
			bfArMean, bfGeoMean, n
		};
		static std::string Str_BfAvgMethod(E_BfAvgMethod i) {
			return std::vector<std::string>{"bfArMean", "bfGeoMean"}.at((unsigned)i);
		}
		
		
		static std::string niceNameStr(E_TgtProp tgtprop, E_KeepCounts keepcounts, E_PruneOutliers prout, E_Kfactor kfact, E_EvalProp evalprop, E_BfAvgMethod avgm) {
			return Str_TgtProp(tgtprop) + "_" + Str_KeepCounts(keepcounts) + "_" + Str_PruneOutliers(prout) + "_" + Str_Kfactor(kfact) + "_" + Str_EvalProp(evalprop) + "_" + Str_BfAvgMethod(avgm);
		}
		
		static std::string niceNameStr(unsigned v0, unsigned v1, unsigned v2, unsigned v3, unsigned v4, unsigned v5) {
			return niceNameStr((E_TgtProp)v0, (E_KeepCounts)v1, (E_PruneOutliers)v2, (E_Kfactor)v3, (E_EvalProp)v4, (E_BfAvgMethod)v5);
		}
	
	};




	

	template<typename D>
	struct CompRemExp_delay {
		using Cost = typename D::template Domain<0>::Cost;
		
		void reset() {
			mLog_nextExpDelayAcc = 0;
			mLog_curExpDelay = 1;
			mLog_pastExpDelays.clear();
			mLog_pastExpDelays.push_back(mLog_curExpDelay);
			mExpSinceLast = 0;
		}

		template<typename Node>
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

		template<typename Node>
		double eval(Node* n, Cost h, Cost d) {
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
				C_RollingBf::E_TgtProp OP_Tgt_Prop,
				C_RollingBf::E_KeepCounts OP_Keep_Counts,
				C_RollingBf::E_PruneOutliers OP_Prune_Outliers,
				C_RollingBf::E_Kfactor OP_K_Factor,
				C_RollingBf::E_EvalProp OP_Eval_Prop,
				C_RollingBf::E_BfAvgMethod OP_BFAvgMethod>
	struct CompRemExp_rollingBf {
		using Cost = typename D::template Domain<0>::Cost;
		
		static std::string niceOptionsStr() {
			return C_RollingBf::niceNameStr(OP_Tgt_Prop, OP_Keep_Counts, OP_Prune_Outliers, OP_K_Factor, OP_Eval_Prop, OP_BFAvgMethod);
		}
		
		void reset() {
			mAvgBf = 1;
			mExpCountMap.clear();
			mLog_pastBf.push_back(mAvgBf);
			mK = 1;
			mAvgDelay = 0;
			mDelayAcc = 0;
			mExpansionsThisPhase = 0;
			mPowSumCache.clear();
			mPowSumCache.push_back(1);
		}

		template<typename Node>
		void informExpansion(Node* n, unsigned pExpDelay) {
			Cost k;
			if(OP_Tgt_Prop == C_RollingBf::E_TgtProp::f)
				k = n->f;
			else if(OP_Tgt_Prop == C_RollingBf::E_TgtProp::depth)
				k = n->depth;
			else if(OP_Tgt_Prop == C_RollingBf::E_TgtProp::uRound) {
				k = std::round(n->u);
			}
			else
				gen_assert(false);

			mExpCountMap[k] += 1;
			mExpansionsThisPhase++;
			mDelayAcc += pExpDelay;
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
			
			if(OP_Prune_Outliers ==C_RollingBf::E_PruneOutliers::prune && bfsamples.size() >= 3) {
				std::sort(bfsamples.begin(), bfsamples.end());
				bfsamples.erase(bfsamples.begin());
				bfsamples.pop_back();
				fast_assert(bfsamples.size() >= 1);
			}
			
			double acc = 0;
			
			for(auto& i : bfsamples) {
				if(OP_BFAvgMethod == C_RollingBf::E_BfAvgMethod::bfArMean)
					acc += i;
				else if(OP_BFAvgMethod == C_RollingBf::E_BfAvgMethod::bfGeoMean)
					acc *= i;
				else
					gen_assert(false);
			}
			
			if(OP_BFAvgMethod == C_RollingBf::E_BfAvgMethod::bfArMean)
				mAvgBf = acc / bfsamples.size();
			else if(OP_BFAvgMethod == C_RollingBf::E_BfAvgMethod::bfGeoMean)
				mAvgBf = std::pow(acc, 1.0/ bfsamples.size());

			mLog_pastBf.push_back(mAvgBf);
			
			if(OP_Keep_Counts == C_RollingBf::E_KeepCounts::dropcounts)
				mExpCountMap.clear();
			
			if(OP_K_Factor == C_RollingBf::E_Kfactor::openlistsz) {
				mK = pAlg.mOpenList.size();
			}
			
			mAvgDelay = (double)mDelayAcc / mExpansionsThisPhase;
			mExpansionsThisPhase = 0;
			mPowSumCache.clear();
			mPowSumCache.push_back(1);
		}
		
		template<typename Node>
		double eval(Node* n, Cost h, Cost d) {
			if(OP_Eval_Prop == C_RollingBf::E_EvalProp::dist)
				return mK * std::pow(mAvgBf, d);
			else if(OP_Eval_Prop == C_RollingBf::E_EvalProp::distAndDepth)
				return mK * std::pow(mAvgBf, n->depth + d);
			
			else if(OP_Eval_Prop == C_RollingBf::E_EvalProp::dist_powsum)
				return mK * countSumExpansions(n->depth, d);
			
			else if(OP_Eval_Prop == C_RollingBf::E_EvalProp::distAndDepth_powsum)
				return mK * countSumExpansions(0, n->depth+d);

			else if(OP_Eval_Prop == C_RollingBf::E_EvalProp::exponentialDelay_powsum)
				return mAvgDelay * powsum(d);

			else
				gen_assert(false);
			return 0;
		}
		
		double powsum(unsigned n) {
			//return:
			//0 -> 0
			//1 -> 1
			//2 -> 1 + bf
			//3 -> 1 + bf + bf**2
			//4 -> 1 + bf + bf**2 + bf**3
			//...
			
			if(n == 0)
				return 0;
			
			while(mPowSumCache.size() < n) {
				unsigned lastValid = mPowSumCache.size()-1;
				mPowSumCache.resize(mPowSumCache.size()*2);
				
				for(unsigned i=lastValid+1; i<mPowSumCache.size(); i++)
					mPowSumCache[i] = mPowSumCache[i-1] + std::pow(mAvgBf, i);
			}
			
			return mPowSumCache[n-1];
		}
		

		
		double countSumExpansions(unsigned n, unsigned g) {
			//for each level that must be passed through from n (not including n) to g (not including g), count proportion of 
			// nodes to however many there are at n.
			
			//e.g. (n,g) -> bf**(1) + bf**(2) + ... + bf**(g-n-1)
			//levels not counted are bf**(0) = 1 = [level of n, already partially explored], and 
			//	bf**(g-n) = [level of g, which will only be partially explored].
			
			slow_assert(n <= g);
			
			if(g-n <= 1)
				//There are no fully explored levels to consider.
				return 0;
			
			unsigned levelsPassed = g-n-1;
			
			while(mPowSumCache.size() < levelsPassed+1) {
				unsigned lastValid = mPowSumCache.size()-1;
				mPowSumCache.resize(mPowSumCache.size()*2);
				
				for(unsigned i=lastValid+1; i<mPowSumCache.size(); i++)
					mPowSumCache[i] = mPowSumCache[i-1] + std::pow(mAvgBf, i);
			}
			
			return mPowSumCache[levelsPassed] - 1;
		}
		


		
		Json report() {
			Json j;
			j["method"] = "RollingBf";
			j["past_bf"] = mLog_pastBf;
			return j;
		}
		
		double mAvgBf, mK;
		double mAvgDelay;
		unsigned mDelayAcc;
		unsigned mExpansionsThisPhase;
		
		std::vector<double> mExpoBfCache;
		
		std::map<Cost, unsigned> mExpCountMap;
		std::vector<double> mLog_pastBf;
		
		std::vector<double> mPowSumCache;
	};



//avg remexp accuracy


	template<typename D, bool Use_Fixed_Exp_Time, typename CompRemExp_t>
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
								
				if(mLog_expd == mParams_expdLimit)
					throw NoSolutionException("expd_limit");
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
			
			fast_assert(mGoalNode);
			
			j["goal_g"] = mGoalNode->g;
			j["goal_f"] = mGoalNode->f;
			
			unsigned goal_depth = 0;
			for(Node* m = mGoalNode->parent; m; m=m->parent) {
				goal_depth++;
			}
			
			j["goal_depth"] = goal_depth;
			
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
			n->u = mParams_wf * n->f + mLog_curDistFact * mCompRemExp.eval(n, hrvals.first, hrvals.second);
			n->remexp = mCompRemExp.eval(n, hrvals.first, hrvals.second);
		}
		
		void informExpansion(Node* n) {
			mCompRemExp.informExpansion(n, mLog_expd - n->expdGen);
		}
		
		void doResort() {
			unsigned expThisPhase = mResort_n == 0 ? 16 : mLog_expd / 2;
			
			if(!Use_Fixed_Exp_Time) {
				mTimer.stop();
				mTimer.start();
				mLog_curExpTime = mTimer.cpuSeconds() / expThisPhase;
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
		

		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		
		Node* mGoalNode;
		Timer mTimer;
		CompRemExp_t mCompRemExp;
		
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
}}}
