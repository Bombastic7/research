#pragma once

#include <array>
#include <iostream>
#include <random>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <utility>
#include <algorithm>
#include <sstream>

#include "util/debug.hpp"
#include "util/math.hpp"
#include "util/json.hpp"
#include "domain/star_abt.hpp"
#include "util/hash.hpp"


namespace mjon661 { namespace gridnav { namespace dim2 {


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
		
		//??static constexpr Cost_t Diag_Mv_Cost = 1.41421356237309504880168872420969807857;

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
	
	
	
	template<CellGraph_t>
	struct StarAbtStackGenerator {
		StarAbtStack(unsigned pHeight, unsigned pWidth, unsigned pAbtRadius, const std::string& pMapStr) :
			mHeight(pHeight),
			mWidth(pWidth),
			mMapStr(pMapStr),
			mCellGraph(pMapStr),
			mParam_abtRadius(pAbtRadius),
			mDomain(mCellGraph, (unsigned)-1)
		{
			makeLevels();
			fast_assert(mTrns.size() == mEdges.size());
		}	

		void makeLevels() {
			{
				mTrns.push_back(std::vector<unsigned>(mCellMap.size()));
				unsigned c = 0;
				
				for(unsigned i=0; i<mCellMap.size(); i++)
					if(mCellMap.isOpen(i))
						mTrns[i] = c++;

				bool isTrivial = true;
				mEdges.push_back(std::vector<std::vector<unsigned>>(c));
				
				for(unsigned i=0; i<mCellGraph.size(); i++) {
					if(!mCellGraph.isOpen(i))
						continue;
					
					for(auto edgeIt = mDomain.getAdjEdges(i); !edgeIt.finished(); edgeIt.next()) {
						unsigned cost = edgeIt.cost() == 1 ? 1 : 2;
						mEdges.back().at(mTrns[i]).push_back({mTrns[edgeIt.state()], cost});
						isTrivial = false;
					}
				}
				if(isTrivial)
					return;
			}
			
			//mTrns[0]: size=baseHeight*baseWidth. Bijection of base state to its 'normalised' 
			//	version (i.e. [0,basesize-1] to [0, no. of open cells-1]).
			//mEdges[0]: size=no. of open cells. Element i is a vector of edges for normalised state i, with form {dst norm state,cost}.
			
			while(true) {
				mTrns.push_back(std::vector<unsigned>(curEdges.size()));
				std::vector<unsigned>& abtTrns = mTrns.back();
				std::fill(abtTrns.begin(), abtTrns.end(), (unsigned)-1);
				
				std::vector<std::vector<std::pair<unsigned,Cost>>> const& curEdges = mEdges.back();
				
				std::vector<std::pair<unsigned, unsigned>> hubRankVec;
				for(unsigned i=0; i<curEdges.size(); i++)
					hubRankVec.push_back({curEdges[i].size(), i});
				
				std::sort(hubRankVec.begin(), hubRankVec.end(), HubPrioComp());
				
				std::vector<unsigned> singletonStates;
				
				for(unsigned i=0; i<hubRankPrio.size(); i++)
					if(doAssignAbtMapping(hubRankPrio[i].second, i, 0) == 1)
						singletonStates.push_back(hubRankPrio[i].second);
				
				slow_assert(std::find(abtTrns.begin(), abtTrns.end(), (unsigned)-1) == abtTrns.end());
				
				slow_assert(uniqueElements(singletonStates));
				
				for(auto ss : singletonStates) {
					for(auto const& e : curEdges.at(ss)) {
						abtTrns.at(ss) = abtTrns.at(e.first);
						break;
					}
				}
					unsigned nAbtStates = 0;
				{
					std::map<unsigned, unsigned> abtStateRelabel;
					for(unsigned i=0; i<abtTrns.size(); i++) {
						if(abtStateRelabel.count(abtTrns[i]) == 0)
							abtStateRelabel[abtTrns[i]] = nAbtStates++;
						abtTrns[i] = abtStateRelabel.at(abtTrns[i]);
					}
				}
				
				mEdges.push_back(std::vector<std::pair<unsigned,Cost>>(nAbtStates));
				std::vector<std::pair<unsigned,Cost>>& abtEdges = mEdges.back();
				
				bool isTrivial = true;
				
				for(unsigned src=0; src<nAbtStates; src++) {
					for(auto const& e : curEdges.at(src)) {
						unsigned dst = abtTrns[e.first];
						unsigned cst = e.second;
						
						if(src == dst)
							continue;
						
						isTrivial = false;
						
						abtEdges.at(src).push_back({dst, cst});
					}
				}

				if(isTrivial)
					break;
			}
		}
		
		
		
