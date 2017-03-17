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
			fast_assert(pMapFile.size() > 0);
			
			if(pMapFile[0] != ',') {
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
				return;
			} 
			
			std::stringstream ss(pMapFile);
			std::string funcStr, t;
			
			std::getline(ss, funcStr, ',');
			std::getline(ss, funcStr, ',');
			
			if(funcStr == "fill") {
				std::fill(mCells.begin(), mCells.end(), Cell_t::Open);
				logDebug("CellMap set to all open.");
			}
			
			else if(funcStr == "random") {
				unsigned seed;
				double prob;
				std::getline(ss, t, ',');
				seed = std::stoul(t);
				std::getline(ss, t, ',');
				prob = std::stod(t);
			
				fast_assert(prob >= 0 && prob <= 1);
				
				initRandomMap(seed, prob);
				logDebugStream() << "Random CellMap init. seed=" << seed << ", blockedprob=" << prob << "\n";
			}
			
			else if(funcStr == "portals") {
				std::getline(ss, t, ',');
				unsigned height = std::stoul(t);
				std::getline(ss, t, ',');
				unsigned width = std::stoul(t);
				std::getline(ss, t, ',');
				unsigned wallInterval = std::stoul(t);

				fast_assert(mSize == height*width);
				
				std::fill(mCells.begin(), mCells.end(), Cell_t::Open);

				std::mt19937 randgen;
				std::uniform_int_distribution<unsigned> ud(0, wallInterval-2);

				for(unsigned x = wallInterval-1; x<width; x+=wallInterval) {
					for(unsigned y=0; y<height; y++)
						mCells.at(x+width*y) = Cell_t::Blocked;
					
					for(unsigned y=0; y<height; y+=wallInterval) {
						unsigned portal_y = y + (ud(randgen) % (height-y));
					
						mCells.at(x+width*portal_y) = Cell_t::Open;
					}
				}
				
				for(unsigned y = wallInterval-1; y<height; y+=wallInterval) {
					for(unsigned x=0; x<width; x++)
						mCells.at(x+width*y) = Cell_t::Blocked;
					
					for(unsigned x=0; x<width; x+=wallInterval) {
						unsigned portal_x = x + (ud(randgen) % (width-x));
					
						mCells.at(portal_x+width*y) = Cell_t::Open;
					}
				}
				
				logDebugStream() << "Portal CellMap init. h=" << height << ", w=" << width << ", wallInterval=" << wallInterval << "\n";
				
			}
			else if(funcStr == "portalsSpanningTree") {
				std::getline(ss, t, ',');
				unsigned height = std::stoul(t);
				std::getline(ss, t, ',');
				unsigned width = std::stoul(t);
				std::getline(ss, t, ',');
				unsigned wallInterval = std::stoul(t);

				fast_assert(mSize == height*width);
				
				std::fill(mCells.begin(), mCells.end(), Cell_t::Open);
				
				for(unsigned x = wallInterval-1; x<width; x+=wallInterval) {
					for(unsigned y=0; y<height; y++)
						mCells.at(x+width*y) = Cell_t::Blocked;
				}
				
				for(unsigned y = wallInterval-1; y<height; y+=wallInterval) {
					for(unsigned x=0; x<width; x++)
						mCells.at(x+width*y) = Cell_t::Blocked;
				}
				
				std::mt19937 randgen;
				std::uniform_int_distribution<unsigned> ud(0, wallInterval-2);
				std::uniform_real_distribution<> edgeCostDist(0, 1);
				
				
				
				
				unsigned spacesX = width/(wallInterval), spacesY = height/(wallInterval);
				if(width % wallInterval != 0)
					spacesX++;
				if(height % wallInterval != 0)
					spacesY++;
					
				logDebug(std::string("spacesX: ") + std::to_string(spacesX) + " spacesY: " + std::to_string(spacesY));
				
				
				using Edge_t = std::tuple<unsigned, unsigned, double>;
				std::vector<Edge_t> spaceEdges;
				
				for(unsigned i=0; i<spacesX*spacesY; i++) {
					unsigned x = i % spacesX, y = i / spacesY;
					
					if(y > 0)
						spaceEdges.push_back(Edge_t(i, i-spacesX, edgeCostDist(randgen)));;
					if(y < spacesY-1)
						spaceEdges.push_back(Edge_t(i, i+spacesX, edgeCostDist(randgen)));
					if(x != 0)
						spaceEdges.push_back(Edge_t(i, i-1, edgeCostDist(randgen)));
					if(x < spacesX-1)
						spaceEdges.push_back(Edge_t(i, i+1, edgeCostDist(randgen)));
				}
				
				std::shuffle(spaceEdges.begin(), spaceEdges.end(), randgen);
				
				for(auto& e : spaceEdges) {
					unsigned src = std::get<0>(e);
					unsigned dst = std::get<1>(e);
					
					unsigned x = (src % spacesX) * wallInterval;
					unsigned y = (src / spacesX) * wallInterval;
					unsigned px = (dst % spacesX) * wallInterval;
					unsigned py = (dst / spacesX) * wallInterval;
					logDebugStream() << "x=" << x << " y=" << y << " px=" << px << " py=" << py << "\n";
				}
				
			
				
				std::vector<std::tuple<unsigned, unsigned, double>> stedges = mathutil::minSpanningTreePrims<double>(spacesX*spacesY, spaceEdges);
				

				for(auto& e : stedges) {
					
					unsigned src = std::get<0>(e);
					unsigned dst = std::get<1>(e);
					
					unsigned x = src % spacesX;
					unsigned y = src / spacesX;
					unsigned px = dst % spacesX;
					unsigned py = dst / spacesX;
					
					logDebugStream() << "x=" << x << " y=" << y << " px=" << px << " py=" << py << "\n";
					g_logDebugOfs.flush();
					
					unsigned wally, wallx;
					wally = wallx = (unsigned)-1;
					
					if(y == py - 1 || y == py + 1) {
						do {
							wallx = x*wallInterval + ud(randgen);
						} while(wallx >= width);
					}
					
					if(x == px - 1 || x == px + 1) {
						do {
							wally = y*wallInterval + ud(randgen);
						} while(wally >= height);
					}
					
					if(y == py - 1) wally = py*wallInterval-1;
					if(y == py + 1) wally = y*wallInterval-1;
					if(x == px - 1) wallx = px*wallInterval-1;
					if(x == px + 1) wallx = x*wallInterval-1;
				
					logDebugStream() << "wallx=" << wallx << " wally=" << wally << "\n";
					mCells.at(wallx + wally*width) = Cell_t::Open;
				}

				logDebugStream() << "PortalMST CellMap init. h=" << height << ", w=" << width << ", wallInterval=" << wallInterval << "\n";
			}
			else
				gen_assert(false);
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
		
		void initRandomMap(unsigned seed, double prob) {
			fast_assert(prob >= 0 && prob <= 1);
			
			std::mt19937 gen(5489u + seed);
			std::uniform_real_distribution<double> d(0.0,1.0);
			
			for(unsigned i=0; i<mSize; i++) {
				mCells[i] = d(gen) < prob ? Cell_t::Blocked : Cell_t::Open;
			}
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
								CellMap<> const& pCellMap) :
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
		CellMap<> const& mCellMap;
		
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
		CellMap<> mCellMap;
		
		State mInitState, mGoalState;
	};
	


}}}
