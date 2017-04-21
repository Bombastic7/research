#pragma once

#include <map>
#include <string>
#include <cstdio>
#include <queue>
#include <cmath>
#include <vector>

#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/exception.hpp"
#include "util/time.hpp"


namespace mjon661 { namespace algorithm {




	template<typename D, bool OP_Static_Exp_Time>
	class Bugsy_testing {
		public:

		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;

		
		struct Node {
			Cost g, f;
			double u;
			unsigned depth;
			unsigned expdAtGen;
			PackedState pkd;
			Node* parent;
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
				if(a->f != b->f)
					return a->f < b->f;
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
		
		
		
		
		
		
		Bugsy_testing(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mParam_wf			(jConfig.at("wf")),
			mParam_wt			(jConfig.at("wt")),
			mParam_stExpTime	(OP_Static_Exp_Time ? jConfig.at("exptime").get<double>() : 0)
		{}

		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();

			mLog_expd = mLog_gend = mLog_dups = mLog_reopnd = 0;
			
			mGoalNode = nullptr;		

			mLog_curDelay = 0;
			mLog_nextDelayAcc = 0;
			mResort_n = 0;
			mResort_next = 16;

			if(OP_Static_Exp_Time)
				mLog_curExpTime = mParam_stExpTime;
			else
				mLog_curExpTime = 0;
			
			mLog_searchTimer.start();
			mLog_resortTimer.start();
			
			mLog_curDistFact = mParam_wt * mLog_curDelay * mLog_curExpTime;
			
			mLog_pastDelays.clear();
			mLog_pastExpTimes.clear();
			
			mLog_pastExpTimes.push_back(mLog_curExpTime);
			mLog_pastDelays.push_back(mLog_curDelay);
			
			mLog_rollingAvgDelay.reset();
			mLog_pastRollingAvgDelay_mean.clear();
			mLog_pastRollingAvgDelay_stddev.clear();
			
			mTest_exp_g.clear();
			mTest_exp_f.clear();
			mTest_exp_depth.clear();
			mTest_exp_u.clear();
		}
		
		void execute(State const& s0) {
			doSearch(s0);
		}
		