		unsigned doAssignAbtMapping(unsigned s, unsigned a, unsigned curDepth) const {
			if(mTrns.back().at(s) != (unsigned)-1 || curDepth > mParam_abtRadius)
				return 0;
			
			mTrns.back().at(s) = a;
			unsigned c = 1;
			
			for(auto const& e : mEdges.back().at(s))
				c += doAssignAbtMapping(e.first, a, curDepth+1);
			
			return c;
		}
		
		struct HubPrioComp {
			bool operator()(std::pair<unsigned, unsigned> const& a, std::pair<unsigned, unsigned> const& b) const {
				return a.first != b.first ? a.first > b.first : a.second < b.second;
			}
		};
		
		
		
		
		std::vector<std::vector<unsigned>> mTrns;
		std::vector<std::vector<std::vector<std::pair<unsigned,Cost>>>> mEdges;
		
		const unsigned mHeight, mWidth;
		const std::string mMapStr;
		const CellGraph_t mCellGraph;
		const unsigned mParam_abtRadius;
		GridNav_BaseDomain<CellGraph_t> mDomain;
	};
	
	
	template<typename CellGraph_t>
	struct StarAbtStackInfo {
		
		StarAbtStackInfo() {
			mHeight = mWidth = mParam_abtRadius = (unsigned)-1;
		}
		
		StarAbtStackInfo(unsigned pHeight, unsigned pWidth, unsigned pAbtRadius, const std::string& pMapStr) :
			mHeight(pHeight),
			mWidth(pWidth),
			mParam_abtRadius(pAbtRadius)
		{
			StarAbtStackGenerator<CellGraph_t> abtgen(mHeight, mWidth, mParam_abtRadius, pMapStr);
			mTrns = abtgen.mTrns;
			mEdges = abtgen.mEdges;
		}
		
		StarAbtStackInfo(std::string const& pDumpFileStr) {
			Json j;
			std::ifstream ifs(pDumpFilestr);
			fast_assert(ifs);
			j << ifs;
			
			mHeight = j.at("base_height");
			mWidth = j.at("base_width");
			mParam_abtRadius = j.at("abt_radius");
			
			for(unsigned i=0; i<j.at("n_levels"); i++) {
				mTrns.push_back(j.at("trns").at(i).get_ref<std::vector<unsigned>&>());
				
				std::vector<std::vector<unsigned>> const& edgesDst = j.at("edges_dst").at(i).get_ref<std::vector<std::vector<unsigned>>&>();
				std::vector<std::vector<unsigned>> const& edgesCst = j.at("edges_cst").at(i).get_ref<std::vector<std::vector<unsigned>>&>();
		
				mEdges.push_back();
				
				for(unsigned j=0; j<edgesDst.size(); j++) {
					mEdges.back().push_back({edgesDst[j], edgesCst[j]});
				}
			}
		}
		
		unsigned getNlevels() const {
			return mEdges.size();
		}
		
		std::vector<unsigned> const& getTrns(unsigned i) const {
			return mTrns.at(i);
		}
		
		std::vector<std::vector<std::vector<std::pair<unsigned,Cost>>>> const& getEdges(unsigned i) const {
			return mEdges.at(i);
		}
			
		void dumpToFile(std::string const& pOutStr) const {
			Json jOut;
			
			jOut["n_levels"] = getNlevels();
			jOut["base_height"] = mHeight;
			jOut["base_width"] = mWidth;
			jOut["abt_radius"] = mParam_abtRadius;
			
			for(unsigned i=0; i<getNlevels(); i++) {
				jOut["trns"].push_back(mTrns[i]);
				
				jOut["edges"].push_back(Json::array_t);
				for(unsigned s=0; s<mEdges.at(i).size(); s++) {
					jOut.at("edges_dst").back().push_back(Json::array_t);
					jOut.at("edges_cst").back().push_back(Json::array_t);
					
					for(auto const& e : mEdges.at(i).at(s)) {
						jOut.at("edges_dst").back().back().push_back(e.first);
						jOut.at("edges_cst").back().back().push_back(e.second);
					}
				}
			}

			std::ofstream ofs(pOutStr);
			fast_assert(ofs);
			
			ofs << jOut.dump(2);
		}
		
		private:
		std::vector<std::vector<unsigned>> mTrns;
		std::vector<std::vector<std::vector<std::pair<unsigned,Cost>>>> mEdges;
		unsigned mHeight, mWidth;
		unsigned mParam_abtRadius;
	};

	
	//~ template<typename CellGraph_t, unsigned Top_Abt>
	//~ class GridNav_StarAbtStack {
		//~ public:
		//~ using BaseDomain_t = GridNav_BaseDomain<CellGraph_t>;
		//~ using Stack_t = GridNav_StarAbtStack<CellGraph_t, Top_Abt>;		
		
		//~ static const unsigned Top_Abstract_Level = Top_Abt;
		
