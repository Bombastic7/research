#pragma once

#include <string>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/exception.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav3 {


	template<typename D, unsigned Top, typename StatsManager>
	class UGSAv3_Base {
		

		public:
		
		using AbtSearch = UGSAv3_Abt<D, 1, Top+1, StatsManager>
		
		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;

		

		struct Node {
			Cost g, f;
			PackedState pkd;
			Operator in_op; parent_op;
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
		
		

		UGSAv3_Base(D& pDomStack, UGSAConfig const& pConfig, UGSABehaviour& pBehaviour, StatsManager& pStats) :
			mConfig				(pConfig),
			mBehaviour			(pBehaviour),
			mStatsAcc			(pStats),
			mAbtSearch			(pDomStack, pConfig, mBehaviour, pStats),
			mDomain				(pDomStack),
			mAbtor				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mInitState			(mDomain.createState())
		{}

		
		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mAbtSearch.reset();
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

		
		
		
		void doSearch(Solution<Domain>& pSolution) {
				
			{
				Node* n0 = mNodePool.construct();
				
				n0->g = 		Cost(0);
				n0->f = 		computeHeuristic(s0);
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;
				
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
					mStatsAcc.submit();
					break;
				}
				
				expand(n, s);
				/*
				if(mBehaviour.shouldUpdate(mStats.expd)) {
					mBehaviour.update(mStats.expd);
					mAbtSearch.clearCacheRec();
					resortOpenList();
				}*/
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
			mStatsAcc.expd();
			
			OperatorSet ops = mDomain.createOperatorSet(s);
			
			for(unsigned i=0; i<ops.size(); i++) {
				if(ops[i] == n->parent_op)
					continue;
				
				mStatsAcc.gend();
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
				mStatsAcc.dups();
				if(kid_dup->g > kid_g) {
					
					kid_dup->f			-= kid_dup->g;
					kid_dup->f			+= kid_g;
					kid_dup->g			= kid_g;
					kid_dup->in_op		= pInOp;
					kid_dup->parent_op	= edge.parentOp();
					kid_dup->parent		= pParentNode;
					
					if(!mOpenList.contains(kid_dup)) {
						mStatsAcc.reopnd();
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
				kid_node->expdAtGen = mStats.expd;
				kid_node->f			= kid->g + computeHeuristic(edge.state());
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
			}
			
			mDomain.destroyEdge(edge);
		}
		
		Cost computeHeuristic(State const& pState) {
			return mAbtSearch.doSearch(pState);
		}
		
		/*
		void resortOpenList() {
			for(unsigned i=0; i<mOpenList.size(); i++) {
				Node* n = mOpenList.at(i);
				
				State s;
				mDomain.unpackState(s, n->pkd);
				
				n->u = computeUtil(s, n->g);
			}
			mOpenList.reinit();
		}
		*/
		
		
		UGSAConfig				mConfig;
		UGSABehaviour			mBehaviour;
		StatsAcc				mStatsAcc;
		AbtSearch				mAbtSearch;
		const Domain			mDomain;
		
		OpenList_t 				mOpenList;
		ClosedList_t 			mClosedList;
		NodePool_t 				mNodePool;
		
		const BaseState 		mInitState;
	};
}}}
