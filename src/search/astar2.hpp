#pragma once

#include <string>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/exception.hpp"


namespace mjon661 { namespace algorithm {


	enum struct Astar2SearchMode {
		Standard, Weighted, Greedy, Speedy, Uninformed
	};
	
	template<typename = void>
	std::string astar2SearchModeStr(Astar2SearchMode m) {
		if(m == Astar2SearchMode::Standard)
			return std::string("Standard");
		else if(m == Astar2SearchMode::Weighted)
			return std::string("Weighted");
		else if(m == Astar2SearchMode::Greedy)
			return std::string("Greedy");
		else if(m == Astar2SearchMode::Speedy)
			return std::string("Speedy");
		return std::string("Uninformed");
	}


	template<typename D, Astar2SearchMode Search_Mode, bool Use_Abstraction_Hr, bool Perfect_Hr>
	class Astar2Impl {
		public:

		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		
		static const unsigned Abt_Working_Level = Perfect_Hr ? 0 : 1;
		
		using AbtSearch = AdmissibleAbtSearch<D, Abt_Working_Level, D::Top_Abstract_Level+1, true>;
		
		static_assert(!Use_Abstraction_Hr || Search_Mode!=Astar2SearchMode::Speedy, "");
		static_assert(!Perfect_Hr || Use_Abstraction_Hr, "");
		
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
		
		
		
		
		
		
		Astar2Impl(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mAbtSearch			(pDomStack, jConfig)
		{
			if(Search_Mode == Astar2SearchMode::Weighted)
				mHrWeight = jConfig.at("weight");
		}

		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();

			mLog_expd = mLog_gend = mLog_dups = mLog_reopnd = 0;
			
			mTest_exp_f.clear();			
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
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s)) {
					mGoalNode = n;
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
			j["search_mode"] = astar2SearchModeStr(Search_Mode);
			
			if(Search_Mode == Astar2SearchMode::Weighted)
				j["hr_weight"] = mHrWeight;

			return j;
		}
		
		
		//private:
		

		
		void expand(Node* n, State& s) {
			mLog_expd++;
			
			mTest_exp_f.push_back(n->f);
			
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
			if(Search_Mode == Astar2SearchMode::Standard)
				n->f = n->g + getCostHr(s);
			else if(Search_Mode == Astar2SearchMode::Weighted)
				n->f = n->g + mHrWeight * getCostHr(s);
			else if(Search_Mode == Astar2SearchMode::Greedy)
				n->f = getCostHr(s);
			else if(Search_Mode == Astar2SearchMode::Speedy)
				n->f = mDomain.distanceHeuristic(s);
			else
				n->f = n->g;
		}
		
		
		template<bool> struct Tag{};
		
		Cost getCostHr(State const& s) {
			if(Use_Abstraction_Hr) {
				Cost h;
				doAbtSearch(s, h, Tag<Perfect_Hr>{});
				return h;
			}
			return mDomain.costHeuristic(s);
		}		
		
		void doAbtSearch(State const& s, Cost& h, Tag<false>) {
			mAbtSearch.doSearch_ParentState(s, h);
		}
		
		void doAbtSearch(State const& s, Cost& h, Tag<true>) {
			mAbtSearch.doSearch(s, h);
		}	

		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		AbtSearch mAbtSearch;
		
		Node* mGoalNode;
		double mHrWeight;
		
		unsigned mLog_expd, mLog_gend, mLog_dups, mLog_reopnd;
		
		std::vector<double> mTest_exp_f;
	};
}}
