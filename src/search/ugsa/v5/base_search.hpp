#pragma once

#include <string>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/exception.hpp"

#include "search/ugsa/v5/common.hpp"
#include "search/ugsa/v5/abt_search.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav5 {


	template<typename D, unsigned Top, typename StatsManager>
	class UGSAv5_Base {
		
		using this_t = UGSAv5_Base<D, Top, StatsManager>;

		public:
		

		
		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;
		using StatsAcc = typename StatsManager::template StatsAcc<0>;
		


		struct Node {
			Cost g, f;
			unsigned depth;
			PackedState pkd;
			Operator in_op, parent_op;
			Node* parent;
			
			unsigned unexpdChildren;
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
		
		
		
		struct UCalcHBF {
			UCalcHBF(this_t& pInst) :
				mInst(pInst)
			{}
			
			Cost operator()(Cost g, unsigned depth) {
				//return mInst.mWf * g + mathutil::sumOfPowers(mInst.mBehaviour.gethbf(), depth);
				return g;
			}
			
			this_t& mInst;
		};
		
		using AbtSearch = UGSAv5_Abt<D, UCalcHBF, 1, Top+1, StatsManager>;
		
		
		

		using OpenList_t = OpenList<Node, Node, OpenOps>;
		
		using ClosedList_t = ClosedList<Node, 
										typename OpenList_t::Wrapped_t, 
										PackedState, 
										ClosedOps,
										ClosedOps,
										Domain::Hash_Range>;
									  
		using NodePool_t = NodePool<Node, typename ClosedList_t::Wrapped_t>;
		
		

		UGSAv5_Base(D& pDomStack, Json const& jConfig, StatsManager& pStats) :
			mStatsAcc			(pStats),
			mBehaviour			(jConfig),
			mUCalc				(*this),
			mAbtSearch			(pDomStack, jConfig, mUCalc, pStats),
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mInitState			(mDomain.createState()),
			mExpandedFront		(0),
			mExpandedBack		(0)
		{}

		
		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mStatsAcc.reset();
			mAbtSearch.reset();
			mBehaviour.reset();
			mExpandedBack = mExpandedFront = 0;
		}
		
		void submitStats() {
			mStatsAcc.submit(mBehaviour.report());
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
				
				n0->unexpdChildren = -1;
				
				SolValues abtRes = mAbtSearch.doSearch(mInitState, 1);
				mBehaviour.setInitNodeValues(abtRes.g, abtRes.depth);
				mBehaviour.informAbtSearch(0, abtRes.g, 0, abtRes.depth);
				
				n0->f = abtRes.u;
				
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

			mBehaviour.informExpansion(n->f);

			slow_assert(n->unexpdChildren == -1);
			unsigned gendChildren = 0;
			
			slow_assert(n->parent->unexpdChildren >= 1);
			n->parent->unexpdChildren--;
			
			if(n->parent->unexpdChildren == 0) {
				mExpandedFront--;
				mExpandedBack++;
			}
						
			OperatorSet ops = mDomain.createOperatorSet(s);
			
			for(unsigned i=0; i<ops.size(); i++) {
				if(ops[i] == n->parent_op)
					continue;
				
				mStatsAcc.a_gend();
				if(considerkid(n, s, ops[i]))
					gendChildren++;
			}
			
			n->unexpdChildren = gendChildren;
			
			if(gendChildren == 0)
				mExpandedBack++;
			else
				mExpandedFront++;
			
			slow_assert(mExpandedFront + mExpandedBack == mStatsAcc.mSrecord.expd);
		}
		
		
		bool considerkid(Node* pParentNode, State& pParentState, Operator const& pInOp) {

			Edge		edge 	= mDomain.createEdge(pParentState, pInOp);
			Cost 		kid_g   = pParentNode->g + edge.cost();
			unsigned 	kid_depth = pParentNode->depth + 1;
			
			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			bool wasGend = true;
			
			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				mStatsAcc.a_dups();
				if(kid_dup->g > kid_g) {
					
					kid_dup->f			-= kid_dup->g;
					kid_dup->f			+= kid_g;
					kid_dup->g			= kid_g;
					kid_dup->depth		= kid_depth;
					kid_dup->in_op		= pInOp;
					kid_dup->parent_op	= edge.parentOp();
					
					
					if(!mOpenList.contains(kid_dup)) {
						mStatsAcc.a_reopnd();
						
					}
					else {
						slow_assert(kid_dup->parent->unexpdChildren >= 1);
						kid_dup->parent->unexpdChildren--;
						
						if(kid_dup->parent->unexpdChildren == 0) {
							mExpandedFront--;
							mExpandedBack++;
						}
					}
					
					kid_dup->parent		= pParentNode;
					kid_dup->unexpdChildren = -1;
					
					mOpenList.pushOrUpdate(kid_dup);
				} 
				else
					wasGend = false;
			}
			else {
				Node* kid_node 		= mNodePool.construct();

				kid_node->g 		= kid_g;
				kid_node->depth		= kid_depth;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				kid_node->unexpdChildren = -1;
				
				SolValues abtRes = mAbtSearch.doSearch(mInitState);
				if(abtRes.searched)
					mBehaviour.informAbtSearch(kid_g, abtRes.g, kid_depth, abtRes.depth);
				
				kid_node->f = kid_g + abtRes.u;
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
			}
			
			mDomain.destroyEdge(edge);
			return wasGend;
		}


		StatsAcc				mStatsAcc;
		SearchBehaviour<>		mBehaviour;
		UCalcHBF				mUCalc;
		AbtSearch				mAbtSearch;
		const Domain			mDomain;
		
		OpenList_t 				mOpenList;
		ClosedList_t 			mClosedList;
		NodePool_t 				mNodePool;
		
		const State		 		mInitState;
		
		unsigned 				mExpandedBack;
		unsigned				mExpandedFront;
	};
}}}
