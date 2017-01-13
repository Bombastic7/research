#pragma once

#include <string>
#include <cmath>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"

#include "search/ugsa/v5/common.hpp"



namespace mjon661 { namespace algorithm { namespace ugsav5 {


	template<typename D, unsigned L, unsigned Bound, typename StatsManager>
	class UGSAv5_Abt {
		

		public:
		
		//using AbtSearch = HAstar_Abt<D, L+1, Bound, StatsManager>;
		
		using Domain = typename D::template Domain<L>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;
		using StatsAcc = typename StatsManager::template StatsAcc<L>;
		
		using BaseAbstractor = typename D::template Abstractor<L-1>;
		using BaseState = typename D::template Domain<L-1>::State;
		



		struct Node {
			Cost g, u;
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
		
		

		UGSAv5_Abt(D& pDomStack, Json const& jConfig, SearchBehaviour<>& pBehaviour, StatsManager& pStats) :
			mStatsAcc			(pStats),
			mBehaviour			(pBehaviour),
			mAbtor				(pDomStack),
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mTradeOff			(jConfig.at("tradeoff"))
		{}

		
		void reset() {
			mStatsAcc.reset();
			mAbtSearch.reset();
		}

		
		void submitStats() {
			mStatsAcc.submit();
			mAbtSearch.submitStats();
		}
		
		
		
		Cost doSearch(BaseState const& pBaseState) {
			
			{
				State s0 = mAbtor(pBaseState);
				PackedState pkd0;
				
				mDomain.packState(s0, pkd0);

				Node* n0 = mNodePool.construct();
				
				n0->pkd =		pkd0;
				n0->g = 		Cost(0);
				n0->u = 		Cost(0);
				n0->depth =		0;
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;

				mOpenList.push(n0);
				mClosedList.add(n0);
			}

			Cost retCost = 0;
			
			while(true) {				
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s)) {
					retCost = n->u;
					break;
				}
				
				expand(n, s);
			}

			
			mStatsAcc.s_openListSize(mOpenList.size());
			mStatsAcc.s_closedListSize(mClosedList.getFill());
			
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();

			mStatsAcc.s_end();
			return retCost;
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
			
			Cost		kid_u		= mTradeOff * kid_g + std::pow( mBehaviour.gethbf(), kid_depth);
			
			
			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				mStatsAcc.a_dups();
				if(kid_dup->u > kid_u) {
					kid_dup->u			=kid_u;
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
				
				Node* kid_node = mNodePool.construct();
				
				kid_node->g 		= kid_g;
				kid_node->depth		= kid_depth;
				kid_node->u			= kid_u;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
			}
			
			mDomain.destroyEdge(edge);
		}
		


		StatsAcc				mStatsAcc;
		SearchBehaviour<>&		mBehaviour;
		BaseAbstractor			mAbtor;
		const Domain			mDomain;
		
		OpenList_t 				mOpenList;
		ClosedList_t 			mClosedList;
		NodePool_t 				mNodePool;
		
		unsigned				mTradeOff;
	};
	
	
	/*
	template<typename D, unsigned Bound, typename StatsManager>
	struct HAstar_Abt<D, Bound, Bound, StatsManager> {
		
		HAstar_Abt(D& pDomStack, StatsManager& pStats, AlgoConf<> const&) {}
		
		typename D::template Domain<Bound-1>::Cost doSearch(typename D::template Domain<Bound-1>::State const&) {return 0;}
		void reset() {}
		void clearCache() {}
		void submitStats() {}
	};
	*/
	
}}}
