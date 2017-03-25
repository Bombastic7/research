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

#include "domain/gridnav/hypernav/common.hpp"
#include "domain/gridnav/cellmap_blocked.hpp"


namespace mjon661 { namespace gridnav { namespace hypernav {



	template<unsigned N, unsigned MaxK>
	struct DomainBlocked {
		
		static_assert(N > 0, "");
		static_assert(N >= MaxK, "");
		
		using Cost = typename CostType<MaxK>::type;
		using State = std::array<unsigned, N>;
		using PackedState = unsigned;
		using AdjEdgeIterator = AdjEdgeIterator_base<N, MaxK>;
		
		static const bool Is_Perfect_Hash = true;


		struct AdjEdgeIterator : public AdjEdgeIterator_base<N, MaxK> {
			
			AdjEdgeIterator(	std::array<unsigned, N>& pState,
								mathutil::HypergridMoveSet<N, MaxK> const& pMvSet,
								std::array<unsigned,N> const& pDimsSz,
								CellMapBlocked<> const& pCellMap) :
				AdjEdgeIterator_base<N, MaxK>(pState, pMvSet, pDimsSz),
				mCellMap(pCellMap),
				mAdjPackedState(doPackState(this->mAdjState, this->mDimSz))
			{}
			
			void next() {
				while(true) {
					AdjEdgeIterator_base<N, MaxK>::next();

					if(this->finished())
						break;
					
					mAdjPackedState = doPackState(this->mAdjState, this->mDimSz);
					
					if(!mCellMap.isOpen(mAdjPackedState))
						continue;
					
					break;
				}
			}
			
			typename CostType<MaxK>::type cost() const {
				if(this->mLastK == 1)
					return 1;
				else
					return this->mMvSet.getMoveCost(this->mLastK);
			}
			
			private:
			unsigned mAdjPackedState;
			CellMapBlocked<> const& mCellMap;
		};
		
		

		DomainBlocked(State const& pGoalState, std::array<unsigned,N> const& pDimSz, CellMapBlocked<> const& pCellMap) :
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
			return AdjEdgeIterator(s, mMvSet, mDimSz, mCellMap);
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
		mathutil::CellMapBlocked<> const& mCellMap;
		const HypergridMoveSet<N, MaxK> mMvSet;
	};
	
	
	
	

	template<unsigned N, unsigned MaxK>
	struct DomainBlockedStack {
		
		using State = StateN<N>;
		
		static const unsigned Top_Abstract_Level = 0;
		
		template<unsigned L>
		struct Domain : public DomainBlocked<N, MaxK> {
			Domain(DomainBlockedStack& pStack) :
				DomainBlocked<N, MaxK>(pStack.mGoalState, pStack.mDimSz, pStack.mCellMap)
			{}
		};
		
		DomainBlockedStack(Json const& jConfig) :
			mDimSz(prepDimSz(jConfig.at("dimsz"))),
			mCellMap(mTotCells, jConfig.at("map").get_ref<std::string const&>())
		{
			unsigned initpkd = 0;
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
		
		State getInitState() const {
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
