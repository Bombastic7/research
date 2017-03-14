#pragma once


#include <array>
#include <vector>
#include <fstream>
#include <string>
#include <random>
#include <cmath>


namespace mjon661 { namespace gridnav { namespace cube_blocked {



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
	struct AdjEdgeIterator {
		
		AdjEdgeIterator(State const& pState, std::array<unsigned, N> const& pDimsSz) :
			mAdjState(pState),
			mK(1),
			mDimsIncr(0),
			mCurCost(1),
			mDimsSz(pDimsSz)
		{
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
		
		bool next() {
			reverseCurOp(mK);
			return adv();
		}
		
		State& state() {
			return mAdjState;
		}
		
		Cost cost() {
			return mCurCost;
		}
		
		private:
		bool adv() {
			do {
				mDimsIncr++;
				
				if(mDimsIncr == 1u << mK) {
					mDimsIncr = 0;
					
					if(!tryAdvTgtDims()) {
						if(mK == N)
							return false;
						
						mK++;
						mCurCost = std::sqrt(mK);
						
						resetTgtDims();
					}
				}
			}
			while(!applyCurOp());
			return true;
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
					if(mAdjState.at(mTgtDims.at(i)) == mDimsSz-1) {
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
		
		State mAdjState;
		unsigned mK;
		std::array<unsigned, N> mTgtDims;
		unsigned mDimsIncr;
		typename CostType::type mCurCost;
		std::array<unsigned, N> const& mDimsSz;
		
	};
	
	
	
	

	
	

	
	

	
	
	
	template<unsigned N>
	struct CellGraph {
		
		static_assert(N > 0, "");
		
		using Cost = CostType<N>::type;
		using State = std::array<unsigned, N>;
		using PackedState = unsigned;
		
		
		PackedState packState(State const& s) {
			PackedState pkd = 0;
			unsigned rdx = 1;
			
			for(unsigned i=0; i<N; i++) {
				pkd += s[i] * rdx;
				rdx *= mDimSz[i];
			}
			
			return pkd;
		}
		
		State unpackState(PackedState pkd) {
			State s;
			
			rdx = mDimSz[0];
			
			for(unsigned i=0; i<N-1; i++) {
				s[i] = pkd % mDimSz[i+1];
				pkd /= rmDimSz[i+1];
			}
			
			s[N-1] = pkd;
			
			return s;
		}
		
		
		AdjEdgeIterator(
		
		
		struct OperatorSet {
			OperatorSet(CellGraph const& pG, State const& s)
			{
				//mValidMvs.resize(pG.mAdjOpSet.size());

				std::array<bool,N> dimsAtEdge;

				for(unsigned i=0; i<N; i++)
					if(s[i] == 0 || s[i] == pG.mDimSz[i]-1)
						dimsAtEdge[i] = true;
				
				for(unsigned i=0; i<pG.mAdjOpSet.size(); i++) {
					mValidMvs[i] = pG.mAdjOpSet
			}
			
			unsigned size() {
				return mN;
			}
			
			unsigned operator()(unsigned i) {
				return i;
			}
			private:
			unsigned mN;
			std::vector<unsigned> mValidMvs;
		};
		
		
		
		
		
		CellGraph(std::vector<unsigned> const& pDimSz)
		{
			for(unsigned i=0; i<N; i++) {
				mDimSz[i] = pDimSz.at(i);
				fast_assert(mDimSz[i] > 0);
			}
		}
		
		
		
		const AdjOpSet<N> mAdjOpSet;
	};
	
	template<bool Use_LifeCost>
	struct CellGraph_6 {
		public:
		using Cost_t = int;

		
		CellGraph_4(unsigned pHeight, unsigned pWidth, unsigned pDepth, std::string const& pMapFile) :
			CellMap(pHeight*pWidth*pDepth, pMapFile),
			mHeight(pHeight),
			mWidth(pWidth),
			mDepth(pDepth)
		{}

		template<unsigned Sz>
		const AdjacentCells fillAdjacentCells(std::array<unsigned, Sz>& adj) const {
			AdjacentCells adj{.n=0};
			
			Cost_t costMul = Use_LifeCost ? s/mWidth : 1;
			
			if(s >= mWidth && this->isOpen(s-mWidth))
				adj.adjCells[adj.n++] = s-mWidth;
			if(s < (mHeight-1)*mWidth && this->isOpen(s+mWidth))
				adj.adjCells[adj.n++] = s+mWidth;
			if(s%mWidth != 0 && this->isOpen(s-1))
				adj.adjCells[adj.n++] = s-1;
			if((s+1)%mWidth != 0 && this->isOpen(s+1))
				adj.adjCells[adj.n++] = s+1;
			return adj;
		}
		
		Cost_t getMoveCost(unsigned src, unsigned dst) const {
			return Use_LifeCost ? src/mWidth : 1;
		}

		unsigned getHeight() const {
			return mHeight;
		}
		
		unsigned getWidth() const {
			return mWidth;
		}
		
		private:
		const unsigned mHeight, mWidth;
		
	};






}}}