		//~ static const unsigned Null_Idx = (unsigned)-1;
		
		
		//~ //Abstract domain. For L > mTopUsedAbtLevel, reuse top level state/edges, and have abstractParentState return state as-is.
		//~ template<unsigned L, typename = void>
		//~ struct Domain : public starabt::StarAbtDomain<BaseDomain_t> {
			//~ Domain(Stack_t& pStack) :
				//~ starabt::StarAbtDomain<BaseDomain_t>(
					//~ pStack.mAbtEdges.at(mathutil::min(L, pStack.mTopUsedAbtLevel)), 
					//~ pStack.abstractBaseState(pStack.mGoalState, mathutil::min(L, pStack.mTopUsedAbtLevel))
				//~ ),
				//~ mStack(pStack)
			//~ {}
			
			//~ unsigned abstractParentState(unsigned bs) const {
				//~ if(L > mStack.mTopUsedAbtLevel) {
					//~ logDebug(std::string("Warning, abstracted past last used level. :Level=") + std::to_string(L));
					//~ return bs;
				//~ }
				
				//~ if(L == 1)
					//~ return mStack.mAbtTrns[0][mStack.mBaseTrns[bs]];

				//~ return mStack.mAbtTrns[L-1][bs];
			//~ }
			
			//~ Stack_t& mStack;
		//~ };
		
		//~ template<typename Ign>
		//~ struct Domain<0, Ign> : public BaseDomain_t {
			//~ Domain(Stack_t const& pStack) :
				//~ BaseDomain_t(pStack.mCellGraph, pStack.mGoalState)
			//~ {}
		//~ };
		
		//~ unsigned abstractBaseState(unsigned bs, unsigned lvl) const {
			//~ slow_assert(mCellGraph.isOpen(bs));
			//~ unsigned s = mBaseTrns.at(bs);
			
			//~ for(unsigned i=0; i<lvl; i++)
				//~ s = mAbtTrns.at(i)[s];
			
			//~ return s;
		//~ }
		
		//~ bool baseStatesConnected(unsigned a, unsigned b) const {
			//~ return abstractBaseState(a, mTopUsedAbtLevel) == abstractBaseState(b, mTopUsedAbtLevel);
		//~ }
		
		//~ unsigned getInitState() const {
			//~ return mInitState;
		//~ }
		
		//~ std::pair<unsigned,unsigned> genRandInitAndGoal(unsigned skip) const {
			//~ fast_assert(skip < 10);
			
			//~ std::mt19937 gen;
			//~ std::uniform_int_distribution<unsigned> dh(0, mHeight-1), dw(0, mWidth-1);
			//~ double diagLen = std::hypot(mHeight, mWidth);
			
			//~ unsigned skipcount = 0;

			//~ while(true) {
				//~ unsigned ix = dw(gen), iy = dh(gen), gx = dw(gen), gy = dh(gen);
				//~ unsigned s0 = ix + iy*mWidth, sg = gx + gy*mWidth;
				
				//~ if(std::hypot((double)ix-gx, (double)iy-gy) < diagLen * 0.5) {
					//~ continue;
				//~ }
				
				//~ if(!mCellGraph.isOpen(s0) || !mCellGraph.isOpen(sg)) {
					//~ continue;
				//~ }

				//~ if(!baseStatesConnected(s0, sg)) {
					//~ continue;
				//~ }

				//~ if(skipcount < skip) {
					//~ skipcount++;
					//~ continue;
				//~ }

				//~ return {s0, sg};
			//~ }
		//~ }
		
		//~ void setInitAndGoalStates(unsigned s0, unsigned sg) {
			//~ mInitState = s0;
			//~ mGoalState = sg;
			//~ fast_assert(s0 < mHeight*mWidth && sg < mHeight * mWidth);
		//~ }
		
		//~ unsigned topUsedAbstractLevel() const {
			//~ return mTopUsedAbtLevel;
		//~ }
		
		//~ GridNav_StarAbtStack(Json const& jConfig) :
			//~ mCellGraph(jConfig.at("height"), jConfig.at("width"), jConfig.at("map")),
			//~ mAbtEdges(1),
			//~ mAbtTrns(),
			//~ mBaseTrns(),
			//~ mInitState(Null_Idx),
			//~ mGoalState(Null_Idx),
			//~ mHeight(jConfig.at("height")),
			//~ mWidth(jConfig.at("width")),
			//~ mTopUsedAbtLevel(0)
		//~ {
			//~ BaseDomain_t dom(mCellGraph, Null_Idx);

			//~ starabt::createBaseMap(dom, mBaseTrns, mAbtEdges[0]);
			
			//~ bool isTrivial = false;
			
			//~ //mBaseTrns maps each open base position [between 0 and H*W-1] to a 'normalised' state [0 to (number of open cells)-1].
			//~ //mAbtEdges[0] specifies the edges for these possible normalised states.
			//~ //mAbtTrns is currently empty.
			//~ //Our current TopLevel is 0.
			
