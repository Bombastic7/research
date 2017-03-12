#pragma once

#include <array>
#include <iostream>
#include <random>
#include <vector>
#include <cmath>
#include <queue>
#include <string>
#include <fstream>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <cstdlib>
#include <random>

#include "util/debug.hpp"
#include "util/math.hpp"
#include "util/json.hpp"
#include "domain/star_abt.hpp"
#include "util/hash.hpp"

#include <cstdio>

namespace mjon661 { namespace gridnav { namespace blocked {


	enum struct Cell_t {
		Open, Blocked, Null
	};
	
	enum struct CardDir_t {
		N, S, W, E, NW, NE, SW, SE
	};
	


	inline unsigned manhat(unsigned pState, unsigned pGoal, unsigned pWidth) {
		int x = pState % pWidth, y = pState / pWidth;
		int gx = pGoal % pWidth, gy = pGoal / pWidth;
		return std::abs(gx - x) + std::abs(gy - y);
	}

	/* returns  sum of contiguous rows from pY to {mGoaly-1 / mGoaly+1} if {pY < mGoalY / pY > mGoalY} */
	inline unsigned verticalPathFactor(int pY, int goaly) {
		int d = std::abs(goaly - pY);

		if(d == 0)
			return 0;

		int s = (d * (d-1)) / 2;
		
		s += pY < goaly ? 
				 pY * d :
			(goaly+1) * d;

		return s;
	}
	
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
	
	
	
	template<bool Use_LifeCost>
	class CellGraph_4 : public CellMap<> {
		public:
		using Cost_t = int;

		struct AdjacentCells {
			unsigned n;
			std::array<unsigned, 4> adjCells;
		};
		
		CellGraph_4(unsigned pHeight, unsigned pWidth, std::string const& pMapFile) :
			CellMap(pHeight*pWidth, pMapFile),
			mHeight(pHeight),
			mWidth(pWidth)
		{}

