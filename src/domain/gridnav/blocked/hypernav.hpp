#pragma once


#include <array>
#include <vector>
#include <fstream>
#include <string>
#include <random>
#include <cmath>


namespace mjon661 { namespace gridnav { namespace hypernav_blocked {



	enum struct Cell_t {
		Open, Blocked, Null
	};



	template<typename = void>
	class CellMap {
		
		public:
		
		class OpenCellIterator {
			public:
			
			OpenCellIterator& operator++() {
				if(mIdx == mInst.getSize())
					return *this;
				++mIdx;
				while(true) {
					bool v = mIdx < mInst.getSize();
					if(!v)
						break;
					
					bool o = mInst.isOpen(mIdx);
					if(o)
						break;
					++mIdx;
				}
				/*
				do {
					++mIdx;
				} while(mIdx < mInst.getSize() && !mInst.isOpen(mIdx));
				*/
				return *this;
			}
			
			bool operator==(OpenCellIterator const& o) {
				return mIdx == o.mIdx;
			}
			
			bool operator!=(OpenCellIterator const& o) {
				return mIdx != o.mIdx;
			}
			
			unsigned operator*() {
				return mIdx;
			}
			
			private:
			friend CellMap<void>;
			
			OpenCellIterator(CellMap<void> const& pInst, bool pAtEnd) :
				mInst(pInst),
				mIdx(0)
			{
				slow_assert(pInst.mCells.size() == pInst.getSize());
				if(pAtEnd)
					mIdx = mInst.getSize();
				else
					while(mIdx < mInst.getSize() && !mInst.isOpen(mIdx))
						++mIdx;
			}
			
			CellMap<void> const& mInst;
			unsigned mIdx;
		};
		


		CellMap(unsigned pSize, std::string const& pMapFile) :
			mSize(pSize),
			mCells(mSize)
		{
			if(pMapFile[0] == '.') {
				unsigned seed = std::strtol(pMapFile.c_str()+1, nullptr, 10);
				initRandomMap(seed);
			}
			else if(pMapFile[0] == '-')
				std::fill(mCells.begin(), mCells.end(), Cell_t::Open);
			
			else {
				std::ifstream ifs(pMapFile);
				
				if(!ifs)
					throw std::runtime_error("Could not open map file");

				for(unsigned i=0; i<mSize; i++) {
					int v;
					Cell_t c;
					ifs >> v;
					c = (Cell_t)v;
					
					gen_assert(c == Cell_t::Open || c == Cell_t::Blocked);
					mCells[i] = c;
				}
			}
		}
		
		
		std::vector<Cell_t> const& cells() const {
			return mCells;
		}
		
		unsigned getSize() const {
			return mSize;
		}
		
		bool isOpen(unsigned i) const {
			slow_assert(i < mSize, "%u %u", i, mSize);
			return cells()[i] == Cell_t::Open;
		}
		
		OpenCellIterator begin() const {
			return OpenCellIterator(*this, false);
		}
		
		OpenCellIterator end() const {
			return OpenCellIterator(*this, true);
		}
		
		private:
		
		void initRandomMap(unsigned seed) {
			std::mt19937 gen(5489u + seed);
			std::uniform_real_distribution<double> d(0.0,1.0);
			
			for(unsigned i=0; i<mSize; i++) {
				mCells[i] = d(gen) <= 0.35 ? Cell_t::Blocked : Cell_t::Open;
			}
			
			logDebugStream() << "Random CellMap init. seed=" << seed << ", blockedprob=0.35" << "\n";
		}
		
		const unsigned mSize;
		std::vector<Cell_t> mCells;
	};
	
	
	
	
	
	
	template<unsigned N>
	using StateN = std::array<unsigned, N>;
	
	using PackedStateN = unsigned;
	
	
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
		
		for(unsigned i=0; i<N-1; i++) {
			s[i] = pkd % pDimSz[i];
			pkd /= pDimSz[i];
		}
		
		s[N-1] = pkd;
		
		return s;
	}
	
	
	
	template<unsigned N>
	struct CostType {
		using type = double;
	};
	
	template<>
	struct CostType<1> {
		using type = unsigned;
	};
	
	
	template<>
	struct CostType<2> {
		using type = unsigned;
	};
	
	
	
	
	
	
	
	template<unsigned N>
	struct AdjEdgeIterator_base {
		
		AdjEdgeIterator_base(StateN<N>& pState, std::array<unsigned, N> const& pDimsSz, CellMap<> const& pCellMap) :
			mFinished(false),
			mAdjState(pState),
			mK(1),
			mDimsIncr(0),
			mCurCost(1),
			mDimsSz(pDimsSz),
			mCellMap(pCellMap)
		{
			slow_assert(mCellMap.isOpen(doPackState(mAdjState, mDimsSz)));
			
			resetTgtDims();
			
			if(!applyCurOp())
				adv();
		}
		
		void prettyPrint(std::ostream& out) {
			for(unsigned i=0; i<mK; i++) {
				out << mTgtDims[i];
				if((mDimsIncr >> i) & 1)
					out << "+ ";
				else
					out << "- ";
			}
		}
		
