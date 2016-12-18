#pragma once

#include <string>
#include "search/hastar/generic/common.hpp"
#include "search/hastar/generic/abt_search.hpp"
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"


namespace mjon661 { namespace algorithm { namespace hastargeneric {


	template<	typename D,
				unsigned Top,
				E_StatsInfo P_Stats_Info>
	class HAstar_Base {
		public:
		
		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;
		using Abstractor = typename AbstractorIndirection<D, 0, 0 != Top>::type;
		using AbtState = typename AbtStateIndirection<D, 0, 0 != Top>::type;
		
		using AbtSearch = HAstar_Abt<D, 1, Top + 1, P_Stats_Info>;
		
		static const bool St_Col_Search = P_Stats_Info >= E_StatsInfo::Level;
		
		struct Node {
			Cost g, f;
			PackedState pkd;
			Operator in_op;
			Operator parent_op;
			Node* parent;
		};
		
		struct SearchStats {
			
			void inc_expd() { if(St_Col_Search) expd++; }
			void inc_gend() { if(St_Col_Search) gend++; }
			void inc_dups() { if(St_Col_Search) dups++; }
			void inc_reopnd() { if(St_Col_Search) reopnd++; }
			
			void reset() {
				expd = gend = dups = reopnd = 0;
			}
			
			unsigned expd, gend, dups, reopnd;
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
		
		

		HAstar_Base(D& pDomStack) :
			mAbtStatsCounter	(),
			mAbtSearch			(pDomStack, mAbtStatsCounter),
			mStats				(),
			mDomain				(pDomStack),
			mAbtor				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			()
		{}

		void fillSolution(Solution<Domain>& pSol, State const& s0) {
			Node* n = doSearch(s0);
			prepareSolution(pSol, n);
		}
		
		void reset(bool pClearCache) {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mStats.reset();
			
			if(pClearCache)
				mAbtSearch.levelResetRec();
			else
				mAbtSearch.levelResetRec_preserveCache();
				
			mAbtStatsCounter.reset();
		}
		
		Json report() {
			Json j;
			j["level 0"]["expd"] = mStats.expd;
			j["level 0"]["gend"] = mStats.gend;
			j["level 0"]["dups"] = mStats.dups;
			j["level 0"]["reopnd"] = mStats.reopnd;
			j["level 0"]["Node size"] = sizeof(Node);
			j["level 0"]["Wrapped Node Size"] = sizeof(typename ClosedList_t::Wrapped_t);
			j["level 0"]["closed fill"] = mClosedList.getFill();
			j["level 0"]["closed table size"] = mClosedList.size();
			j["level 0"]["open size"] = mOpenList.size();
			j["level 0"]["open capacity"] = mOpenList.capacity();
			mAbtSearch.addToReportRec(j);
			return j;
		}
		
		void dumpSearchesRec(std::string const& pPrefix) {
			mAbtSearch.dumpSearchesRec(pPrefix);
		}
		
		private:
		
		Node* doSearch(State const& s0) {
			{
				Node* n0 = mNodePool.construct();
				
				n0->g = 		Cost(0);
				n0->f = 		evalH(s0);
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;
				
				mDomain.packState(s0, n0->pkd);
				mOpenList.push(n0);
				mClosedList.add(n0);
			}
			
			while(true) {				
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s))
					return n;
				
				expand(n, s);
			}
			
			return nullptr;
		}
		
		
		void prepareSolution(Solution<typename D::template Domain<0>>& sol, Node* pGoalNode) {
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
			mStats.inc_expd();
			
			OperatorSet ops = mDomain.createOperatorSet(s);
			
			for(unsigned i=0; i<ops.size(); i++) {
				if(ops[i] == n->parent_op)
					continue;
				
				mStats.inc_gend();
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
				mStats.inc_dups();
				if(kid_dup->g > kid_g) {
					kid_dup->f 		   -= kid_dup->g;
					kid_dup->f 		   += kid_g;
					kid_dup->g 		   = kid_g;
					kid_dup->in_op	   = pInOp;
					kid_dup->parent_op = edge.parentOp();
					kid_dup->parent	   = pParentNode;
					
					if(!mOpenList.contains(kid_dup))
						mStats.inc_reopnd();
					
					mOpenList.pushOrUpdate(kid_dup);
				}
			} else {
				Node* kid_node = mNodePool.construct();
				kid_node->g 		= kid_g;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				
				kid_node->f = kid_g + evalH(edge.state());
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
			}
			
			mDomain.destroyEdge(edge);
		}
		
		Cost evalH(State const& pState) {
			AbtState abtState = mAbtor(pState);
			
			return mAbtSearch.getPathCost(abtState);
		}
		


		AbtStatsCounter			mAbtStatsCounter;
		AbtSearch				mAbtSearch;

		SearchStats 			mStats;
		const Domain			mDomain;
		Abstractor				mAbtor;
		
		OpenList_t 				mOpenList;
		ClosedList_t 			mClosedList;
		NodePool_t 				mNodePool;
	};
	
}}}
