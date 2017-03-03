#pragma once

#include <vector>

#include "search/openlist.hpp"
#include "search/closedlist.hpp"
#include "search/nodepool.hpp"
#include "search/cache_store.hpp"
#include "structs/object_pool.hpp"
#include "util/json.hpp"
#include "util/debug.hpp"

#include "search/ugsa/v6/defs.hpp"
#include "search/ugsa/v6/abt_search.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav6 {


	template<typename D>
	class UGSAv6_Base {
		public:

		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;

		using AbtSearch = UGSAv6_Abt<D, 1, 1>;
		
		struct Node {
			Cost g;
			//unsigned depth;
			Util_t u;
			
			PackedState pkd;
			Node* parent;
			Operator in_op, parent_op;
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



		UGSAv6_Base(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mAbtSearch			(pDomStack, jConfig),
			mParams_wf			(jConfig.at("wf")),
			mParams_wt			(jConfig.at("wt"))
		{
			fast_assert(mParams_wf >= 0 && mParams_wt >= 0);
		}

		
		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mAbtSearch.reset();
		}
		
		void clearCache() {
			mAbtSearch.clearCache();
		}
		

		
		void doSearch(State const& s0, Solution<D>& pSolution) {
			mAbtSearch.resetParams(1, 1.2);
			
			{
				Node* n0 = mNodePool.construct();

				n0->g = 		Cost(0);
				n0->in_op = 	mDomain.getNoOp();
				n0->parent_op = mDomain.getNoOp();
				n0->parent = 	nullptr;

				n0->u = evalUtil(s0, 0);

				mDomain.packState(s0, n0->pkd);
				
				mOpenList.push(n0);
				mClosedList.add(n0);
			}			
			
			while(true) {				
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s)) {
					prepareSolution(pSolution, n);
					//mStatsAcc.s_end();
					break;
				}
				
				expand(n, s);
				
				//resorting
			}
			
			//mStatsAcc.s_end();
		}
		
		
		private:
		
		void prepareSolution(Solution<D>& sol, Node* pGoalNode) {
			std::vector<Node*> reversePath;
			
			for(Node *n = pGoalNode; n != nullptr; n = static_cast<Node*>(n->parent))
				reversePath.push_back(n);
			
			sol.states.clear();
			sol.operators.clear();
			
			for(unsigned i=reversePath.size()-1; i!=(unsigned)-1; i--) {
				State s;
				mDomain.unpackState(s, reversePath[i]->pkd);
				
				sol.states.push_back(s);
				
				if(i != reversePath.size()-1)
					sol.operators.push_back(reversePath[i]->in_op);	
				else
					fast_assert(reversePath[i]->in_op == mDomain.getNoOp());
			}
		}
		
		
		void expand(Node* n, State& s) {
			//inform expansion
			//mStatsAcc.a_expd();
			
			OperatorSet ops = mDomain.createOperatorSet(s);
			
			for(unsigned i=0; i<ops.size(); i++) {
				if(ops[i] == n->parent_op)
					continue;

				considerkid(n, s, ops[i]);
			}
		}
		
		
		void considerkid(Node* pParentNode, State& pParentState, Operator const& pInOp) {
			Edge		edge 	= mDomain.createEdge(pParentState, pInOp);
			Cost 		kid_g   = pParentNode->g + edge.cost();
			
			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				//mStatsAcc.a_dups();
				if(kid_dup->g > kid_g) {
					kid_dup->u 			= evalUtil(edge.state(), kid_g);
					kid_dup->g			= kid_g;

					kid_dup->in_op		= pInOp;
					kid_dup->parent_op	= edge.parentOp();
					kid_dup->parent		= pParentNode;
					
					if(!mOpenList.contains(kid_dup)) {
						//mStatsAcc.a_reopnd();
					}

					mOpenList.pushOrUpdate(kid_dup);
				}
			}
			else {
				Node* kid_node 		= mNodePool.construct();

				kid_node->g 		= kid_g;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				kid_node->u			= evalUtil(edge.state(), kid_g);

				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
			}
			
			mDomain.destroyEdge(edge);
		}
		
		Util_t evalUtil(State const& s, Cost g) {
			Cost remcost;
			Util_t remexp;
			mAbtSearch.computeRemainingEffort_parentState(s, remcost, remexp);
			return mParams_wf*(g + remcost) + mParams_wt*remexp;
		}
		
		
		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		AbtSearch mAbtSearch;
		Util_t mParams_wf, mParams_wt;
	};



}}}
