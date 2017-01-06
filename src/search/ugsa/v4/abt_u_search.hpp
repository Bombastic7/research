#pragma once

#include <string>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"

#include "search/ugsa/v4/defs.hpp"
#include "search/ugsa/v4/behaviour.hpp"
#include "search/ugsa/v4/cache.hpp"
#include "search/ugsa/v4/abt_h_search.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav4 {


	template<typename D, unsigned Bound, typename StatsManager>
	class UGSAv4_Abt_U {
		

		public:

		using AbtSearch = UGSAv4_Abt_H<D, 1, Bound, StatsManager>;
		
		using Domain = typename D::template Domain<1>;
		using BaseDomain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using UCost = ucost_t;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;
		using StatsAcc = typename StatsManager::template StatsAcc<1>;
		
		using BaseAbstractor = typename D::template Abstractor<0>;
		using BaseState = typename D::template Domain<0>::State;
		



		struct Node {
			UCost u;
			Cost g;
			unsigned depth;
			PackedState pkd;
			Operator in_op, parent_op;
			Node* parent;
		};
		
		
		struct CacheEntry {
			PackedState pkd;
			UCost u;
			Cost g;
		};
		
		using CacheStore_t = CacheStore<Domain, CacheEntry>;
		
		
		
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
				return a->u == b->u ? a->g > b->g : a->u < b->u;
			}
		};
		
		
		

		using OpenList_t = OpenList<Node, Node, OpenOps>;
		
		using ClosedList_t = ClosedList<Node, 
										typename OpenList_t::Wrapped_t, 
										PackedState, 
										ClosedOps,
										ClosedOps,
										Domain::Hash_Range>;
									  
		using NodePool_t = NodePool<Node, typename ClosedList_t::Wrapped_t>;
		
		

		UGSAv4_Abt_U(D& pDomStack, UGSABehaviour<BaseDomain>& pBehaviour, StatsManager& pStats) :
			mBehaviour			(pBehaviour),
			mStatsAcc			(pStats),
			mAbtSearch			(pDomStack, pStats),
			mAbtor				(pDomStack),
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mCache				(mDomain)
		{}

		
		void reset() {
			mStatsAcc.reset();
			mAbtSearch.reset();
		}
		
		void submitStats() {
			mStatsAcc.submit();
			mAbtSearch.submitStats();
		}
		
		void clearCache() {
			mCache.clear();
			mAbtSearch.clearCache();
		}
		

		
		void doSearch(BaseState const& pBaseState, Cost& out_g, UCost& out_u) {
			
				State s0 = mAbtor(pBaseState);
				PackedState pkd0;
				mDomain.packState(s0, pkd0);

			{
				CacheEntry* ent = mCache.retrieve(pkd0);
				
				if(ent) {
					out_g = ent->g;
					out_u = ent->u;
					return;
				}

				
				//mBehaviour.informAbtSearchBegins(1, pBaseFrontierSz);
				
				Node* n0 = mNodePool.construct();
				
				n0->pkd =		pkd0;
				n0->g = 		0;
				n0->depth =		0;
				n0->u =			mBehaviour.abtHtoU(mAbtSearch.doSearch(s0));
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;

				mOpenList.push(n0);
				mClosedList.add(n0);
			}
			

			Cost retGCost = 0;
			UCost retUCost = 0;
			
			while(true) {				
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s)) {
					retGCost = n->g;
					retUCost = n->u;
					mStatsAcc.s_solutionFull();
					break;
				}
				
				expand(n, s);
			}

			
			//mBehaviour.informPath(L, goalNode->g, goalNode->depth);
			
			mStatsAcc.s_openListSize(mOpenList.size());
			mStatsAcc.s_closedListSize(mClosedList.getFill());
			
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			
			if(mBehaviour.abtShouldCache()) {
				CacheEntry* ent = nullptr;
				mCache.get(pkd0, ent);
				ent->g = retGCost;
				ent->u = retUCost;
			}
		
			//mBehaviour.informAbtSearchEnds();
			mStatsAcc.s_end();
			
			out_g = retGCost;
			out_u = retUCost;
		}
		
		
		private:
		
		void expand(Node* n, State& s) {
			mStatsAcc.a_expd();
			
			OperatorSet ops = mDomain.createOperatorSet(s);
			
			for(unsigned i=0; i<ops.size(); i++) {
				if(ops[i] == n->parent_op)
					continue;
				
				mStatsAcc.a_gend();
				considerkid(n, s, ops[i]);
			}
		}
		
		void considerkid(Node* pParentNode, State& pParentState, Operator const& pInOp) {

			Edge		edge 		= mDomain.createEdge(pParentState, pInOp);
			Cost		kid_g	 	= pParentNode->g + edge.cost();
			unsigned	kid_depth	= pParentNode->depth + 1;
			UCost		kid_u   	= mBehaviour.compute_U(kid_g, kid_depth) + mBehaviour.abtHtoU(mAbtSearch.doSearch(edge.state()));
			
			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				mStatsAcc.a_dups();
				if(kid_dup->u > kid_u) {
					kid_dup->u			= kid_u;
					kid_dup->g			= kid_g;
					kid_dup->depth		= kid_depth;
					kid_dup->in_op		= pInOp;
					kid_dup->parent_op	= edge.parentOp();
					kid_dup->parent		= pParentNode;

					
					if(!mOpenList.contains(kid_dup)) {
						mStatsAcc.a_reopnd();
					}
					
					mOpenList.pushOrUpdate(kid_dup);

				}
			} else {
				
				Node* kid_node 		= mNodePool.construct();
				
				kid_node->g 		= kid_g;
				kid_node->u			= kid_u;
				kid_node->depth		= kid_depth;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
			}
			
			mDomain.destroyEdge(edge);
		}
		

		UGSABehaviour<BaseDomain>&	mBehaviour;

		StatsAcc				mStatsAcc;
		AbtSearch				mAbtSearch;
		BaseAbstractor			mAbtor;
		const Domain			mDomain;
		
		OpenList_t 				mOpenList;
		ClosedList_t 			mClosedList;
		NodePool_t 				mNodePool;
		
		CacheStore_t			mCache;
	};	
	
}}}
