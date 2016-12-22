#pragma once

#include <string>
#include "search/ugsa/v2_bf/common.hpp"
#include "search/ugsa/v2_bf/abt_search.hpp"
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/exception.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav2_bf {


	template<	typename D,
				unsigned Top,
				E_StatsInfo P_Stats_Info>
	class UGSAv2_Base {
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
		
		using AbtSearch = UGSAv2_Abt<D, 1, Top + 1, P_Stats_Info>;
		
		static const bool St_Col_Search = P_Stats_Info >= E_StatsInfo::Level;
		
		struct Node {
			Cost g;
			Util_t u;
			PackedState pkd;
			Operator in_op;
			Operator parent_op;
			Node* parent;
			unsigned depth;
		};
		
		struct SearchStats {
			
			void inc_expd() { expd++; }
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
		
		

		UGSAv2_Base(D& pDomStack, Json const& j) :
			mConfig				(j),
			mBehaviour			(),
			mAbtStatsCounter	(),
			mAbtSearch			(pDomStack, mAbtStatsCounter, mConfig, mBehaviour),
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
				mAbtSearch.clearCacheRec();

			mAbtSearch.levelResetRec();
				
			mAbtStatsCounter.reset();
		}
		
		Json report() {
			Json jAll, j;
			j["expd"] = mStats.expd;
			j["gend"] = mStats.gend;
			j["dups"] = mStats.dups;
			j["reopnd"] = mStats.reopnd;
			j["Node size"] = sizeof(Node);
			j["Wrapped Node Size"] = sizeof(typename ClosedList_t::Wrapped_t);
			j["closed fill"] = mClosedList.getFill();
			j["closed table size"] = mClosedList.size();
			j["open size"] = mOpenList.size();
			j["open capacity"] = mOpenList.capacity();
			j["config"] = mConfig.report();
			j["behaviour"] = mBehaviour.report();
			jAll["level 0"] = j;
			mAbtSearch.addToReportRec(jAll);
			return jAll;
		}
		
		void dumpSearchesRec(std::string const& pPrefix) {
			mAbtSearch.dumpSearchesRec(pPrefix);
		}
		
		private:
		
		Node* doSearch(State const& s0) {
			mBehaviour.reset();
				
			{
				Node* n0 = mNodePool.construct();
				
				n0->g = 		Cost(0);
				n0->u = 		computeUtil(s0, n0->g);
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;
				n0->depth =		0;
				
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
				
				if(mBehaviour.shouldUpdate(mStats.expd)) {
					mBehaviour.update(mStats.expd);
					mAbtSearch.clearCacheRec();
					resortOpenList();
				}
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
			mBehaviour.putExpansion(n->depth);
			
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
					kid_dup->g 		   = kid_g;
					kid_dup->in_op	   = pInOp;
					kid_dup->parent_op = edge.parentOp();
					kid_dup->parent	   = pParentNode;
					kid_dup->u		   = computeUtil(edge.state(), kid_g);
					
					if(!mOpenList.contains(kid_dup)) {
						mStats.inc_reopnd();
					}
					
					mOpenList.pushOrUpdate(kid_dup);
				}
			} else {
				Node* kid_node 		= mNodePool.construct();

				kid_node->g 		= kid_g;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				kid_node->depth		= pParentNode->depth + 1;
				kid_node->u			= computeUtil(edge.state(), kid_g);
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
			}
			
			mDomain.destroyEdge(edge);
		}
		
		Util_t computeUtil(State const& pState, Cost pG) {
			AbtState abtState = mAbtor(pState);
			double ug = pG * mConfig.wf;
			
			return mAbtSearch.getUtilEst(abtState, ug);
		}
		
		void resortOpenList() {
			for(unsigned i=0; i<mOpenList.size(); i++) {
				Node* n = mOpenList.at(i);
				
				State s;
				mDomain.unpackState(s, n->pkd);
				
				n->u = computeUtil(s, n->g);
			}
			mOpenList.reinit();
		}
		
		
		UGSAConfig				mConfig;
		UGSABehaviour			mBehaviour;
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
