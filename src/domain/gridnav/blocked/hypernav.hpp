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


namespace mjon661 { namespace gridnav { namespace hypernav_blocked {



	
	
	
		
	template<unsigned N>
	using StateN = std::array<unsigned, N>;
	
	using PackedStateN = unsigned;
	
	
	template<long unsigned N>
	void doPrettyPrintState(StateN<N> const& s, std::ostream& out) {
			out << "[ ";
			for(unsigned i=0; i<N; i++)
				out << s[i] << " ";
			out << "]";
	}

	
	
	template<long unsigned N>
	PackedStateN doPackState(StateN<N> const& s, std::array<unsigned,N> const& pDimSz) {
		PackedStateN pkd = 0;
		unsigned rdx = 1;
		
		for(unsigned i=0; i<N; i++) {
			pkd += s[i] * rdx;
			rdx *= pDimSz[i];
		}
		
		return pkd;
	}
	
	template<long unsigned N>
	StateN<N> doUnpackState(PackedStateN pkd, std::array<unsigned,N> const& pDimSz) {
		StateN<N> s;
		
		for(unsigned i=0; i<N; i++) {
			s[i] = pkd % pDimSz[i];
			pkd /= pDimSz[i];
		}
		
		return s;
	}
	
	template<long unsigned N>
	inline unsigned manhat(StateN<N> const& s, StateN<N> const& g) {
		unsigned d = 0;
		
		for(unsigned i=0; i<N; i++)
			d += std::abs((int)s[i] - (int)g[i]);
		
		return d;
	}
	
	
	template<long unsigned N>
	inline double euclid_dist(StateN<N> const& s, StateN<N> const& g) {
		double acc = 0;
		
		for(unsigned i=0; i<N; i++)
			acc += std::pow((double)s[i] - g[i], 2);
		
		return std::sqrt(acc);
	}
	
	
	template<unsigned K>
	struct CostType {
		using type = double;
	};
	
	template<>
	struct CostType<1> {
		using type = unsigned;
	};	
	
	
	
	
	//Iterates through adjacent states, a state being a position in the N-dimensional cellmap.
	//MaxK is the maximum number of dimensions that can be moved through in a single move. That is, 1 to N inclusive.
	//New states are prepared inplace using the State provided to the ctor. This state cannot be used until finished() returns true,
	//	or dtor called.
	
	//state(), cost() - returns current edge info. Only valid when finished() returns false.
	//finished() - returns false if more adjacent edges can be generated with next().
	//next() - tries to advance to the next adjacent edge. If current edge is the last, instead sets finished() true and restores
	// state given in ctor.
	
	
	//Each move affects [1..MaxK] dimensions, those dimensions can be any combination of [0,1,..,N-1], and the dimensions selected can 
	//	each be either increased or decreased. The maximum possible number of moves a state can have is:
	//SUM{k=1..MaxK} NumMovesK(k),  where NumMovesK(k) = N!/(k!(N-k)!) * 2**k
	
	//The real number of moves is this, less the number of off-the-map positions, and positions corresponding to blocked cells.

	template<unsigned N, unsigned MaxK>
	struct AdjEdgeIterator_base {
		AdjEdgeIterator_base(	StateN<N>& pState,
								std::array<unsigned,N> const& pDimsSz, 
								CellMapBlocked<> const& pCellMap) :
			mDimsSz(pDimsSz),
			mCellMap(pCellMap),
			mCurState(pState),
			mFinished(true)
		{
			mCurK = 1;
			mMaxIncrDim = 2;
			mCurCost = 1;
			
			for(unsigned i=0; i<mCurK; i++)
				mTgtDims[i] = i;
			
			mIncrDim = 0;
			
			while(true) {
				if(applyMv()) {
					if(!mCellMap.isOpen(doPackState(mCurState, mDimsSz))) {
						undoMv();
					} else {
						mFinished = false;
						break;
					}
				}
				
				if(!adv())
					break;
			}
		}
		
		~AdjEdgeIterator_base() {
			if(!mFinished)
				undoMv();
		}
		
		
		bool finished() {
			return mFinished;
		}
		
		StateN<N>& state() {
			return mCurState;
		}
		
		typename CostType<MaxK>::type cost() {
			return mCurCost;
		}
		
		void next() {
			slow_assert(!mFinished);
			
			undoMv();
			
			while(true) {
				if(!adv()) {
					mFinished = true;
					break;
				}
				
				if(applyMv()) {
					if(mCellMap.isOpen(doPackState(mCurState, mDimsSz)))
						return;
					else
						undoMv();
				}
			}
		}
		
		
		bool applyMv() {
			unsigned i = 0;
			bool oob = false;
			
			for(; i<mCurK; i++) {
				bool incr = (mIncrDim >> i) & 1;
				
				if(incr) {
					if(mCurState[mTgtDims[i]] == mDimsSz[mTgtDims[i]]-1) {
						oob = true;
						break;
					}
					mCurState[mTgtDims[i]]++;
				}
				else {
					if(mCurState[mTgtDims[i]] == 0) {
						oob = true;
						break;
					}
					mCurState[mTgtDims[i]]--;
				}
			}
			
			if(oob) {
				for(unsigned j=0; j<i; j++) {
					bool incr = (mIncrDim >> j) & 1;
					
					if(incr)
						mCurState[mTgtDims[j]]--;
					else
						mCurState[mTgtDims[j]]++;
				}
				return false;
			}
			
			return true;
		}
		
		
		void undoMv() {
			for(unsigned j=0; j<mCurK; j++) {
				bool incr = (mIncrDim >> j) & 1;
				
				if(incr)
					mCurState[mTgtDims[j]]--;
				else
					mCurState[mTgtDims[j]]++;
			}
		}
		
		
		bool adv() {
			mIncrDim++;
			
			if(mIncrDim == mMaxIncrDim) {
				mIncrDim = 0;
				
				bool foundMid = false;
		
				for(unsigned i=1; i<mCurK; i++) {
					if(mTgtDims[i] >= mTgtDims[i-1]+2) {
						foundMid = true;
						mTgtDims[i-1]++;
						
						for(unsigned j=0; j<i-1; j++)
							mTgtDims[j] = j;
						
						break;
					}
				}
				
				if(!foundMid) {
					if(mTgtDims[mCurK-1] == N-1) {
						if(mCurK == MaxK)
							return false;
						mCurK++;
						mMaxIncrDim = 1 << mCurK;
						for(unsigned i=0; i<mCurK; i++)
							mTgtDims[i] = i;
						mCurCost = std::sqrt(mCurK);
					}
					else
						mTgtDims[mCurK-1]++;
				}
			}
			
			return true;
		}
		
		
		
		std::array<unsigned,N> const& mDimsSz;
		CellMapBlocked<> const& mCellMap;
		
		StateN<N>& mCurState;
		unsigned mCurK;
		std::array<unsigned, MaxK> mTgtDims;
		unsigned mMaxIncrDim;
		unsigned mIncrDim;
		bool mFinished;
		typename CostType<MaxK>::type mCurCost;
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
