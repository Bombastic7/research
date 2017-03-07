#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <utility>
#include <tuple>

#include "search/openlist.hpp"
#include "search/closedlist.hpp"
#include "search/nodepool.hpp"
#include "search/cache_store.hpp"
#include "structs/object_pool.hpp"
#include "util/json.hpp"
#include "util/debug.hpp"

#include "search/ugsa/v6/defs.hpp"

namespace mjon661 { namespace algorithm { namespace ugsav6 {



	template<typename D, unsigned L, unsigned Bound>
	class UGSAv6_Abt {
		public:

		using Domain = typename D::template Domain<L>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;



		struct Node {
			Cost g;
			unsigned depth;
			Util_t ug;
			Util_t remexp;
			
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
				if(a->ug != b->ug)
					return a->ug > b->ug;
				if(a->depth != b->depth)
					return a->depth > b->depth;
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



		UGSAv6_Abt(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mParams_wf			(jConfig.at("wf")),
			mParams_wt			(jConfig.at("wt")),
			mParams_k			(0),
			mParams_bf			(0)
		{
			fast_assert(mParams_wf >= 0 && mParams_wt >= 0);
		}

		
		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
		}

		
		
		template<typename PS>
		std::tuple<Cost, unsigned, Util_t, Util_t> computeRemainingEffort_parentState(PS const& pParentState, Util_t bf, Util_t k) {
			State s0 = mDomain.abstractParentState(pParentState);
			return computeRemainingEffort(s0, bf, k);
		}
		
		std::tuple<Cost, unsigned, Util_t, Util_t> computeRemainingEffort(State const& s0, Util_t bf, Util_t k) {
			mParams_bf = bf;
			mParams_k = k;
			return doSearch(s0);
		}

		
		std::tuple<Cost, unsigned, Util_t, Util_t> doSearch(State const& s0) {
			reset();
			Node* n0 = mNodePool.construct();

			n0->g = 		Cost(0);
			n0->depth =		0;
			n0->ug = 		0;
			n0->remexp =	0;
			n0->in_op = 	mDomain.getNoOp();
			n0->parent_op = mDomain.getNoOp();
			n0->parent = 	nullptr;

			mDomain.packState(s0, n0->pkd);
			
			mOpenList.push(n0);
			mClosedList.add(n0);
			
			Node* goalNode = nullptr;
			
			while(true) {				
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s)) {
					goalNode = n;
					break;
				}
				
				expand(n, s);
			}
			
			return std::make_tuple(goalNode->g, goalNode->depth, goalNode->remexp, goalNode->ug);
			
		}
		

		Json report() {
			return Json();
		}

		
		void expand(Node* n, State& s) {
			//mStatsAcc.a_expd();
			
			OperatorSet ops = mDomain.createOperatorSet(s);
			
			for(unsigned i=0; i<ops.size(); i++) {
				if(ops[i] == n->parent_op)
					continue;
				
				considerkid(n, s, ops[i]);
			}
		}
		
		
		void considerkid(Node* pParentNode, State& pParentState, Operator const& pInOp) {
			Edge		edge 	= 		mDomain.createEdge(pParentState, pInOp);
			Cost 		kid_g   = 		pParentNode->g + edge.cost();
			unsigned	kid_depth = 	pParentNode->depth + 1;
			Util_t		kid_remexp =	pParentNode->remexp + mParams_k * std::pow(mParams_bf, kid_depth);
			Util_t		kid_ug = 		mParams_wf * kid_g + mParams_wt * kid_remexp;
			
			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				//mStatsAcc.a_dups();
				if(kid_dup->ug > kid_ug) {
					kid_dup->g			= kid_g;
					kid_dup->depth		= kid_depth;
					kid_dup->remexp		= kid_remexp;
					kid_dup->ug			= kid_ug;

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
				kid_node->depth		= kid_depth;
				kid_node->remexp	= kid_remexp;
				kid_node->ug		= kid_ug;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;

				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
			}
			
			mDomain.destroyEdge(edge);
		}



		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		double mParams_wf, mParams_wt, mParams_k, mParams_bf;
	};
}}}
