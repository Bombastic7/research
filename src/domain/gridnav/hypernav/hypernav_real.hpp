#pragma once


#include <array>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <random>
#include <cmath>
#include <random>
#include <utility>
#include <tuple>
#include <algorithm>

#include "search/make_goal_state.hpp"
#include "util/math.hpp"

#include "domain/gridnav/blocked/cellmap_blocked.hpp"


namespace mjon661 { namespace gridnav { namespace hypernav_real {



	template<unsigned N, unsigned MaxK>
	struct AdjEdgeIterator_base {
		
		using Mv_t = std::array<std::pair<unsigned, bool>>;
		
		
		AdjEdgeIterator_base(	std::array<unsigned, N>& pState,
								HypergridMoveSet<N, MaxK> const& pMvSet,
								std::array<unsigned,N> const& pDimsSz, 
								CellMapReal<double> const& pCellMap) :
			mMvSet(pMvSet),
			mDimsSz(pDimsSz),
			mCellMap(pCellMap),
			mAdjState(pState),
			mLastOrd(0),
			mLastK(0),				//Invalid, set in tryApplyMv().
			mTest_origState(pState)
		{
			for(; mLastOrd<mMvSet.size(); mLastOrd++) {
				if(tryApplyMv(mLastOrd))
					break;
			}
		}
		
		~AdjEdgeIterator_base() {
			if(!finished())
				undoMv();
			slow_assert(mTest_origState == mAdjState);
		}
		
		
		void next() {
			slow_assert(!finished());
			undoMv();
			for(; mLastOrd<mMvSet.size(); mLastOrd++) {
				if(tryApplyMv(mLastOrd))
					break;
			}
		}
		
		bool finished() const {
			bool b = mLastOrd == mMvSet.size();
			if(b)
				slow_assert(mTest_origState == mAdjState);
		}
		
		std::array<unsigned, N>& state() const {
			slow_assert(!finished());
			return mAdjState;
		}
		
		double cost() const {
			slow_assert(!finished());
			return mCurCost * mCellMap.cells[doPackState(mAdjState)];
		}
		
	
		private:
		
		void undoMv() {
			unsigned k;
			Mv_t const& mv = mMvSet.getMove(mLastOrd, k);
			
			for(unsigned j=0; j<k; j++) {
				if(mv[j].second)
					mAdjState[mv[j].first]--;
				else
					mAdjState[mv[j].first]++;
			}
		}
			
		
		bool tryApplyMv(unsigned i) {
			unsigned k;
			Mv_t const& mv = mMvSet.getMove(i, k);
			
			for(unsigned j=0; j<k; j++) {
				if(mAdjState[mv[j].first] == 0 && !mv[j].second)
					return false;
				if(mAdjState[mv[j].first] == mDimSz[mv[j].first]-1 && mv[j].second)
					return false;
			}
			
			for(unsigned j=0; j<k; j++) {
				if(mv[j].second)
					mAdjState[mv[j].first]++;
				else
					mAdjState[mv[j].first]--;
			}
			
			if(k != mLastK) {
				mCurCost = mMvSet.getMoveCost(i);
				mLastK = k;
			}
			
			mLastOrd = i;
			return true;
		}
		
		
		HypergridMoveSet<N, MaxK> const& mMvSet;
		std::array<unsigned,N> const& mDimsSz;
		CellMapReal<double> const& pCellMap;
		
		std::array<unsigned, N> mAdjState;
		unsigned mLastOrd;
		unsigned mLastK;
		double mCurCost;
		
		const std::array<unsigned, N> mTest_origState;
	};
	
	
	
	

	template<unsigned N, unsigned MaxK>
	struct Domain_base {
		
		static_assert(N > 0, "");
		static_assert(N >= MaxK, "");
		
		using Cost = typename CostType<MaxK>::type;
		using State = StateN<N>;
		using PackedState = PackedStateN;
		using AdjEdgeIterator = AdjEdgeIterator_base<N, MaxK>;
		
		static const bool Is_Perfect_Hash = true;


		Domain_base(State const& pGoalState, std::array<unsigned,N> const& pDimSz, CellMapBlocked<> const& pCellMap) :
			mGoalState(pGoalState),
			mDimSz(pDimSz),
			mCellMap(pCellMap)
		{
			unsigned acc = 1;
			for(unsigned sz : mDimSz)
				acc *= sz;
			fast_assert(acc == mCellMap.cells().size());
		}
		
		void packState(State const& s, PackedState& pkd) const {
			slow_assert(mCellMap.isOpen(doPackState(s, mDimSz)));
			pkd = doPackState(s, mDimSz);
		}
		
		void unpackState(State& s, PackedState const& pkd) const {
			s = doUnpackState(pkd, mDimSz);
			slow_assert(mCellMap.isOpen(doPackState(s, mDimSz)));
		}
		
		AdjEdgeIterator getAdjEdges(State& s) const {
			
			return AdjEdgeIterator(s, mDimSz, mCellMap);
		}
		
		size_t hash(PackedState pPacked) const {
			return pPacked;
		}
		
