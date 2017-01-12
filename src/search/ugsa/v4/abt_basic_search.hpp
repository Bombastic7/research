#pragma once

#include <string>
#include <cmath>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/math.hpp"

#include "search/ugsa/v4/defs.hpp"
#include "search/ugsa/v4/behaviour.hpp"
#include "search/ugsa/v4/cache.hpp"

#include <iostream>

namespace mjon661 { namespace algorithm { namespace ugsav4 {


	template<typename D, typename StatsManager>
	class UGSAv4_Abt {
		

		public:
		
		using Domain = typename D::template Domain<1>;
		using BaseDomain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;
		using StatsAcc = typename StatsManager::template StatsAcc<1>;
		
		using BaseAbstractor = typename D::template Abstractor<0>;
		using BaseState = typename D::template Domain<0>::State;


		

		struct Node {
			ucost_t ug;
			Cost g;
			unsigned depth;
			PackedState pkd;
			Operator in_op, parent_op;
			Node* parent;
		};
		
		struct CacheEntry {
			PackedState pkd;
			ucost_t u;
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
				return a->ug == b->ug ? a->g > b->g : a->ug < b->ug;
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
		
		

		UGSAv4_Abt(D& pDomStack, Json const& jConfig, UGSABehaviour<>& pBehaviour, StatsManager& pStats) :
			mBehaviour			(pBehaviour),
			mStatsAcc			(pStats),
			mAbtor				(pDomStack),
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			()
			//mCache				()
		{}

		
		void reset() {
			mStatsAcc.reset();
		}
		
		void submitStats() {
			mStatsAcc.submit();
		}

		
		ucost_t doSearch(BaseState const& pBaseState, unsigned pOpenSz) {
			
			mVar_openSz = pOpenSz;
			
			State s0 = mAbtor(pBaseState);
			PackedState pkd0;
			mDomain.packState(s0, pkd0);

			{

				
				Node* n0 = mNodePool.construct();
				
				n0->pkd =		pkd0;
				n0->g = 		0;
				n0->ug =		0;
				n0->depth =		0;
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;

				mOpenList.push(n0);
				mClosedList.add(n0);
			}
			

			ucost_t retu;
			
			while(true) {				
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s)) {
					retu = n->ug;
					break;
				}
				
				expand(n, s);
			}
			
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();

			mStatsAcc.s_end();
			return retu;
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
			ucost_t		kid_ug   	= compute_ug(kid_g, kid_depth);
			
			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				mStatsAcc.a_dups();
				if(kid_dup->ug > kid_ug) {
					kid_dup->ug			= kid_ug;
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
				kid_node->ug		= kid_ug;
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
		
		//ug = c + k * bf ** ug
		//c = wf * g
		//k = exptime * wt
		//bf = branching factor
		
		//ug = (c log(bf) - W(- bf^c k log(bf))) / log(bf)
		//where W is the Lambert W function.
		
		ucost_t compute_ug(Cost g, unsigned depth) {
			
			double c = mBehaviour.c_wf * g;
			double k = mBehaviour.getExpansionTime() * mBehaviour.c_wt;
			double bf = mBehaviour.getBranchingFactor();
			
			ucost_t ug = (c - mathutil::lambertW(- (std::pow(bf, c) * k * std::log(bf))) ) / std::log(bf);
			
			if(!std::isfinite(ug) || ug < c)
				ug = 0;
			
			ug = mathutil::max(ug, mathutil::max(c, depth * mBehaviour.c_wt));

			slow_assert(ug < 1e9); //Sanity check
			
			return ug;
		}


		

		UGSABehaviour<>&		mBehaviour;
		StatsAcc				mStatsAcc;

		BaseAbstractor			mAbtor;
		const Domain			mDomain;
		
		OpenList_t 				mOpenList;
		ClosedList_t 			mClosedList;
		NodePool_t 				mNodePool;
		
		unsigned mVar_openSz;
	};	
	
}}}