		const AdjacentCells getAdjacentCells(unsigned s) const {
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
	
	
	template<bool Use_LifeCost>
	class CellGraph_8 : public CellMap<> {
		using Cost_t = float;
		
		static constexpr Cost_t Diag_Mv_Cost = 1.41421356237309504880168872420969807857;

		struct AdjacentCells {
			unsigned n;
			std::array<unsigned, 8> adjCells;
		};
		
		CellGraph_8(unsigned pHeight, unsigned pWidth, std::string const& pMapFile) :
			CellMap(pHeight*pWidth, pMapFile),
			mHeight(pHeight),
			mWidth(pWidth)
		{}

		const AdjacentCells getAdjacentCells(unsigned s) const {
			AdjacentCells adj{.n=0};

			bool vn=false, vs=false, ve=false, vw=false;
			
			if(s >= mWidth && this->isOpen(s-mWidth)) {
				adj.adjCells[adj.n++] = s-mWidth;
				vn = true;
			}
			if(s < (mHeight-1)*mWidth && this->isOpen(s+mWidth)) {
				adj.adjCells[adj.n++] = s+mWidth;
				vs = true;
			}
			if(s%mWidth != 0 && this->isOpen(s-1)) {
				adj.adjCells[adj.n++] = s-1;
				vw = true;
			}
			if((s+1)%mWidth != 0 && this->isOpen(s+1)) {
				adj.adjCells[adj.n++] = s+1;
				ve = true;
			}
			if(vn && vw && this->isOpen(s-mWidth-1))
				adj.adjCells[adj.n++] = s-mWidth-1;
			if(vn && ve && this->isOpen(s-mWidth+1))
				adj.adjCells[adj.n++] = s-mWidth+1;
			if(vs && vw && this->isOpen(s+mWidth-1))
				adj.adjCells[adj.n++] = s+mWidth-1;
			if(vs && ve && this->isOpen(s+mWidth+1))
				adj.adjCells[adj.n++] = s+mWidth+1;
			return adj;
		}
		
		Cost_t getMoveCost(unsigned src, unsigned dst) const {
			Cost_t cst = Use_LifeCost ? src/mWidth : 1;

			if(src%mWidth != dst%mWidth && src/mWidth != dst/mWidth)
				cst *= Diag_Mv_Cost;
			
			return cst;
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
	
	
	template<bool Use_LifeCost, bool Use_Hr>
	class CellGraph_4_hr : public CellGraph_4<Use_LifeCost> {
		public:
		using typename CellGraph_4<Use_LifeCost>::Cost_t;
		
		CellGraph_4_hr(unsigned pHeight, unsigned pWidth, std::string const& pMapFile) :
			CellGraph_4<Use_LifeCost>(pHeight, pWidth, pMapFile)
		{}
		
		void compHrVals(unsigned pPos, unsigned pGoal, Cost_t& out_h, Cost_t& out_d) const {
			if(!Use_Hr) {
				out_h = 0;
				out_d = 0;
				return;
			}
			
			if(Use_LifeCost)
				return lifeCostHeuristics(pPos, pGoal, out_h, out_d);
			return unitCostHeuristics(pPos, pGoal, out_h, out_d);
		}
		
		private:
		void unitCostHeuristics(unsigned pPos, unsigned pGoal, Cost_t& out_h, Cost_t& out_d) const {
			out_h = out_d = manhat(pPos, pGoal, this->getWidth());
		}
		
		void lifeCostHeuristics(unsigned pPos, unsigned pGoal, Cost_t& out_h, Cost_t& out_d) const {
			unsigned pWidth = this->getWidth();
			
			int x = pPos % pWidth, y = pPos / pWidth;
			int gx = pGoal % pWidth, gy = pGoal / pWidth;
			
			int dx = std::abs(x-gx), miny = mathutil::min(y, gy);
			
			// Horizontal segment at the cheaper of y/gy. Vertical segment straight from y to goaly.
			int p1 = dx * miny + verticalPathFactor(y, gy);
			
			// From (x,y) to (x,0), then to (gx, 0), then to (gx, gy). Note that horizontal segment is free (row 0).
			int p2 = verticalPathFactor(y, 0) + verticalPathFactor(0, gy);
			
			if(p1 < p2) {
				out_h = p1;
				out_d = dx + std::abs(y - gy);
			} else {
				out_h = p2;
				out_d = dx + y + gy;
			}
		}
	};
	

	template<bool Use_LifeCost, bool Use_Hr>
	class CellGraph_8_hr : public CellGraph_8<Use_LifeCost> {
		public:
		using typename CellGraph_8<Use_LifeCost>::Cost_t;
		using CellGraph_8<Use_LifeCost>::Diag_Mv_Cost;
		
		CellGraph_8_hr(unsigned pHeight, unsigned pWidth, std::string const& pMapFile) :
			CellGraph_8<Use_LifeCost>(pHeight, pWidth, pMapFile)
		{}
		
		void compHrVals(unsigned pPos, unsigned pGoal, Cost_t& out_h, Cost_t& out_d) const {
			if(!Use_Hr) {
				out_h = 0;
				out_d = 0;
				return;
			}
			
			if(Use_LifeCost)
				return lifeCostHeuristics(pPos, pGoal, out_h, out_d);
			return unitCostHeuristics(pPos, pGoal, out_h, out_d);
		}
		
		private:
		
		void unitCostHeuristics(unsigned pPos, unsigned pGoal, Cost_t& out_h, Cost_t& out_d) const {
			unsigned pWidth = this->getWidth();
			
			int dx = std::abs(pPos % pWidth - pGoal % pWidth), dy = std::abs(pPos / pWidth - pGoal / pWidth);
				
			out_h = std::abs(dx-dy) + mathutil::min(dx, dy) * Diag_Mv_Cost;
			out_d = mathutil::max(dx, dy);
		}
		
		void lifeCostHeuristics(unsigned pPos, unsigned pGoal, Cost_t& out_h, Cost_t& out_d) const {
			unsigned pWidth = this->getWidth();
			
			int x = pPos % pWidth, y = pPos / pWidth;
			int gx = pGoal % pWidth, gy = pGoal / pWidth;
			
			int dx = std::abs(x - gx);
			int dy = std::abs(y - gy);
			
			if(dx <= dy) {
				out_h = verticalPathFactor(pPos, gy);
				out_d = dy;
				return;
			}
			
			//int maxdown = min(y, mGoaly);
			int extra = dx - dy;
			
			int down = mathutil::min(mathutil::min(y, gy), (dx-dy)/2);
			int botRow = mathutil::min(y, y) - down;
			int across = extra - 2*down;
			
			out_h = verticalPathFactor(y, botRow) + across * botRow + verticalPathFactor(botRow, gy);
			out_d = dx;
		}
	};



	template<unsigned Nways, bool Use_LifeCost, bool Use_Hr>
	class CellGraph_;
	
	template<bool Use_LifeCost, bool Use_Hr>
	class CellGraph_<4, Use_LifeCost, Use_Hr> : public CellGraph_4_hr<Use_LifeCost, Use_Hr> {
		using CellGraph_4_hr<Use_LifeCost, Use_Hr>::CellGraph_4_hr;
	};

	template<bool Use_LifeCost, bool Use_Hr>
	class CellGraph_<8, Use_LifeCost, Use_Hr> : public CellGraph_8_hr<Use_LifeCost, Use_Hr> {
		using CellGraph_8_hr<Use_LifeCost, Use_Hr>::CellGraph_8_hr;
	};
	
	template<unsigned Nways, bool Use_LifeCost, bool Use_Hr>
	class CellGraph : public CellGraph_<Nways, Use_LifeCost, Use_Hr> {
		using CellGraph_<Nways, Use_LifeCost, Use_Hr>::CellGraph_;
	};
	
	
	template<typename CellGraph_t>
	class GridNav_BaseDomain {
		public:
		using State = unsigned;
		using PackedState = unsigned;
		using Cost = typename CellGraph_t::Cost_t;
		using Operator = unsigned;
		
		static const bool Is_Perfect_Hash = true;
		
		struct OperatorSet : public CellGraph_t::AdjacentCells {
			OperatorSet(typename CellGraph_t::AdjacentCells const& o) :
				CellGraph_t::AdjacentCells(o)
			{}
			
			unsigned size() {
				return this->n;
			}
			
			Operator const& operator[](unsigned i) {
				return this->adjCells[i];
			}
		};
		
		struct Edge {
			Edge(unsigned pState, Cost pCost, unsigned pParentOp) :
				mState(pState),
				mCost(pCost),
				mParentOp(pParentOp)
			{}
			
			unsigned& state() {
				return mState;
			}
			
			Cost cost() {
				return mCost;
			}
			
			unsigned parentOp() {
				return mParentOp;
			}
			
			unsigned mState;
			const Cost mCost;
			unsigned mParentOp;
		};
		
		
		GridNav_BaseDomain(CellGraph_t const& pCellGraph, unsigned pGoalState) :
			mCellGraph(pCellGraph),
			mGoalState(pGoalState)
		{}
		
		Operator getNoOp() const {
			return (unsigned)-1;
		}
		
		State getGoalState() const {
			return mGoalState;
		}

		void packState(State const& pState, PackedState& pPacked) const {
			pPacked = pState;
		}
		
		void unpackState(State& pState, PackedState const& pPacked) const {
			pState = pPacked;
		}
		
		Edge createEdge(State pState, Operator op) const {
			return Edge(op, mCellGraph.getMoveCost(pState, op), pState);
		}
		
		void destroyEdge(Edge&) const {
		}
		
		OperatorSet createOperatorSet(unsigned pState) const {
			return OperatorSet(mCellGraph.getAdjacentCells(pState));
		}
		
		size_t hash(PackedState pPacked) const {
			return pPacked;
		}
		
		Cost costHeuristic(unsigned pState) const {
			Cost h, d;
			mCellGraph.compHrVals(pState, mGoalState, h, d);
			return h;
		}
		
		Cost distanceHeuristic(unsigned pState) const {
			Cost h, d;
			mCellGraph.compHrVals(pState, mGoalState, h, d);
			return d;
		}
		
		std::pair<Cost, Cost> pairHeuristics(unsigned pState) const {
			Cost h, d;
			mCellGraph.compHrVals(pState, mGoalState, h, d);
			return std::pair<Cost, Cost>(h, d);
		}
		
		bool checkGoal(unsigned pState) const {
			return pState == mGoalState;
		}

		bool compare(unsigned a, unsigned b) const {
			return a == b;
		}

		//~ bool compare(PackedState const& a, PackedState const& b) const {
			//~ return a == b;
		//~ }
		
		void prettyPrint(State const& s, std::ostream& out) const {
			out << "[" << s << ", " << s%mCellGraph.getWidth() << ", " << s/mCellGraph.getWidth() << "]";
		}
		
		//~ void prettyPrint(Operator const& op, std::ostream &out) const {

		//~ }
		
		
		using StateIterator = typename CellGraph_t::OpenCellIterator;
		
		StateIterator stateBegin() const {
			return mCellGraph.begin();
		}
		
		StateIterator stateEnd() const {
			return mCellGraph.end();
		}
		

		private:
		CellGraph_t const& mCellGraph;
		const unsigned mGoalState;
	};
	
	
	//Params:
	//	height=[>0], width=[>0]
	//	map= filename, or ".<n>"
	//	init=[0 ... height*width-1], or -<n>
	//	goal=[0 ... height*width-1], or ignored if init is < 0.
	
	
	template<typename CellGraph_t, unsigned Top_Abt>
	class GridNav_StarAbtStack {
		public:
		using BaseDomain_t = GridNav_BaseDomain<CellGraph_t>;
		using Stack_t = GridNav_StarAbtStack<CellGraph_t, Top_Abt>;		
		
		static const unsigned Top_Abstract_Level = Top_Abt;
		
		static const unsigned Null_Idx = (unsigned)-1;
		
		
		//Abstract domain. For L > mTopUsedAbtLevel, reuse top level state/edges, and have abstractParentState return state as-is.
		template<unsigned L, typename = void>
		struct Domain : public starabt::StarAbtDomain<BaseDomain_t> {
			Domain(Stack_t& pStack) :
				starabt::StarAbtDomain<BaseDomain_t>(
					pStack.mAbtEdges.at(mathutil::min(L, pStack.mTopUsedAbtLevel)), 
					pStack.abstractBaseState(pStack.mGoalState, mathutil::min(L, pStack.mTopUsedAbtLevel))
				),
				mStack(pStack)
			{}
			
			unsigned abstractParentState(unsigned bs) const {
				if(L > mStack.mTopUsedAbtLevel) {
					logDebug(std::string("Warning, abstracted past last used level. :Level=") + std::to_string(L));
					return bs;
				}
				
				if(L == 1)
					return mStack.mAbtTrns[0][mStack.mBaseTrns[bs]];

				return mStack.mAbtTrns[L-1][bs];
			}
			
			Stack_t& mStack;
		};
		
		template<typename Ign>
		struct Domain<0, Ign> : public BaseDomain_t {
			Domain(Stack_t const& pStack) :
				BaseDomain_t(pStack.mCellGraph, pStack.mGoalState)
			{}
		};
		
		unsigned abstractBaseState(unsigned bs, unsigned lvl) const {
			slow_assert(mCellGraph.isOpen(bs));
			unsigned s = mBaseTrns.at(bs);
			
			for(unsigned i=0; i<lvl; i++)
				s = mAbtTrns.at(i)[s];
			
			return s;
		}
		
		bool baseStatesConnected(unsigned a, unsigned b) const {
			return abstractBaseState(a, mTopUsedAbtLevel) == abstractBaseState(b, mTopUsedAbtLevel);
		}
		
		unsigned getInitState() const {
			return mInitState;
		}
		
		std::pair<unsigned,unsigned> genRandInitAndGoal(unsigned skip) const {
			fast_assert(skip < 10);
			
			std::mt19937 gen;
			std::uniform_int_distribution<unsigned> dh(0, mHeight-1), dw(0, mWidth-1);
			double diagLen = std::hypot(mHeight, mWidth);
			
			unsigned skipcount = 0;

			while(true) {
				unsigned ix = dw(gen), iy = dh(gen), gx = dw(gen), gy = dh(gen);
				unsigned s0 = ix + iy*mWidth, sg = gx + gy*mWidth;
				
				if(std::hypot((double)ix-gx, (double)iy-gy) < diagLen * 0.5) {
					continue;
				}
				
				if(!mCellGraph.isOpen(s0) || !mCellGraph.isOpen(sg)) {
					continue;
				}

				if(!baseStatesConnected(s0, sg)) {
					continue;
				}

				if(skipcount < skip) {
					skipcount++;
					continue;
				}

				return {s0, sg};
			}
		}
		
		void setInitAndGoalStates(unsigned s0, unsigned sg) {
			mInitState = s0;
			mGoalState = sg;
			fast_assert(s0 < mHeight*mWidth && sg < mHeight * mWidth);
		}
		
		
		GridNav_StarAbtStack(Json const& jConfig) :
			mCellGraph(jConfig.at("height"), jConfig.at("width"), jConfig.at("map")),
			mAbtEdges(1),
			mAbtTrns(),
			mBaseTrns(),
			mInitState(Null_Idx),
			mGoalState(Null_Idx),
			mHeight(jConfig.at("height")),
			mWidth(jConfig.at("width")),
			mTopUsedAbtLevel(0)
		{
			BaseDomain_t dom(mCellGraph, Null_Idx);

			starabt::createBaseMap(dom, mBaseTrns, mAbtEdges[0]);
			
			bool isTrivial = false;
			
			//mBaseTrns maps each open base position [between 0 and H*W-1] to a 'normalised' state [0 to (number of open cells)-1].
			//mAbtEdges[0] specifies the edges for these possible normalised states.
			//mAbtTrns is currently empty.
			//Our current TopLevel is 0.
			
			//Each round of the below loop prepares mAbtTrns[TopLevel], which is a many-to-one mapping for each State[TopLevel] to 
			//	a State[TopLevel+1], and prepares mAbtEdges[TopLevel+1].
			
			//Break when mAbtEdges[TopLevel][someTopLevelState].size() == 0, for all someTopLevelState.
			
			while(true) {
				std::vector<std::vector<starabt::GroupEdge<BaseDomain_t>>> abtedges;
				std::vector<unsigned> abttrns;
				
				if(isTrivial)
					logDebug(std::string("Intermediate level is trivial: ") + std::to_string(mTopUsedAbtLevel));
				
				isTrivial = starabt::createAbstractLevel(2, mAbtEdges.back(), abttrns, abtedges);
				
				mAbtEdges.push_back(abtedges);
				mAbtTrns.push_back(abttrns);
				mTopUsedAbtLevel++;
				
				std::cout << mTopUsedAbtLevel << "\n";
				//g_logDebugOfs.flush();
				
				if(!isTrivial && Top_Abstract_Level == mTopUsedAbtLevel) {
					logDebug(std::string("Last level is not trivial, but out of abt levels to use. TopLevel=") + 
								std::to_string(mTopUsedAbtLevel));
					break;
				}
				
				if(isTrivial) {
					logDebug(std::string("Finished with trivial level. TopUsedLevel=") + std::to_string(mTopUsedAbtLevel));
					break;
				}
			}

			logDebug(std::string("gridnav abtstack info hash: ") + std::to_string(hashStackInfo()));
			logDebug(std::string("sizeof mAbtEdges: ") + std::to_string(mAbtEdges.size()));
			
			if(jConfig.at("init") < 0 || jConfig.at("goal") < 0) {
				int skip = - (int)jConfig.at("init");
				fast_assert(skip >= 0);
				auto genStates = genRandInitAndGoal((unsigned)skip);
				
				mInitState = genStates.first;
				mGoalState = genStates.second;
				
				logDebugStream() << "Random init and goal (skip=" << skip << "): "
					<< "i: ";
					dom.prettyPrint(mInitState, g_logDebugOfs);
					g_logDebugOfs << "  g: ";
					dom.prettyPrint(mGoalState, g_logDebugOfs);
					g_logDebugOfs << "\n";
			}
			else {
				mInitState = jConfig.at("init");
				mGoalState = jConfig.at("goal");
				logDebugStream() << "Supplied init and goal: "
					<< "i: ";
					dom.prettyPrint(mInitState, g_logDebugOfs);
					g_logDebugOfs << "  g: ";
					dom.prettyPrint(mGoalState, g_logDebugOfs);
					g_logDebugOfs << "\n";
			}
			
			fast_assert(baseStatesConnected(mInitState, mGoalState));
		}
		
		
		//Hopefully makes a nice hash value for our dynamically created abstract mapping. Just used for debugging.
		size_t hashStackInfo() const {
			std::vector<unsigned> hashVals;
			
			for(unsigned i=0; i<=mTopUsedAbtLevel; i++)
				for(unsigned j=0; j<mAbtEdges[i].size(); j++)
					for(unsigned k=0; k<mAbtEdges[i][j].size(); k++)
						hashVals.push_back(mAbtEdges[i][j][k].dst);
			
			for(auto it=mBaseTrns.begin(); it!=mBaseTrns.end(); ++it) {
				hashVals.push_back(it->first);
				hashVals.push_back(it->second);
			}
			
			for(unsigned i=0; i<mTopUsedAbtLevel; i++)
				for(unsigned j=0; j<mAbtTrns[i].size(); j++)
					hashVals.push_back(mAbtTrns[i][j]);
			
			return hashfunctions::vec_32(hashVals);
		}
		
		
		CellGraph_t mCellGraph;
		std::vector<std::vector<std::vector<starabt::GroupEdge<BaseDomain_t>>>> mAbtEdges;
		std::vector<std::vector<unsigned>> mAbtTrns;
		std::map<unsigned, unsigned> mBaseTrns;
		unsigned mInitState, mGoalState;
		const unsigned mHeight, mWidth;
		unsigned mTopUsedAbtLevel;
		
	};
}}}
