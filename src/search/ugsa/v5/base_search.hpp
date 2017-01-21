#pragma once

#include <array>
#include <string>
#include <cmath>
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


	template<typename> struct ComputeHBF;

	template<typename D, unsigned Top, HeuristicModes H_Mode, typename UCalc, typename StatsManager>
	struct UGSABaseHeuristic;
	
	
	template<typename D, unsigned Top, typename UCalc, typename StatsManager>
	struct UGSABaseHeuristic<D, Top, HeuristicModes::Min_Cost, UCalc, StatsManager> {
		
		using Domain = typename D::template Domain<0>;
		using State = typename Domain::State;
		using Cost = typename Domain::Cost;
		using AbtSearch = UGSAv5_Abt<D, 1, Top+1, true, StatsManager>;

		static const bool Has_Mult = false;

		UGSABaseHeuristic(D& pDomStack, Json const& jConfig, StatsManager& pStats) :
			mAbtSearch(pDomStack, jConfig, pStats)
		{}
		
		bool eval(State const&, Cost& out_h, unsigned& out_d, bool) {
			return mAbtSearch.doSearch(s, out_h, out_d);
		}

		AbtSearch mAbtSearch;
	};
	
	template<typename D, unsigned Top, typename UCalc, typename StatsManager>
	struct UGSABaseHeuristic<D, Top, HeuristicModes::Min_Dist, UCalc, StatsManager> {
		using Domain = typename D::template Domain<0>;
		using State = typename Domain::State;
		using Cost = typename Domain::Cost;
		using AbtSearch = UGSAv5_Abt<D, 1, Top+1, false, StatsManager>;
		
		static const bool Has_Mult = false;

		UGSABaseHeuristic(D& pDomStack, Json const& jConfig, StatsManager& pStats) :
			mAbtSearch(pDomStack, jConfig, pStats)
		{}
		
		bool eval(State const&, Cost& out_h, unsigned& out_d, bool) {
			return mAbtSearch.doSearch(s, out_h, out_d);
		}
		
		AbtSearch mAbtSearch;
	};
	
	template<typename D, unsigned Top, typename UCalc, typename StatsManager>
	struct UGSABaseHeuristic<D, Top, HeuristicModes::Min_Cost_And_Dist, UCalc, StatsManager> {
		using Domain = typename D::template Domain<0>;
		using State = typename Domain::State;
		using Cost = typename Domain::Cost;
		using AbtSearch_cost = UGSAv5_Abt<D, 1, Top+1, true, StatsManager>;
		using AbtSearch_dist = UGSAv5_Abt<D, 1, Top+1, false, StatsManager>;
		
		static const bool Has_Mult = true;

		UGSABaseHeuristic(D& pDomStack, Json const& jConfig, StatsManager& pStats) :
			mAbtSearch_cost(pDomStack, jConfig, pStats),
			mAbtSearch_dist(pDomStack, jConfig, pStats)
		{}
		
		bool eval(State const&, Cost& out_h, unsigned& out_d, bool b) {
			if(b)
				return mAbtSearchCost.doSearch(s, out_h, out_d);
			return mAbtSearchDist.doSearch(s, out_h, out_d);
		}
		
		AbtSearch_cost mAbtSearchCost;
		AbtSearch_dist mAbtSearchDist;
	};

	template<typename D, unsigned Top, typename UCalc, typename StatsManager>
	struct UGSABaseHeuristic<D, Top, HeuristicModes::Min_Cost_Or_Dist, UCalc, StatsManager> {
		using Domain = typename D::template Domain<0>;
		using State = typename Domain::State;
		using Cost = typename Domain::Cost;
		using AbtSearch_cost = UGSAv5_Abt2<D, 1, Top+1, true, StatsManager>;
		using AbtSearch_dist = UGSAv5_Abt2<D, 1, Top+1, false, StatsManager>;
		
		static const bool Has_Mult = false;

		UGSABaseHeuristic(D& pDomStack, Json const& jConfig, StatsManager& pStats) :
			mAbtSearchCost(pDomStack, jConfig, pStats),
			mAbtSearchDist(pDomStack, jConfig, pStats)
		{}
		
		bool eval(State const&, Cost& out_h, unsigned& out_d, bool) {
			
			bool res = mAbtSearchCost.doSearch(s, out_h);
			res = mAbtSearchDist.doSearch(s, out_d) || res;
			return res;
		}
		
		AbtSearch_cost mAbtSearchCost;
		AbtSearch_dist mAbtSearchDist;
	};
	
	







	template<typename D, unsigned Top, HeuristicModes H_Mode, typename StatsManager>
	class UGSAv5_Base {
		
		using this_t = UGSAv5_Base<D, Top, H_Mode, StatsManager>;

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
			Cost g, f;
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
			mComputeHBF			(),
			mAbtSearch			(pDomStack, jConfig, pStats),
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mInitState			(mDomain.createState()),
			mWf					(jConfig.at("wf")),
			mUseResorting		(jConfig.at("resort"))
		{
			fast_assert(jConfig.at("wt") == 1);
		}

		
		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mStatsAcc.reset();
			mAbtSearch.reset();
			mComputeHBF.reset();
		}
		
		void submitStats() {
			Json j = mComputeHBF.report();
			//j["hbf"] = mExpansionStats.computeHBF();
			
			mStatsAcc.submit(j);
			mAbtSearch.submitStats();
		}
		
		
		void doSearch(Solution<Domain>& pSolution) {
				
			{
				Node* n0 = mNodePool.construct();

				n0->g = 		Cost(0);
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;
				
				Cost hval;
				unsigned dval;
				
				mAbtSearch.doSearch(mInitState, hval, dval);
				
				n0->f = hval;
				n0->u = compute_u(hval, dval);
				
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
			
			mStatsAcc.s_end();
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

			mComputeHBF.inform_expansion(n->u);
	
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
					
					kid_dup->u			-= kid_dup->g * mWf;
					kid_dup->u			+= kid_g * mWf;
					
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
				
				Cost hval;
				unsigned dval;
				
				mAbtSearch.doSearch(edge.state(), hval, dval);
				
				kid_node->f			= kid_g + hval;
				kid_node->u			= compute_u(kid_node->f, dval);
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
			}
			
			mDomain.destroyEdge(edge);
		}
		
		void resortOpenList() {			
			for(unsigned i=0; i<mOpenList.size(); i++) {
				Node* n = mOpenList.at(i);
				
				State s;
				mDomain.unpackState(s, n->pkd);
				
				unsigned hval, dval;
				mAbtSearch.doSearch(s, hval, dval);
				
				n->u = compute_u(n->f, dval);
			}
		}
		
		ucost_t compute_u(Cost f, unsigned d) {

			return f * mWf + mComputeHBF.raiseToPower(d);
		}



		StatsAcc				mStatsAcc;
		ComputeHBF<Cost>		mComputeHBF;
		
		AbtSearch				mAbtSearch;
		const Domain			mDomain;
		
		OpenList_t 				mOpenList;
		ClosedList_t 			mClosedList;
		NodePool_t 				mNodePool;
		
		const State		 		mInitState;
		
		const unsigned			mWf;
		const bool				mUseResorting;
	};
	
	
	
	template<typename Cost>
	struct ComputeHBF {

		ComputeHBF() {
			reset();
		}
		
		void reset() {
			mFCount_cur = mFCount_prevSum = 0;
			mCurF = 0;
			mHBF = 1;
			mPowCache.clear();
			mNobserved = 0;
		}
		
		unsigned raiseToPower(unsigned n) {
			return std::pow(2, n); //.........
			
			if(n <= mPowCache.size())
				for(unsigned i=mPowCache.size(); i<=n; i++)
					mPowCache.push_back(std::pow(mHBF, i));
			
			return mPowCache[n];
		}
		
		double getHBF() {
			return mHBF;
		}
		
		unsigned getObservedLevels() {
			return mNobserved;
		}
		
		void inform_expansion(Cost pF) {
			mTest_fLvls.push_back(pF);
			return; //..........
			
			slow_assert(pF >= mCurF);
			slow_assert(pF != 0);		//If pF is 0, mNobserved will be incorrect.
			
			if(pF == mCurF)
				mFCount_cur++;
				
			else {
				if(mFCount_prevSum != 0)
					mHBF = (double)(mFCount_cur + mFCount_prevSum) / mFCount_prevSum;

				mFCount_prevSum += mFCount_cur;
				mFCount_cur = 1;
				
				mCurF = pF;
				
				mPowCache.clear();
				
				mNobserved++;
			}
		}
		
		Json report() {
			Json j;
			j["mFCount_cur"] = mFCount_cur;
			j["mFCount_prevSum"] = mFCount_prevSum;
			j["mNobserved"] = mNobserved;
			j["mTest_fLvls"] = mTest_fLvls;
			return j;
		}
		
		private:

		unsigned mFCount_cur;
		unsigned mFCount_prevSum;
		unsigned mNobserved;
		
		Cost mCurF;
		double mHBF;
		
		std::vector<unsigned> mPowCache;
		std::vector<Cost> mTest_fLvls;
	};
}}}