			//~ //Each round of the below loop prepares mAbtTrns[TopLevel], which is a many-to-one mapping for each State[TopLevel] to 
			//~ //	a State[TopLevel+1], and prepares mAbtEdges[TopLevel+1].
			
			//~ //Break when mAbtEdges[TopLevel][someTopLevelState].size() == 0, for all someTopLevelState.
			
			//~ while(true) {
				//~ std::vector<std::vector<starabt::GroupEdge<BaseDomain_t>>> abtedges;
				//~ std::vector<unsigned> abttrns;
				
				//~ if(isTrivial)
					//~ logDebug(std::string("Intermediate level is trivial: ") + std::to_string(mTopUsedAbtLevel));
				
				//~ isTrivial = starabt::createAbstractLevel(2, mAbtEdges.back(), abttrns, abtedges);
				
				//~ mAbtEdges.push_back(abtedges);
				//~ mAbtTrns.push_back(abttrns);
				//~ mTopUsedAbtLevel++;
				
				//~ if(!isTrivial && Top_Abstract_Level == mTopUsedAbtLevel) {
					//~ logDebug(std::string("Last level is not trivial, but out of abt levels to use. TopLevel=") + 
								//~ std::to_string(mTopUsedAbtLevel));
					//~ break;
				//~ }
				
				//~ if(isTrivial) {
					//~ logDebug(std::string("Finished with trivial level. TopUsedLevel=") + std::to_string(mTopUsedAbtLevel));
					//~ break;
				//~ }
			//~ }

			//~ logDebug(std::string("gridnav abtstack info hash: ") + std::to_string(hashStackInfo()));
			//~ logDebug(std::string("sizeof mAbtEdges: ") + std::to_string(mAbtEdges.size()));
			
			//~ if(jConfig.at("init") < 0 || jConfig.at("goal") < 0) {
				//~ int skip = - (int)jConfig.at("init");
				//~ fast_assert(skip >= 0);
				//~ auto genStates = genRandInitAndGoal((unsigned)skip);
				
				//~ mInitState = genStates.first;
				//~ mGoalState = genStates.second;
				
				//~ logDebugStream() << "Random init and goal (skip=" << skip << "): "
					//~ << "i: ";
					//~ dom.prettyPrint(mInitState, g_logDebugOfs);
					//~ g_logDebugOfs << "  g: ";
					//~ dom.prettyPrint(mGoalState, g_logDebugOfs);
					//~ g_logDebugOfs << "\n";
			//~ }
			//~ else {
				//~ mInitState = jConfig.at("init");
				//~ mGoalState = jConfig.at("goal");
				//~ logDebugStream() << "Supplied init and goal: "
					//~ << "i: ";
					//~ dom.prettyPrint(mInitState, g_logDebugOfs);
					//~ g_logDebugOfs << "  g: ";
					//~ dom.prettyPrint(mGoalState, g_logDebugOfs);
					//~ g_logDebugOfs << "\n";
			//~ }
			
			//~ fast_assert(baseStatesConnected(mInitState, mGoalState));
		//~ }
		
		
		//~ //Hopefully makes a nice hash value for our dynamically created abstract mapping. Just used for debugging.
		//~ size_t hashStackInfo() const {
			//~ std::vector<unsigned> hashVals;
			
			//~ for(unsigned i=0; i<=mTopUsedAbtLevel; i++)
				//~ for(unsigned j=0; j<mAbtEdges[i].size(); j++)
					//~ for(unsigned k=0; k<mAbtEdges[i][j].size(); k++)
						//~ hashVals.push_back(mAbtEdges[i][j][k].dst);
			
			//~ for(auto it=mBaseTrns.begin(); it!=mBaseTrns.end(); ++it) {
				//~ hashVals.push_back(it->first);
				//~ hashVals.push_back(it->second);
			//~ }
			
			//~ for(unsigned i=0; i<mTopUsedAbtLevel; i++)
				//~ for(unsigned j=0; j<mAbtTrns[i].size(); j++)
					//~ hashVals.push_back(mAbtTrns[i][j]);
			
			//~ return hashfunctions::vec_32(hashVals);
		//~ }
		
		
		//~ CellGraph_t mCellGraph;
		//~ std::vector<std::vector<std::vector<starabt::GroupEdge<BaseDomain_t>>>> mAbtEdges;
		//~ std::vector<std::vector<unsigned>> mAbtTrns;
		//~ std::map<unsigned, unsigned> mBaseTrns;
		//~ unsigned mInitState, mGoalState;
		//~ const unsigned mHeight, mWidth;
		//~ unsigned mTopUsedAbtLevel;
		
	//~ };
}}}