		Cost costHeuristic(State const& pState) const {
			if(MaxK == 1)
				return manhat(pState, mGoalState);
			return 0;
		}
		
		Cost distanceHeuristic(State const& pState) const {
			if(MaxK == 1)
				return manhat(pState, mGoalState);
			return 0;
		}
		
		std::pair<Cost,Cost> pairHeuristics(State const& pState) const {
			return {costHeuristic(pState), distanceHeuristic(pState)};
		}
		
		bool checkGoal(State const& pState) const {
			return pState == mGoalState;
		}
		
		void prettyPrintState(State const& s, std::ostream& out) const {
			doPrettyPrintState(s, out);
		}
		
		private:
		
		State mGoalState;
		std::array<unsigned, N> const& mDimSz;
		CellMapBlocked<> const& mCellMap;
	};
	

	template<unsigned N, unsigned MaxK>
	struct TestDomainStack {
		
		using State = StateN<N>;
		
		static const unsigned Top_Abstract_Level = 0;
		
		template<unsigned L>
		struct Domain : public Domain_base<N, MaxK> {
			Domain(TestDomainStack& pStack) :
				Domain_base<N, MaxK>(pStack.mGoalState, pStack.mDimSz, pStack.mCellMap)
			{}
		};
		
		TestDomainStack(Json const& jConfig) :
			mDimSz(prepDimSz(jConfig.at("dimsz"))),
			mCellMap(mTotCells, jConfig.at("map").get_ref<std::string const&>())
		{
			PackedStateN initpkd = 0;
			while(true) {
				if(mCellMap.isOpen(initpkd))
					break;
				initpkd++;
				if(initpkd == mTotCells)
					gen_assert(false);
			}
			
			mInitState = doUnpackState(initpkd, mDimSz);
			
			PackedStateN goalpkd = mTotCells-1;
			
			while(true) {
				if(mCellMap.isOpen(goalpkd))
					break;
				if(goalpkd == 0)
					gen_assert(false);
				goalpkd--;
			}
			
			mGoalState = doUnpackState(goalpkd, mDimSz);
			
			logDebugStream() << "Init: ";
			doPrettyPrintState(mInitState, g_logDebugOfs);
			g_logDebugOfs << " Goal: ";
			doPrettyPrintState(mGoalState, g_logDebugOfs);
			g_logDebugOfs << "\n";
			g_logDebugOfs.flush();
			
			fast_assert(mCellMap.isOpen(doPackState(mInitState, mDimSz)));
			fast_assert(mCellMap.isOpen(doPackState(mGoalState, mDimSz)));
		}
		
		State getInitState() {
			return mInitState;
		}
		
		std::array<unsigned,N> prepDimSz(std::vector<unsigned> const& pDimSz) {
			std::array<unsigned,N> dimsz;
			mTotCells = 1;
			
			for(unsigned i=0; i<N; i++) {
				dimsz[i] = pDimSz.at(i);
				mTotCells *= dimsz[i];
				fast_assert(dimsz[i] > 0);
			}
			
			return dimsz;
		}
		

		
		void assignInitGoalStates(std::pair<StateN<N>, StateN<N>> const& s) {
			mInitState = s.first;
			mGoalState = s.second;
			fast_assert(mCellMap.isOpen(doPackState(mInitState, mDimSz)));
			fast_assert(mCellMap.isOpen(doPackState(mGoalState, mDimSz)));
			
			logDebugStream() << "Assigned. Init: ";
			doPrettyPrintState(mInitState, g_logDebugOfs);
			g_logDebugOfs << " Goal: ";
			doPrettyPrintState(mGoalState, g_logDebugOfs);
			g_logDebugOfs << "\n";
			g_logDebugOfs.flush();
		}
		
		
		struct CompDistSeparation {
			typename CostType<MaxK>::type operator()(StateN<N> const& a, StateN<N> const& b) {
				return euclid_dist(a, b);
			}
		};
		
		std::pair<StateN<N>, StateN<N>> genRandInitGoal(typename CostType<MaxK>::type pMinCost) {
			std::random_device rd;
			std::mt19937 randgen(rd());
			std::uniform_int_distribution<> ud(0, mTotCells-1);
			
			algorithm::MakeGoalStateAlg<TestDomainStack<N,MaxK>, CompDistSeparation> makeGoalStateAlg(*this);

			
			for(unsigned ntries = 0; ; ntries++) {
				fast_assert(ntries < 100);
				
				StateN<N> initState, goalState;
				
				while(true) {
					PackedStateN initPkd = ud(randgen);
					if(!mCellMap.isOpen(initPkd))
						continue;
					
					initState = doUnpackState(initPkd, mDimSz);
					break;
				}
			
				
				try {
					makeGoalStateAlg.execute(initState, pMinCost);
					goalState = makeGoalStateAlg.getGoalState();
					
				} catch(NoSolutionException const&) {
					continue;
				}
				
				return {initState, goalState};
			}
		}

		
		std::array<unsigned,N> mDimSz;
		unsigned mTotCells;
		CellMapBlocked<> mCellMap;
		
		State mInitState, mGoalState;
	};
	


}}}
