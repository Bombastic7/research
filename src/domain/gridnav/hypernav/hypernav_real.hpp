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

#include "domain/gridnav/cellmap_real.hpp"
#include "domain/gridnav/hypernav/common.hpp"


namespace mjon661 { namespace gridnav { namespace hypernav {



	template<unsigned N, unsigned MaxK>
	struct DomainReal {
		
		static_assert(N > 0, "");
		static_assert(N >= MaxK, "");
		
		using Cost = double;
		using State = StateN<N>;
		using PackedState = unsigned;
		
		static const bool Is_Perfect_Hash = true;

		struct AdjEdgeIterator : public AdjEdgeIterator_base<N, MaxK> {
			
			AdjEdgeIterator(	std::array<unsigned, N>& pState,
								mathutil::HypergridMoveSet<N, MaxK> const& pMvSet,
								std::array<unsigned,N> const& pDimsSz,
								CellMapReal<double> const& pCellMap) :
				AdjEdgeIterator_base<N, MaxK>(pState, pMvSet, pDimsSz),
				mAdjPackedState(doPackState(this->mAdjState, this->mDimsSz)),
				mCellMap(pCellMap)
			{}
			
			void next() {
				AdjEdgeIterator_base<N, MaxK>::next();
				
				if(this->finished())
					return;

				mAdjPackedState = doPackState(this->mAdjState, this->mDimsSz);
			}
			
			double cost() const {
				return this->mMvSet.getMoveCost(this->mLastK) * mCellMap.cells()[mAdjPackedState];
			}
			
			private:
			unsigned mAdjPackedState;
			CellMapReal<double> const& mCellMap;
		};
		

		DomainReal(State const& pGoalState, std::array<unsigned,N> const& pDimSz, CellMapReal<double> const& pCellMap) :
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
			pkd = doPackState(s, mDimSz);
		}
		
		void unpackState(State& s, PackedState const& pkd) const {
			s = doUnpackState(pkd, mDimSz);
		}
		
		AdjEdgeIterator getAdjEdges(State& s) const {
			return AdjEdgeIterator(s, mMvSet, mDimSz, mCellMap);
		}
		
		size_t hash(PackedState pPacked) const {
			return pPacked;
		}
		
		Cost costHeuristic(State const& pState) const {
			return 0;
		}
		
		Cost distanceHeuristic(State const& pState) const {
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
		CellMapReal<double> const& mCellMap;
		const mathutil::HypergridMoveSet<N, MaxK> mMvSet;
	};
	
	
	
	

	template<unsigned N, unsigned MaxK>
	struct DomainRealStack {
		
		using State = StateN<N>;
		
		static const unsigned Top_Abstract_Level = 0;
		
		template<unsigned L>
		struct Domain : public DomainReal<N, MaxK> {
			Domain(DomainRealStack& pStack) :
				DomainReal<N, MaxK>(pStack.mGoalState, pStack.mDimSz, pStack.mCellMap)
			{}
		};
		
		DomainRealStack(Json const& jConfig) :
			mDimSz(prepDimSz(jConfig.at("dimsz"))),
			mCellMap(mTotCells, jConfig.at("map").get_ref<std::string const&>())
		{
			mInitState = doUnpackState(0, mDimSz);
			mGoalState = doUnpackState(mTotCells-1, mDimSz);
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
			
			logDebugStream() << "Assigned. Init: ";
			doPrettyPrintState(mInitState, g_logDebugOfs);
			g_logDebugOfs << " Goal: ";
			doPrettyPrintState(mGoalState, g_logDebugOfs);
			g_logDebugOfs << "\n";
			g_logDebugOfs.flush();
		}

		
		std::array<unsigned,N> mDimSz;
		unsigned mTotCells;
		CellMapReal<double> mCellMap;
		
		State mInitState, mGoalState;
	};
	


}}}