		void next() {
			reverseCurOp(mK);
			adv();
		}
		
		StateN<N>& state() {
			return mAdjState;
		}
		
		typename CostType<N>::type cost() {
			return mCurCost;
		}
		
		bool finished() {
			return mFinished;
		}
		
		private:
		void adv() {
			while(true) {
				mDimsIncr++;
				
				if(mDimsIncr == 1u << mK) {
					mDimsIncr = 0;
					
					if(!tryAdvTgtDims()) {
						if(mK == N) {
							mFinished = true;
							return;
						}
						
						mK++;
						mCurCost = std::sqrt(mK);
						
						resetTgtDims();
					}
				}
				
				if(!applyCurOp())
					continue;
				
				if(!mCellMap.isOpen(doPackState(mAdjState, mDimsSz))) {
					reverseCurOp(mK);
					continue;
				}
				break;
			}
		}
		
		void resetTgtDims() {
			for(unsigned i=0; i<mK; i++)
				mTgtDims[i] = i;
		}
		
		bool tryAdvTgtDims() {
			bool foundMid = false;
			
			for(unsigned i=1; i<mK; i++) {
				if(mTgtDims[i] >= mTgtDims[i-1]+2) {
					foundMid = true;
					mTgtDims[i-1]++;
					
					for(unsigned j=0; j<i-1; j++)
						mTgtDims[j] = j;
					
					break;
				}
			}
			
			if(!foundMid) {
				if(mTgtDims[mK-1] < N-1)
					mTgtDims[mK-1]++;
				else
					return false;
			}
			return true;
		}
		
		void reverseCurOp(unsigned n) {			
			for(unsigned i=0; i<n; i++) {
				bool incr = (mDimsIncr >> i) & 1;

				if(incr)
					mAdjState.at(mTgtDims[i]) -= 1;
				else
					mAdjState.at(mTgtDims[i]) += 1;
			}
		}
		
		bool applyCurOp() {
			unsigned i=0;
			bool failed = false;
			
			for(; i<mK; i++) {
				bool incr = (mDimsIncr >> i) & 1;
				
				if(incr) {
					if(mAdjState.at(mTgtDims.at(i)) == mDimsSz[i]-1) {
						failed = true;
						break;
					}
					mAdjState.at(mTgtDims.at(i)) += 1;
				}
				else {
					if(mAdjState.at(mTgtDims.at(i)) == 0) {
						failed = true;
						break;
					}
					mAdjState.at(mTgtDims.at(i)) -= 1;
				}
			}
			
			if(!failed)
				return true;

			reverseCurOp(i);
			return false;
		}
		
		bool mFinished;
		StateN<N>& mAdjState;
		unsigned mK;
		std::array<unsigned, N> mTgtDims;
		unsigned mDimsIncr;
		typename CostType<N>::type mCurCost;
		std::array<unsigned, N> const& mDimsSz;
		CellMap<> const& mCellMap;
		
	};
	
	
	
	

	
	
	
	template<unsigned N>
	struct Domain_base {
		
		static_assert(N > 0, "");
		
		using Cost = typename CostType<N>::type;
		using State = StateN<N>;
		using PackedState = PackedStateN;
		using AdjEdgeIterator = AdjEdgeIterator_base<N>;
		
		static const bool Is_Perfect_Hash = true;


		Domain_base(State const& pGoalState, std::array<unsigned,N> const& pDimSz, CellMap<> const& pCellMap) :
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
			return AdjEdgeIterator(s, mDimSz, mCellMap);
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
		
		std::pair<Cost,Cost> pairHeuristics(State const&pState) const {
			return {0,0};
		}
		
		bool checkGoal(State const& pState) const {
			return pState == mGoalState;
		}
		
		void prettyPrintState(State const& s, std::ostream& out) const {
			out << "[ ";
			for(unsigned i=0; i<N; i++)
				out << s[i] << " ";
			out << "]";
		}
		
		private:
		
		State mGoalState;
		std::array<unsigned, N> const& mDimSz;
		CellMap<> const& mCellMap;
	};
	

	template<unsigned N>
	struct TestDomainStack {
		
		using State = StateN<N>;
		
		static const unsigned Top_Abstract_Level = 0;
		
		template<unsigned L>
		struct Domain : public Domain_base<N> {
			Domain(TestDomainStack& pStack) :
				Domain_base<N>(pStack.mGoalState, pStack.mDimSz, pStack.mCellMap)
			{}
		};
		
		TestDomainStack(Json const& jConfig) :
			mDimSz(prepDimSz(jConfig.at("dimsz"))),
			mCellMap(mTotCells, jConfig.at("map").get_ref<std::string const&>())
		{
			mInitState.fill(0);
			
			for(unsigned i=0; i<N; i++)
				mGoalState[i] = mDimSz[i]-1;
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
		
		
		
		std::array<unsigned,N> mDimSz;
		unsigned mTotCells;
		CellMap<> mCellMap;
		
		State mInitState, mGoalState;
	};

}}}