		void doSearch(State const& s0) {
			reset();
			{
				Node* n0 = mNodePool.construct();

				n0->g = 		Cost(0);
				n0->depth =		0;
				n0->parent = 	nullptr;
				
				evalHr(n0, s0);

				mDomain.packState(s0, n0->pkd);
				
				mOpenList.push(n0);
				mClosedList.add(n0);
			}
			

			while(true) {
				if(mLog_expd % 10000 == 0 && debugCheckMemLimit())
					throw NoSolutionException("memlimit");
				
				Node* n = nullptr;
				try {
					n = mOpenList.pop();
				}
				catch(AssertException const& e) {
					throw NoSolutionException("");
				}
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
			mLog_searchTimer.stop();
		}
		
		void doResort() {
			unsigned expThisPhase = mResort_n == 0 ? 16 : mLog_expd / 2;

			mLog_curDelay = (double)mLog_nextDelayAcc / expThisPhase;
			mLog_nextDelayAcc = 0;

			if(!OP_Static_Exp_Time) {
				mLog_resortTimer.stop();
				mLog_resortTimer.start();
				mLog_curExpTime = mLog_resortTimer.cpuSeconds() / expThisPhase;
			}

			mLog_curDistFact = mParam_wt * mLog_curDelay * mLog_curExpTime;

			for(unsigned i=0; i<mOpenList.size(); i++) {
				State s;
				Node* n = mOpenList.at(i);
				mDomain.unpackState(s, n->pkd);
				evalHr(n, s);
			}
			mOpenList.reinit();
			
			mResort_n++;
			mResort_next *= 2;
			
			mLog_pastExpTimes.push_back(mLog_curExpTime);
			mLog_pastDelays.push_back(mLog_curDelay);
		}
		
		
		Json report() {
			Json j;
			j["expd"] = mLog_expd;
			j["gend"] = mLog_gend;
			j["reopnd"] = mLog_reopnd;
			j["dups"] = mLog_dups;
			j["wf"] = mParam_wf;
			j["wt"] = mParam_wt;
			
			if(OP_Static_Exp_Time)
				j["fixed_exp_time"] = mParam_stExpTime;
			
			j["wall_time"] = mLog_searchTimer.wallSeconds();
			j["cpu_time"] = mLog_searchTimer.cpuSeconds();
			
			fast_assert(mGoalNode);
			
			j["goal_g"] = mGoalNode->g;
			j["goal_f"] = mGoalNode->f;
			
			unsigned goal_depth = 0;
			for(Node* m = mGoalNode->parent; m; m=m->parent) {
				goal_depth++;
			}
			
			j["goal_depth"] = goal_depth;

			j["past_delays"] = mLog_pastDelays;
			j["past_exptimes"] = mLog_pastExpTimes;
			
			j["past_rollingavgdelays_mean"] = mLog_pastRollingAvgDelay_mean;
			j["past_rollingavgdelays_stddev"] = mLog_pastRollingAvgDelay_stddev;
			
			j["exp_f"] = mTest_exp_f;
			j["exp_g"] = mTest_exp_g;
			j["exp_u"] = mTest_exp_u;
			j["exp_depth"] = mTest_exp_depth;
			return j;
		}
		
		
		//private:
		

		
		void expand(Node* n, State& s) {
			mLog_expd++;
			
			mTest_exp_f.push_back(n->f);
			mTest_exp_g.push_back(n->g);
			mTest_exp_u.push_back(n->u);
			mTest_exp_depth.push_back(n->depth);
			
			if(mLog_expd % 1000 == 0) {
				mLog_pastRollingAvgDelay_mean.push_back(mLog_rollingAvgDelay.get_mean());
				mLog_pastRollingAvgDelay_stddev.push_back(mLog_rollingAvgDelay.get_stddev());
			}
			
			mLog_rollingAvgDelay.put(mLog_expd - n->expdAtGen);
			
			
			//logDebugStream() << mLog_expd << ": " << n->u << "\n";
			
			slow_assert(n->expdAtGen < mLog_expd);
			mLog_nextDelayAcc += mLog_expd - n->expdAtGen;
			
			typename Domain::AdjEdgeIterator edgeIt = mDomain.getAdjEdges(s);
			
			for(; !edgeIt.finished(); edgeIt.next()) {
				
				PackedState kid_pkd;
				mDomain.packState(edgeIt.state(), kid_pkd);
								
				if(n->pkd == kid_pkd)
					continue;
				
				mLog_gend++;
				
				Cost kid_g = n->g + edgeIt.cost();
				unsigned kid_depth = n->depth + 1;
				
				Node* kid_dup = mClosedList.find(kid_pkd);

				if(kid_dup) {
					mLog_dups++;
					if(kid_dup->g > kid_g) {
						kid_dup->g			= kid_g;
						kid_dup->depth		= kid_depth;
						kid_dup->parent		= n;
						kid_dup->expdAtGen	= mLog_expd;
						
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
					kid_node->depth		= kid_depth;
					kid_node->pkd 		= kid_pkd;
					kid_node->parent	= n;
					kid_node->expdAtGen	= mLog_expd;
					
					evalHr(kid_node, edgeIt.state());

					mOpenList.push(kid_node);
					mClosedList.add(kid_node);
				}
			}
		}
		

		void evalHr(Node* n, State const& s) {
			n->f = n->g + mDomain.costHeuristic(s);			
			n->u = mParam_wf * n->f + mLog_curDistFact * mDomain.distanceHeuristic(s);
			//n->u = mParam_wf * n->f + mParam_wt * mLog_curExpTime * mLog_rollingAvgDelay.get_mean() * mDomain.distanceHeuristic(s);
		}
		
		
		template<unsigned N>
		struct RollingAvgDelay {
			
			RollingAvgDelay() {
				reset();
			}
			
			void reset() {
				mVals.fill(0);
				mPos = 0;
				mAcc = 0;
			}
			
			void put(unsigned v) {
				mAcc -= mVals[mPos];
				mAcc += v;
				mVals[mPos] = v;
				mPos++;
				mPos %= N;
			}
			
			double get_mean() const {
				return (double)mAcc / N;
			}
			
			double get_stddev() const {
				double acc = 0;
				const double mean = get_mean();
				
				for(unsigned i=0; i<N; i++) {
					acc += std::pow((double)mVals[i] - mean, 2);
				}
				
				return std::sqrt(acc / (N-1));
			}
			
			std::array<unsigned, N> mVals;
			unsigned mPos, mAcc;
		};
	
		
		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		const double mParam_wf, mParam_wt, mParam_stExpTime;
		
		Node* mGoalNode;
		
		unsigned mLog_expd, mLog_gend, mLog_dups, mLog_reopnd;
		
		double mLog_curDistFact, mLog_curExpTime, mLog_curDelay;
		unsigned mResort_n, mResort_next, mLog_nextDelayAcc;
		
		std::vector<double> mLog_pastDelays, mLog_pastExpTimes;
		
		Timer mLog_resortTimer, mLog_searchTimer;
		
		RollingAvgDelay<1000> mLog_rollingAvgDelay;
		std::vector<double> mLog_pastRollingAvgDelay_mean;
		std::vector<double> mLog_pastRollingAvgDelay_stddev;
		
		std::vector<double> mTest_exp_g, mTest_exp_f, mTest_exp_depth, mTest_exp_u;
	};
}}
