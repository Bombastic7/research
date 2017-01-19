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
#include "search/ugsa/v5/expansion_stats.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav5 {


	template<typename D, unsigned Top, typename StatsManager>
	class UGSAv5_Base {
		
		using this_t = UGSAv5_Base<D, Top, StatsManager>;

		public:
		
		using AbtSearch = UGSAv5_Abt<D, 1, Top+1, StatsManager>;
		
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
			Cost g;
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
				return a->u == b->u ? a->g > b->g : a->u < b->u;
			}
		};
		
		
		/*
		struct UCalcHBF {
			UCalcHBF(this_t& pInst) :
				mInst(pInst)
			{}
			
			void clearCache() {
				mSumPowCache.clear();
			}
			
			Cost operator()(Cost g, unsigned depth) {
				return g;//..............
				if(!mSumPowCache.contains(depth)) {
					mSumPowCache[depth].val = 
						mInst.mWf * g + 
						mathutil::sumOfPowers(mInst.mVarCurHBF, mInst.mVarCurBaseDepth, mInst.mVarCurBaseDepth + depth);
				}
				
				return mSumPowCache[depth].val;
			}
			
			SimpleHashMap<unsigned, unsigned, 100> mSumPowCache;
			this_t& mInst;
		};
		*/
		
		
		
		

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
			mExpansionStats		(),
			mUCalc				(*this),
			mAbtSearch			(pDomStack, jConfig, mUCalc, pStats),
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mInitState			(mDomain.createState()),
			mWf					(jConfig.at("wf")),
			mUseResorting		(jConfig.at("resort")),
			mExpdNextResort		(),
			mVarCurBaseDepth	(),
			mVarCurHBF			()
		{
			fast_assert(jConfig.at("wt") == 1);
		}

		
		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mStatsAcc.reset();
			mAbtSearch.reset();
			mExpansionStats.reset();
		}
		
		void submitStats() {
			Json j;
			j["hbf"] = mExpansionStats.computeHBF();
			j["bf"] = mExpansionStats.computeDepthBF();
			
			mStatsAcc.submit(j);
			mAbtSearch.submitStats();
		}
		
		
		void doSearch(Solution<Domain>& pSolution) {
			
			if(mUseResorting) {
				mExpdNextResort = 64;
			}
			
			mVarCurHBF = 1;
			mVarCurBaseDepth = 0;
				
			{
				Node* n0 = mNodePool.construct();

				n0->g = 		Cost(0);
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;
				
				Cost hval;
				unsigned dval;
				
				mAbtSearch.doSearch(mInitState, hval, dval);
				
				n0->u = abtRes.u;
				//n0->f = hval;
				
				
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
				
				if(mUseResorting && mStatsAcc.getExpd() >= mExpdNextResort) {
					//update hbf
					resortOpenList();
					mExpdNextResort *= 2;
				}
			}
		}
		
		
		private:
		
		void prepareSolution(Solution<Domain>& sol, Node* pGoalNode) {
			std::vector<Node*> reversePath;
			
			for(Node *n = pGoalNode; n != nullptr; n = n->parent)
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

			//mExpansionStats.informExpansion(n->f, n->depth);
	
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
			//unsigned 	kid_depth = pParentNode->depth + 1;
			
			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);
			
			
			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				mStatsAcc.a_dups();
				if(kid_dup->g > kid_g) {
					
					kid_dup->u			-= kid_dup->g / mWf;
					kid_dup->u			+= kid_g * mWf;
					
					//kid_dup->f			-= kid_dup->g;
					//kid_dup->f			+= kid_g;
					
					kid_dup->g			= kid_g;
					kid_dup->in_op		= pInOp;
					kid_dup->parent_op	= edge.parentOp();
					kid_dup->parent		= pParentNode;
					
					if(!mOpenList.contains(kid_dup)) {
						mStatsAcc.a_reopnd();
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
				
				if(!mUseResorting)
					mVarCurHBF = mExpansionStats.computeHBF();
				
				mVarCurBaseDepth = kid_depth;
				
				SolValues abtRes;
				mAbtSearch.doSearch(edge.state(), abtRes);
				mUCalc.clearCache();
				
				kid_node->u = kid_g * mWf + abtRes.u;
				kid_node->f = kid_g + abtRes.g;
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
			}
			
			mDomain.destroyEdge(edge);
		}
		
		void resortOpenList() {
			mVarCurHBF = mExpansionStats.computeHBF();
			
			for(unsigned i=0; i<mOpenList.size(); i++) {
				Node* n = mOpenList.at(i);
				mVarCurBaseDepth = n->depth;
				
				State s;
				mDomain.unpackState(s, n->pkd);
				
				SolValues abtRes;
				bool full = mAbtSearch.doSearch(s, abtRes);
				mUCalc.clearCache();
				if(full)
					mTest_solvedFull++;
				else
					mTest_solvedCached++;
				
				n->u = n->g * mWf + abtRes.u;
			}
		}


		StatsAcc				mStatsAcc;
		ExpansionStats<>		mExpansionStats;
		UCalcHBF				mUCalc;
		AbtSearch				mAbtSearch;
		const Domain			mDomain;
		
		OpenList_t 				mOpenList;
		ClosedList_t 			mClosedList;
		NodePool_t 				mNodePool;
		
		const State		 		mInitState;
		
		const unsigned			mWf;
		const bool				mUseResorting;
		unsigned				mExpdNextResort;
		//unsigned				mVarCurBaseDepth;
		//double					mVarCurHBF;
		
		//unsigned mTest_solvedFull, mTest_solvedCached;
	};
}}}
