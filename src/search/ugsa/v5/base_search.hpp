#pragma once

#include <array>
#include <string>
#include <vector>
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


	template<typename D, unsigned Top, HeuristicModes H_Mode, typename StatsManager>
	struct UGSABaseHeuristic;
	

	template<typename D, unsigned Top, typename StatsManager>
	struct UGSABaseHeuristic<D, Top, HeuristicModes::Min_Cost, StatsManager> {
		
		using Domain = typename D::template Domain<0>;
		using State = typename Domain::State;
		using Cost = typename Domain::Cost;
		using AbtSearch = UGSAv5_Abt<D, 1, Top+1, true, StatsManager>;

		static const bool Has_Mult = false;

		UGSABaseHeuristic(D& pDomStack, Json const& jConfig, StatsManager& pStats) :
			mAbtSearch(pDomStack, jConfig, pStats)
		{}
		
		bool eval(State const& s, Cost& out_h, unsigned& out_d, bool = true) {
			return mAbtSearch.doSearch(s, out_h, out_d);
		}
		
		void reset() {
			mAbtSearch.reset();
		}
		
		void clearCache() {
			mAbtSearch.clearCache();
		}
		
		void submitStats() {
			mAbtSearch.submitStats();
		}

		AbtSearch mAbtSearch;
	};
	
	template<typename D, unsigned Top, typename StatsManager>
	struct UGSABaseHeuristic<D, Top, HeuristicModes::Min_Dist, StatsManager> {
		using Domain = typename D::template Domain<0>;
		using State = typename Domain::State;
		using Cost = typename Domain::Cost;
		using AbtSearch = UGSAv5_Abt<D, 1, Top+1, false, StatsManager>;
		
		static const bool Has_Mult = false;

		UGSABaseHeuristic(D& pDomStack, Json const& jConfig, StatsManager& pStats) :
			mAbtSearch(pDomStack, jConfig, pStats)
		{}
		
		bool eval(State const& s, Cost& out_h, unsigned& out_d, bool = true) {
			return mAbtSearch.doSearch(s, out_h, out_d);
		}
		
		void reset() {
			mAbtSearch.reset();
		}
		
		void clearCache() {
			mAbtSearch.clearCache();
		}
		
		void submitStats() {
			mAbtSearch.submitStats();
		}
		
		AbtSearch mAbtSearch;
	};

	
	template<typename D, unsigned Top, typename StatsManager>
	struct UGSABaseHeuristic<D, Top, HeuristicModes::Min_Cost_Or_Dist, StatsManager> {
		using Domain = typename D::template Domain<0>;
		using State = typename Domain::State;
		using Cost = typename Domain::Cost;
		using AbtSearch_cost = UGSAv5_Abt<D, 1, Top+1, true, StatsManager>;
		using AbtSearch_dist = UGSAv5_Abt<D, 1, Top+1, false, StatsManager>;
		
		static const bool Has_Mult = true;

		UGSABaseHeuristic(D& pDomStack, Json const& jConfig, StatsManager& pStats) :
			mAbtSearchCost(pDomStack, jConfig, pStats),
			mAbtSearchDist(pDomStack, jConfig, pStats)
		{}
		
		bool eval(State const& s, Cost& out_h, unsigned& out_d, bool b = true) {
			if(b)
				return mAbtSearchCost.doSearch(s, out_h, out_d);
			return mAbtSearchDist.doSearch(s, out_h, out_d);
		}
		
		void reset() {
			mAbtSearchCost.reset();
			mAbtSearchDist.reset();
		}
		
		void clearCache() {
			mAbtSearchCost.clearCache();
			mAbtSearchDist.clearCache();
		}
		
		void submitStats() {
			mAbtSearchCost.submitStats(0);
			mAbtSearchDist.submitStats(1);
		}
		
		AbtSearch_cost mAbtSearchCost;
		AbtSearch_dist mAbtSearchDist;
	};

	template<typename D, unsigned Top, typename StatsManager>
	struct UGSABaseHeuristic<D, Top, HeuristicModes::Min_Cost_And_Dist, StatsManager> {
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
		
		bool eval(State const& s, Cost& out_h, unsigned& out_d, bool b = true) {
			
			bool res = mAbtSearchCost.doSearch(s, out_h);
			res = mAbtSearchDist.doSearch(s, out_d) || res;
			return res;
		}
		
		void reset() {
			mAbtSearchCost.reset();
			mAbtSearchDist.reset();
		}
		
		void clearCache() {
			mAbtSearchCost.clearCache();
			mAbtSearchDist.clearCache();
		}
		
		void submitStats() {
			mAbtSearchCost.submitStats(0);
			mAbtSearchDist.submitStats(1);
		}
		
		AbtSearch_cost mAbtSearchCost;
		AbtSearch_dist mAbtSearchDist;
	};






	template<typename Cost>
	struct ComputeHBF {
		
		static const unsigned Resort_Fact = 2;


		ComputeHBF(Json const& jConfig) :
			mWf(jConfig.at("wf")),
			mUseConstantBF(jConfig.at("constant bf")),
			mConstantBF(jsonOptOrDef(jConfig, "constant bf", 0))
		{
			reset();
		}
		
		void reset() {
			mFCount_cur = mFCount_prevSum = 0;
			mCurF = 0;
			mHBF = 1;
			mPowCache.clear();
			mNobserved = 0;
			mNextResort = 16;
			
			if(mUseConstantBF)
				mHBF = mConstantBF;
		}
		
		bool shouldResort(unsigned pExpd) {
			if(mUseConstantBF || mNextResort > pExpd)
				return false;
			
			mNextResort *= Resort_Fact;
			return true;
		}

		Json report() {
			Json j;
			j["mFCount_cur"] = mFCount_cur;
			j["mFCount_prevSum"] = mFCount_prevSum;
			j["mNobserved"] = mNobserved;
			j["mTest_fLvls"] = mTest_fLvls;
			return j;
		}
	
		template<typename Nd>
		ucost_t compute_u(Nd* n, Cost h, unsigned d) {
			return compute_u(n->f, n->depth+d);
		}
		
		template<typename Nd>
		void inform_expansion(Nd* n, unsigned) {

			Cost pF = n->u;
			
			slow_assert(pF >= mCurF);
			slow_assert(pF != 0);		//If pF is 0, mNobserved will be incorrect.
			
			if(pF == mCurF)
				mFCount_cur++;
				
			else {
				if(mFCount_prevSum != 0 && !mUseConstantBF)
					mHBF = (double)(mFCount_cur + mFCount_prevSum) / mFCount_prevSum;

				mFCount_prevSum += mFCount_cur;
				mFCount_cur = 1;
				mCurF = pF;
				mPowCache.clear();
				mNobserved++;
			}
		}
		
		ucost_t compute_u(Cost f, unsigned dtot) {
			return f * mWf + mComputeHBF.raiseToPower(dtot);
		}
		
		unsigned raiseToPower(unsigned n) {
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
		

		
		private:

		unsigned mFCount_cur;
		unsigned mFCount_prevSum;
		unsigned mNobserved;
		
		Cost mCurF;
		double mHBF;
		
		std::vector<unsigned> mPowCache;
		std::vector<Cost> mTest_fLvls;

		unsigned mNextResort;
		
		const unsigned mWf;
		const bool mUseConstantBF;
		const double mConstantBF;
	};






	struct ComputeDelay {

		static const unsigned Delay_Resort_Fact = 2;
		
		ComputeDelay(Json const& jConfig) :
			mWf(jConfig.at("wf"))
		{
			reset();
		}

		void reset() {
			mNextResort = 16;
			mNext_expDelayAcc = 0;
			mLast_expDelay = 0;
			mNresorts = 0;
			mAllExpDelays.clear();
			mAllExpDelays.push_back(0);
		}
		
		bool shouldResort(unsigned pExpd) {
			if(mNextResort > pExpd)
				return false;
			
			mLast_expDelay = mNext_expDelayAcc / (mNextResort - mNextResort/Delay_Resort_Fact);
			mNext_expDelayAcc = 0;
			mNextResort *= Delay_Resort_Fact;
			mNresorts++;
			mAllExpDelays.push_back(mLast_expDelay);
			return true;
		}
		
		Json report() {
			Json j;
			j["exp delays"] = mAllExpDelays;
			j["resorts"] = mNresorts;
			return j;
		}
	
		template<typename Nd>
		ucost_t compute_u(Nd* n, Cost h, unsigned d) {
			return compute_u(n->f, d);
		}
		
		ucost_t compute_u(Cost f, unsigned d) {
			return f * mWf + d * mLast_expDelay;
		}
		
		template<typename Nd>
		void inform_expansion(Nd* n, unsigned pExpd) {
			mNext_expDelayAcc += pExpd - n->expdAtGen;
		}
		
		
		unsigned				mNextResort;
		unsigned				mNresorts;
		double					mLast_expDelay;
		double					mNext_expDelayAcc;
		std::vector<double>		mAllExpDelays;
		const unsigned 			mWf;
	};


	


	/*
	 * 	There are 3 components: the base space search algorithm, the h/d heuristics, and the u calculation.
	 * 
	 * 	Base space search algorithm is below.
	 * 	
	 * 	Heuristic components are specialisations of UGSABaseHeuristic.
	 * 		Min_Cost: return cost and depth of the cheapest solution path in abstract space.
	 * 		Min_Dist: return cost and depth of the shortest.
	 * 		Min_Cost_Or_Dist: minimum of Min_Cost and Min_Dist.
	 * 		Min_Cost_And_Dist: return cost of cheapest path, depth of shortest path.
	 * 
	 * 		UGSABaseHeuristic(DomStack, jConfig, StatsManager)
	 * 		bool eval(state, out_h, out_d, bool useFirstOption)
	 * 		void reset()
	 * 		void clearCache()
	 * 		void submitStats()
	 * 
	 * 
	 * 	U Calculation component:
	 * 		ctor(jConfig)
	 * 		void reset()
	 * 		bool shouldResort()
	 * 		Json report()
	 * 		void inform_expansion(Node)		
	 * 		ucost_t compute_u(Node n, Cost h, unsigned d)
	 * 		
	 * 		compute_u() expects all values, besides n->u to be set.
	 * 
	 * 		If shouldResort() returns true, the base space search algorithm should 
	 * 			immediately revaluate u for all open nodes, and resort the open list.
	 * 
	 */
	template<typename D, unsigned Top, HeuristicModes H_Mode, UCalcMode U_Mode, typename StatsManager>
	class UGSAv5_Base {
		
		using this_t = UGSAv5_Base<D, Top, H_Mode, U_Mode, StatsManager>;

		public:

		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;
		using StatsAcc = typename StatsManager::template StatsAcc<0>;
		
		using HeuristicsModule = UGSABaseHeuristic<D, Top, H_Mode, StatsManager>;
		
	
		template<U_Mode U, typename = void>
		struct NodeBase {
			using compu_t = ComputeHBF<Cost>;
			ucost_t u;
			Cost g, f;
			PackedState pkd;
			Operator in_op, parent_op;
			Node* parent;
			
			void set_expdAtGen(unsigned) {}
			void set_depth(unsigned) {}
			unsigned get_depth() {return 0;}
		};
		
		template<typename Ign>
		struct NodeBase<U_Mode::Delay, Ign> : public NodeBase<U_Mode::HBF> {
			using compu_t = ComputeDelay<Cost>;
			unsigned expdAtGen;
			unsigned depth;
			
			void set_expdAtGen(unsigned n) {
				expdAtGen = n;
			}
			
			void set_depth(unsigned d) {
				depth = d;
			}
			
			unsigned get_depth() {
				return depth;
			}
		};
		
		using Node = NodeBase<U_Mode>;
		using Ucompute_t = typename Node::compu_t;
		
		
		
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
				if(a->u != b->u)
					return a->u < b->u;
				if(a->f != b->f)
					return a->f < b->f;
				return a->g > b->g;
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
			mCompU				(jConfig),
			mHeuristics			(pDomStack, jConfig, pStats),
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
			mHeuristics.reset();
			mCompU.reset();
		}
		
		void submitStats() {
			Json j;
			j["compute u"] = mCompU.report();

			mStatsAcc.submit(j);
			mHeuristics.submitStats();
		}
		
		
		void doSearch(Solution<Domain>& pSolution) {
			{
				Node* n0 = mNodePool.construct();

				n0->g = 		Cost(0);
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;
				
				n0->set_expdAtGen(0);
				n0->set_depth(0);
				
				eval_heuristics(n0, mInitState);

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
				
				if(mCompU.shouldResort()) {
					resortOpenList();
				}
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
			mCompU.inform_expansion(n, mStatsAcc.getExpd());

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
			unsigned	kid_depth = pParentNode->get_depth() + 1;
			
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

					kid_dup->set_expdAtGen(mStatsAcc.getExpd());
					kid_dup->set_depth(kid_depth);
					
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

				kid_node->set_expdAtGen(mStatsAcc.getExpd());
				kid_node->set_depth(kid_depth);
				
				eval_heuristics(kid_node, edge.state());
				
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

				eval_heuristics(n, s);
			}
			mOpenList.reinit();
		}
		
		void eval_heuristics(Node* n, State const& s) {
			Cost hval;
			unsigned dval;
			ucost_t uval;
			
			mHeuristics.eval(mInitState, hval, dval);
			n->f = n->g + hval;

			uval = mCompU.compute_u(n, hval, dval);
			
			if(HeuristicsModule::Has_Mult) {
				Cost hval2;
				ucost_t uval2;
				mHeuristics.eval(mInitState, hval2, dval, false);
				n->f = n->g + hval2;
				uval2 = mCompU.compute_u(n, hval2, dval);
				
				if(uval2 < uval) {
					uval = uval2;
					hval = hval2;
				}
			}
			n->f = g + hval;
			n->u = uval;
		}


		StatsAcc				mStatsAcc;
		Ucompute_t				mCompU;
		HeuristicsModule		mHeuristics;

		const Domain			mDomain;
		
		OpenList_t 				mOpenList;
		ClosedList_t 			mClosedList;
		NodePool_t 				mNodePool;
		
		const State		 		mInitState;
		
		const unsigned			mWf;
		const bool				mUseResorting;
		

	};
	
	
	
	
	
}}}
