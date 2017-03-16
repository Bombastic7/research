#pragma once


#include <array>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
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
			if(pMapFile[0] == ',') {
				initRandomMap(pMapFile);
			}
			else if(pMapFile[0] == '-') {
				std::fill(mCells.begin(), mCells.end(), Cell_t::Open);
				logDebug("CellMap set to all open.");
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
		
		void initRandomMap(std::string const& mapSeedStr) {
			unsigned seed;
			double prob;
			
			std::stringstream ss(mapSeedStr);
			std::string t;
			
			std::getline(ss, t, ',');
			std::getline(ss, t, ',');
			seed = std::stoul(t);
			std::getline(ss, t, ',');
			prob = std::stod(t);
			
			fast_assert(prob >= 0 && prob <= 1);
			
			std::mt19937 gen(5489u + seed);
			std::uniform_real_distribution<double> d(0.0,1.0);
			
			for(unsigned i=0; i<mSize; i++) {
				mCells[i] = d(gen) < prob ? Cell_t::Blocked : Cell_t::Open;
			}
			
			logDebugStream() << "Random CellMap init. seed=" << seed << ", blockedprob=" << prob << "\n";
		}
		
		const unsigned mSize;
		std::vector<Cell_t> mCells;
	};
	
	
		
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
	
	
	template<unsigned K>
	struct CostType {
		using type = double;
	};
	
	template<>
	struct CostType<1> {
		using type = unsigned;
	};	
	
	
	


	template<unsigned N, unsigned MaxK>
	struct AdjStateIterator {
		AdjStateIterator(	StateN<N>& pState,
							std::array<unsigned,N> const& pDimsSz, 
							CellMap<> const& pCellMap) :
			mCurState(pState),
			mDimsSz(pDimsSz),
			mCellMap(pCellMap),
			mFinished(true)
		{
			
			mMaxIncrDim = 2;
			
			for(unsigned i=0; i<K; i++)
				mTgtDims[i] = i;
			
			mIncrDim = 0;
			
			while(true) {
				if(applyMv()) {
					if(!mCellMap.isOpen(doPackState(mCurState))) {
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
		
		~AdjStateIterator() {
			if(!mFinished)
				undoMv();
		}
		
		
		bool finished() {
			return mFinished;
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
					if(mCellMap.isOpen(doPackState(mCurState)))
						return;
					else
						undoMv();
				}
			}
		}
		
		
		bool applyMv() {
			unsigned i = 0;
			bool oob = false;
			
			for(; i<K; i++) {
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
					bool incr = (mIncrDim >> i) & 1;
					
					if(incr)
						mCurState[mTgtDims[i]]--;
					else
						mCurState[mTgtDims[i]]++;
				}
				return false;
			}
		}
		
		
		void undoMv() {
			for(unsigned j=0; j<K; j++) {
				bool incr = (mIncrDim >> i) & 1;
				
				if(incr)
					mCurState[mTgtDims[i]]--;
				else
					mCurState[mTgtDims[i]]++;
			}
		}
		
		
		bool adv() {
			mIncrDim++;
			
			if(mIncrDim == mMaxIncrDim) {
				mIncrDim = 0;
				
				bool foundMid = false;
		
				for(unsigned i=1; i<K; i++) {
					if(mTgtDims[i] >= mTgtDims[i-1]+2) {
						foundMid = true;
						mTgtDims[i-1]++;
						
						for(unsigned j=0; j<i-1; j++)
							mTgtDims[j] = j;
						
						break;
					}
				}
				
				if(!foundMid) {
					if(mTgtDims[K-1] == N-1)
						return false;
					else
						mTgtDims[K-1]++;
				}
			}
			
			return true;
		}
		
		
		
		std::array<unsigned,N> const& mDimsSz;
		CellMap<> const& mCellMap;
		
		
		StateN<N>& mCurState;
		std::array<unsigned, K> mTgtDims;
		unsigned mMaxIncrDim;
		unsigned mIncrDim;
	}
	
	
	
	
	
	
	
	
	template<unsigned N, unsigned MaxK>
	struct AdjEdgeIterator_base {
		template<unsigned> struct Tag;
		
		
		template<unsigned K>
		struct IteratorSet : public IteratorSet<K-1> {
			
		
		
		AdjEdgeIterator_base(	StateN<N> const& pState,
								std::array<unsigned,N> const& pDimsSz, 
								CellMap<> const& pCellMap) :
			mFinished(false),
			mPos(0)
		{
			doFindAdjState_rec(pState, pDimSz, pCellMap, Tag<0>{});
				
			slow_assert(mAdjStates.size() == mEdgeCosts.size());
			
			if(mAdjStates.size() == 0)
				mFinished = true;
		}
		
		bool finished() {
			return mFinished;
		}
		
		void next() {
			slow_assert(!mFinished);
			mPos++;
			if(mPos == mAdjStates.size())
				mFinished = true;
		}
		
		StateN<N> const& state() {
			return mAdjStates[mPos];
		}
		
		typename CostType::type cost() {
			return mEdgeCosts[mPos];
		}

		private:
		
		
		
		template<unsigned
		void adv() {
			
			
			while(true) {
			
				bool oob = false;
				unsigned i = 0;
				
				for(; i<K; i++) {
					bool incr = (incrDim >> i) & 1;
					
					if(incr) {
						if(s[tgtDims[i]] == pDimsSz[tgtDims[i]]-1) {
							oob = true;
							break;
						}
						s[tgtDims[i]]++;
					}
					else {
						if(s[tgtDims[i]] == 0) {
							oob = true;
							break;
						}
						s[tgtDims[i]]--;
					}
				}
				
				if(oob) {
					for(unsigned j=0; j<i; j++) {
						bool incr = (incrDim >> i) & 1;
						
						if(incr)
							s[tgtDims[i]]--;
						else
							s[tgtDims[i]]++;
					}
					continue;
				}

				if(pCellMap.isOpen(doPackState(s, pDimsSz))
					mAdjStates.push_back(s);
				
				for(unsigned i=0; i<K; i++) {
					bool incr = (incrDim >> i) & 1;
				
					if(incr)
						s[tgtDims[i]]--;
					else
						s[tgtDims[i]]++;
				}
				
				
				incrDim++;
				
				if(incrDim == maxIncrDim) {
					incrDim = 0;
					
					bool foundMid = false;
			
					for(unsigned i=1; i<K; i++) {
						if(mTgtDims[i] >= mTgtDims[i-1]+2) {
							foundMid = true;
							mTgtDims[i-1]++;
							
							for(unsigned j=0; j<i-1; j++)
								mTgtDims[j] = j;
							
							break;
						}
					}
					
					if(!foundMid) {
						if(mTgtDims[K-1] == N-1)
							break;
						else
							mTgtDims[K-1]++;
					}
				}
			}
			
			
			
		}
		
		
		
		
		
		
		void doFindAdjState_rec(StateN<N> const& pState,
								std::array<unsigned,N> const& pDimsSz, 
								CellMap<> const& pCellMap,
								Tag<MaxK>)
		{
			unsigned prevSz = mEdgeCosts.size();
			findAdjStates<N,MaxK>(pState, mEdgeCosts, pDimsSz, pCellMap);
			mEdgeCosts.resize(mAdjStates.size());
			std::fill(mEdgeCosts.begin() + prevSz, mEdgeCosts.end(), std::sqrt(K));
			
		}
		
		template<unsigned K>
		void doFindAdjState_rec(StateN<N> const& pState,
								std::array<unsigned,N> const& pDimsSz, 
								CellMap<> const& pCellMap,
								Tag<K>)
		{
			unsigned prevSz = mEdgeCosts.size();
			findAdjStates<N,K>(pState, mAdjStates, pDimsSz, pCellMap);
			mEdgeCosts.resize(mAdjStates.size());
			std::fill(mEdgeCosts.begin() + prevSz, mEdgeCosts.end(), std::sqrt(K));
			doFindAdjState_rec(pState, pDimsSz, pCellMap, Tag<K+1>);
			
		}
		
		
		const unsigned mMaxIncrDim;
		std::array<unsigned, K> mTgtDims;
		unsigned mIncrDim;
		
		
		std::vector<StateN<N>> mAdjStates;
		std::vector<typename CostType<MaxK>::type> mEdgeCosts;
		bool mFinished;
		unsigned mPos;
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
		CellMap<> const& mCellMap;
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
		
		
		
		std::array<unsigned,N> mDimSz;
		unsigned mTotCells;
		CellMap<> mCellMap;
		
		State mInitState, mGoalState;
	};

}}}
