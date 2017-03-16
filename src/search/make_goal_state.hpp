#pragma once

#include <algorithm>
#include <string>
#include <random>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/exception.hpp"


namespace mjon661 { namespace algorithm {


	//Algorithm for generating a goal state from a provided initial state and a minimum cost for the path between them.
	//The goal state is connected to the initial state.
	
	//Call execute(initState, minCost), then getGoalState().
	
	//This a best-first algorithm which maximises node g (partial path cost). Duplicate nodes with non-maximum g are pruned.
	//Generated states are push onto the openlist in non-deterministic order, to avoid operator-ordering causing issues.
	
	template<typename D>
	class MakeGoalStateAlg {
		public:

		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		
		
		struct Node {
			Cost g;
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
		
		
		
		
		
		
		MakeGoalStateAlg(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mRandDev			(),
			mRandGen			(mRandDev())
		{}

		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();

			mGoalNode = nullptr;
		}
		
		State getGoalState() {
			fast_assert(mGoalNode);
			
			State s;
			mDomain.unpackState(s, mGoalNode->pkd);
			return s;
		}

		
		void execute(State const& s0, Cost const& pMinCost) {
			doSearch(s0, pMinCost);
		}
		
		void doSearch(State const& s0, Cost const& pMinCost) {
			reset();
			{
				Node* n0 = mNodePool.construct();

				n0->g = 		Cost(0);
				n0->parent = 	nullptr;

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

				if(n->g >= pMinCost) {
					mGoalNode = n;
					break;
				}
				
				expand(n, s);
			}
		}
		

		void expand(Node* n, State& s) {
			mLog_expd++;
			
			std::vector<std::pair<State, Cost>> adjEdge;
			
			for(typename Domain::AdjEdgeIterator edgeIt = mDomain.getAdjEdges(s); !edgeIt.finished(); edgeIt.next())
				adjEdge.push_back({edgeIt.state(), edgeIt.cost()});
			
			std::shuffle(adjEdge.begin(), adjEdge.end(), mRandGen);
			
			for(auto& e : adjEdge) {
				
				PackedState kid_pkd;
				mDomain.packState(e.first, kid_pkd);
								
				if(n->pkd == kid_pkd)
					continue;
				
				mLog_gend++;
				
				Cost kid_g = n->g + e.second;

				Node* kid_dup = mClosedList.find(kid_pkd);

				if(kid_dup) {
					mLog_dups++;
					if(kid_dup->g < kid_g) {
						kid_dup->g			= kid_g;
						kid_dup->parent		= n;
						
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

					mOpenList.push(kid_node);
					mClosedList.add(kid_node);
				}
			}
		}
		


		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		
		Node* mGoalNode;
		
		std::random_device mRandDev;
		std::mt19937 mRandGen;
		
		unsigned mLog_expd, mLog_gend, mLog_dups, mLog_reopnd;

	};
}}
