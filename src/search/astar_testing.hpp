#pragma once

#include <limits>
#include <string>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/exception.hpp"
#include "util/math.hpp"

#include "search/admissible_abtsearch.hpp"

#include "search/astar.hpp"


namespace mjon661 { namespace algorithm {

	template<typename D, AstarSearchMode Search_Mode, AstarHrMode Hr_Mode>
	class Astar_testing {
		public:

		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;

		
		
		template<AstarHrMode, typename = void>
		struct HrModule;
		
		template<typename Ign>
		struct HrModule<AstarHrMode::DomainHr, Ign> {
			HrModule(D& pDomStack, Domain const& pDomain, Json const& jConfig) :
				mDomain(pDomain)
			{}
			
			Cost costHeuristic(State const& s) {
				return mDomain.costHeuristic(s);
			}
			
			Cost distanceHeuristic(State const& s) {
				return mDomain.distanceHeuristic(s);
			};
						
			void insertReport(Json& jReport) {}
			
			private:
			Domain const& mDomain;
		};
		
		template<typename Ign>
		struct HrModule<AstarHrMode::AbtHr, Ign> {
			HrModule(D& pDomStack, Domain const& pDomain, Json const& jConfig) :
				mAbtSearch_cost(pDomStack, jConfig),
				mAbtSearch_dist(pDomStack, jConfig)
			{}
			
			Cost costHeuristic(State const& s) {
				return mAbtSearch_cost.getHrVal(s);
			}
			
			Cost distanceHeuristic(State const& s) {
				return mAbtSearch_dist.getHrVal(s);
			};
			
			void insertReport(Json& jReport) {
				Json j;
				j["cost_abt"] = Json();
				mAbtSearch_cost.insertReport(j.at("cost_abt"));
				j["dist_abt"] = Json();
				mAbtSearch_dist.insertReport(j.at("dist_abt"));
				jReport["hrmod"] = j;
			}
			
			private:
			AdmissibleAbtSearch<D,1,D::Top_Abstract_Level+1,true> mAbtSearch_cost;
			AdmissibleAbtSearch<D,1,D::Top_Abstract_Level+1,false> mAbtSearch_dist;
		};
		
		
		struct Node {
			Cost g, f;
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
				if(Search_Mode == AstarSearchMode::Greedy || Search_Mode == AstarSearchMode::Speedy)
					return a->f < b->f;
				else if(Search_Mode == AstarSearchMode::Uninformed)
					return a->g < b->g;
				
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
		
		
		
		
		
		
		Astar_testing(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mHrModule			(pDomStack, mDomain, jConfig),
			mParam_hrWeight		(Search_Mode == AstarSearchMode::Weighted ? jConfig.at("hr_weight").get<double>() : 0)
		{}

		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();

			mLog_expd = mLog_gend = mLog_dups = mLog_reopnd = 0;
			
			mGoalNode = nullptr;
			
			mTest_maxf = 0;
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
				
				evalHr(n0, s0);

				mDomain.packState(s0, n0->pkd);
				
				mOpenList.push(n0);
				mClosedList.add(n0);
			}			
			

			while(true) {
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
					gen_assert(mTest_maxf <= n->f);
					gen_assert(n->g == n->f);
					break;
				}
				
				expand(n, s);
			}
		}
		
		
		Json report() {
			Json j;
			j["expd"] = mLog_expd;
			j["gend"] = mLog_gend;
			j["reopnd"] = mLog_reopnd;
			j["dups"] = mLog_dups;
			j["search_mode"] = astarSearchModeStr(Search_Mode);
			j["hr_mode"] = astarHrModeStr(Hr_Mode);
			
			if(Search_Mode == AstarSearchMode::Weighted)
				j["hr_weight"] = mParam_hrWeight;

			if(mGoalNode) {
				j["goal_g"] = (double)mGoalNode->g;
				j["goal_f"] = (double)mGoalNode->f;
				
				unsigned goal_depth = 0;
				for(Node* m = mGoalNode; m; m=m->parent) {
					goal_depth++;
				}
				goal_depth -= 1;

				j["goal_depth"] = goal_depth;
			}
			
			mHrModule.insertReport(j);
			
			std::vector<double> hr_est;
			
			for(Node* n=mGoalNode; n; n=n->parent) {
				Cost hrval = n->f - n->g;
				Cost costToGo = mGoalNode->g - n->g;
				
				std::cout << hrval << " " << costToGo << "\n";
				
				gen_assert(hrval <= costToGo);
				
				if(costToGo != Cost(0))
					hr_est.push_back((double)hrval / costToGo);
			}
			
			j["hr_est"] = hr_est;
			return j;
		}
		
		
		//private:
		

		
		void expand(Node* n, State& s) {
			mLog_expd++;
			
			mTest_maxf = mathutil::max(mTest_maxf, n->f);

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
					
					evalHr(kid_node, edgeIt.state());

					mOpenList.push(kid_node);
					mClosedList.add(kid_node);
				}
			}
		}
		

		void evalHr(Node* n, State const& s) {
			if(Search_Mode == AstarSearchMode::Standard)
				n->f = n->g + mHrModule.costHeuristic(s);
			else if(Search_Mode == AstarSearchMode::Weighted)
				gen_assert(false);
				//n->f = n->g + mParam_hrWeight * mHrModule.costHeuristic(s);
			else if(Search_Mode == AstarSearchMode::Greedy)
				n->f = mHrModule.costHeuristic(s);
			else if(Search_Mode == AstarSearchMode::Speedy)
				n->f = mHrModule.distanceHeuristic(s);
			else
				n->f = n->g;
		}
		


		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		HrModule<Hr_Mode> mHrModule;
		const double mParam_hrWeight;
		
		Node* mGoalNode;
		
		unsigned mLog_expd, mLog_gend, mLog_dups, mLog_reopnd;
		
		Cost mTest_maxf;
	};
}}
