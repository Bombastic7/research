#pragma once

#include <string>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/exception.hpp"

#include "search/ugsa/v4/defs.hpp"
#include "search/ugsa/v4/abt_u_search.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav4 {


	template<typename D, unsigned Top, typename StatsManager>
	class UGSAv4_Base {
		

		public:
		
		using AbtSearch = UGSAv4_Abt_U<D, Top+1, StatsManager>;
		
		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;
		using StatsAcc = typename StatsManager::template StatsAcc<0>;
		


		struct Node {
			ucost_t u;
			Cost f;
			Cost g;
			unsigned depth;
			PackedState pkd;
			Operator in_op, parent_op;
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
				return a->f == b->f ? a->g > b->g : a->f < b->f;
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
		
		

		UGSAv4_Base(D& pDomStack, Json const& jConfig, UGSABehaviour<Domain>& pBehaviour, StatsManager& pStats) :
			mBehaviour			(pBehaviour),
			mStatsAcc			(pStats),
			mAbtSearch			(pDomStack, jConfig, mBehaviour, pStats),
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mInitState			(mDomain.createState())
		{}

		
		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mStatsAcc.reset();
			mAbtSearch.reset();
		}
		
		void clearCache() {
			mAbtSearch.clearCache();
		}
		
		void submitStats() {
			mStatsAcc.submit();
			mAbtSearch.submitStats();
		}
		
		
		
		void doSearch(Solution<Domain>& pSolution) {
				
			{
				Node* n0 = mNodePool.construct();

				n0->g = 		Cost(0);
				n0->depth =		0;
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;
				
				mAbtSearch.doSearch(mInitState, n0->f, n0->u);
				
				mDomain.packState(mInitState, n0->pkd);
				
				mOpenList.push(n0);
				mClosedList.add(n0);
			}
			
			while(true) {				
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s)) {
					prepareSolution(pSolution, n);
					mStatsAcc.s_end();
					break;
				}
				
				expand(n, s);

			}
		}
		
		
		private:
		
		void prepareSolution(Solution<Domain>& sol, Node* pGoalNode) {
			std::vector<Node*> reversePath;
			
			for(Node *n = pGoalNode; n; n = n->parent)
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
					fast_assert(reversePath[i]->in_op == mDomain.noOp);
			}
		}
		
		void expand(Node* n, State& s) {
			mStatsAcc.a_expd();
			
			mBehaviour.informNodeExpansion(n->g, n->f, n->u, n->depth);
			
			OperatorSet ops = mDomain.createOperatorSet(s);
			
			for(unsigned i=0; i<ops.size(); i++) {
				if(ops[i] == n->parent_op)
					continue;
				
				mStatsAcc.a_gend();
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
				mStatsAcc.a_dups();
				if(kid_dup->g > kid_g) {
					
					kid_dup->f			-= kid_dup->g;
					kid_dup->f			+= kid_g;
					//kid_dup->u is not changed.
					kid_dup->g			= kid_g;
					kid_dup->depth 		= pParentNode->depth + 1;
					kid_dup->in_op		= pInOp;
					kid_dup->parent_op	= edge.parentOp();
					kid_dup->parent		= pParentNode;
					
					if(!mOpenList.contains(kid_dup)) {
						mStatsAcc.a_reopnd();
					}
					
					mOpenList.pushOrUpdate(kid_dup);
					
					//mBehaviour.informNodeGeneration(kid_dup->g, kid_dup->f, kid->u, kid_dup->depth);
				}
			} else {
				Node* kid_node 		= mNodePool.construct();

				kid_node->g 		= kid_g;
				kid_node->depth		= pParentNode->depth + 1;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				
				Cost kid_h;
				ucost_t kid_uh;
				
				mAbtSearch.doSearch(edge.state(), kid_h, kid_uh);
				
				kid_node->f = kid_g + kid_h;
				kid_node->u = kid_g + kid_uh;
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
				
				//mBehaviour.informNodeGeneration(kid_node->g, kid_node->f, kid_node->u, kid_node->depth);
			}
			
			mDomain.destroyEdge(edge);			
		}
		


		UGSABehaviour<Domain>&	mBehaviour;
		StatsAcc				mStatsAcc;
		AbtSearch				mAbtSearch;
		const Domain			mDomain;
		
		OpenList_t 				mOpenList;
		ClosedList_t 			mClosedList;
		NodePool_t 				mNodePool;
		
		const State		 		mInitState;
	};
}}}
