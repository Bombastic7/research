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
#include <functional>
#include <map>

#include "util/debug.hpp"
#include "util/math.hpp"
#include "util/json.hpp"
#include "util/hash.hpp"

#include "domain/gridnav/cellmap_blocked.hpp"


namespace mjon661 { namespace gridnav { namespace dim2 {



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

	
	
	template<bool Use_LifeCost>
	class CellGraph_4 : public CellMapBlocked<> {
		public:
		using Cost_t = int;

		static std::string costToString(Cost_t c) {
			return std::to_string(c);
		}
		
		static Cost_t costFromString(std::string const& s) {
			return std::stoul(s);
		}
		
		struct AdjacentCells {
			unsigned n;
			std::array<unsigned, 4> adjCells;
		};
		
		CellGraph_4(unsigned pHeight, unsigned pWidth, std::string const& pMapFile) :
			CellMapBlocked<>(pHeight*pWidth, pMapFile),
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
	struct CellGraph_8 : public CellMapBlocked<> {
		//using Cost_t = float;
		
		//??static constexpr Cost_t Diag_Mv_Cost = 1.41421356237309504880168872420969807857;

		struct Cost_t {
			unsigned short dg, st;
		};
		
		static std::string costToString(Cost_t c) {
			return std::to_string(c.dg) + "," + std::to_string(c.st);
		}
		
		static Cost_t costFromString(std::string const& s) {
			Cost_t c;
			char* p;
			c.dg = std::strtol(s.c_str(), &p, 10);
			c.st = std::strtol(p+1, &p, 10);
		}
	
		struct AdjacentCells {
			unsigned n;
			std::array<unsigned, 8> adjCells;
		};
		
		CellGraph_8(unsigned pHeight, unsigned pWidth, std::string const& pMapFile) :
			CellMapBlocked<>(pHeight*pWidth, pMapFile),
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
			unsigned row = Use_LifeCost ? src/mWidth : 1;

			if(src%mWidth != dst%mWidth && src/mWidth != dst/mWidth)
				return {row,0};
			
			return {0,row};
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
	
	std::ostream& operator<<(std::ostream& out, typename CellGraph_8<false>::Cost_t const& cst) {
		out << "(" << cst.dg << "," << cst.st << ")";
		return out;
	}
	
	
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
				out_h = {0,0};
				out_d = {0,0};
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
				
			out_h = {mathutil::min(dx, dy), std::abs(dx-dy)};
			out_d = {0, mathutil::max(dx, dy)};
		}
		
		void lifeCostHeuristics(unsigned pPos, unsigned pGoal, Cost_t& out_h, Cost_t& out_d) const {
			unsigned pWidth = this->getWidth();
			
			int x = pPos % pWidth, y = pPos / pWidth;
			int gx = pGoal % pWidth, gy = pGoal / pWidth;
			
			int dx = std::abs(x - gx);
			int dy = std::abs(y - gy);
			
			if(dx <= dy) {
				out_h = {0, verticalPathFactor(pPos, gy)};
				out_d = {0, dy};
				return;
			}
			
			//int maxdown = min(y, mGoaly);
			int extra = dx - dy;
			
			int down = mathutil::min(mathutil::min(y, gy), (dx-dy)/2);
			int botRow = mathutil::min(y, y) - down;
			int across = extra - 2*down;
			
			out_h = {0, verticalPathFactor(y, botRow) + across * botRow + verticalPathFactor(botRow, gy)};
			out_d = {0, dx};
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
	
	
	
	

	//CellGraph contains all info about the base gridnav level. Inherits from CellMapBlocked, and the base classes for 4/8 way movement.
	//	e.g. if a cell is open or closed, heuristics, adjacent edges for open cells.
	//	Does not contain any info about initial and goal states, or anything to do with abstraction.

	template<unsigned Nways, bool Use_LifeCost, bool Use_Hr>
	struct CellGraph : public CellGraph_<Nways, Use_LifeCost, Use_Hr> {
		using CellGraph_<Nways, Use_LifeCost, Use_Hr>::CellGraph_;
		
		using self_t = CellGraph<Nways, Use_LifeCost, Use_Hr>;
		
		struct AdjEdgeIterator {
			AdjEdgeIterator(unsigned s, self_t const& pSelf) :
				mState(s),
				mPos(0),
				mAdjCells(pSelf.getAdjacentCells(s)),
				mSelf(mSelf)
			{}
			
			bool finished() const {
				unsigned n = mAdjCells.n;
				return mPos == n;
			}
			
			unsigned state() const {
				slow_assert(!finished());
				return mAdjCells.adjCells[mPos];
			}
			
			void next() {
				slow_assert(!finished());
				mPos++;
			}
			
			typename self_t::Cost_t cost() const {
				slow_assert(!finished());
				return mSelf.getMoveCost(mState, mAdjCells.adjCells[mPos]);
			}
			
			unsigned mState;
			unsigned mPos;
			const typename self_t::AdjacentCells mAdjCells;
			self_t const& mSelf;
		};
		
		AdjEdgeIterator getAdjEdges(unsigned s) const {
			return AdjEdgeIterator(s, *this);
		}
		
	};
	
	
	//Domain class for the base level of a gridnav domain. Does not deal with abstraction.
	//	Basically its provided a reference to a CellGraph and goal state, and handles search-related functions.
	template<typename CellGraph_t>
	class GridNav_BaseDomain {
		public:
		using State = unsigned;
		using PackedState = unsigned;
		using Cost = typename CellGraph_t::Cost_t;
		
		static const bool Is_Perfect_Hash = true;
		
		using AdjEdgeIterator = typename CellGraph_t::AdjEdgeIterator;
		
		
		GridNav_BaseDomain(CellGraph_t const& pCellGraph, unsigned pGoalState) :
			mCellGraph(pCellGraph),
			mGoalState(pGoalState)
		{}
		

		State getGoalState() const {
			return mGoalState;
		}

		void packState(State const& pState, PackedState& pPacked) const {
			pPacked = pState;
		}
		
		void unpackState(State& pState, PackedState const& pPacked) const {
			pState = pPacked;
		}

		typename CellGraph_t::AdjEdgeIterator getAdjEdges(unsigned s) const {
			slow_assert(mCellGraph.isOpen(s));
			return mCellGraph.getAdjEdges(s);
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

		void prettyPrintState(State const& s, std::ostream& out) const {
			out << "[" << s << ", " << s%mCellGraph.getWidth() << ", " << s/mCellGraph.getWidth() << "]";
		}
		
		void drawCells(std::ostream& out) const {
			for(unsigned h=0; h<mCellGraph.getHeight(); h++) {
				for(unsigned w=0; w<mCellGraph.getWidth(); w++) {
					if(mCellGraph.isOpen(h*mCellGraph.getWidth() + w))
						out << ".";
					else
						out << "0";
					if((w+1) % 5 == 0)
						out << " ";
				}
				out << "\n";
				if((h+1) % 5 == 0)
					out << "\n";
			}
		}

		private:
		CellGraph_t const& mCellGraph;
		const unsigned mGoalState;
	};
	
	
	
	
	//The following is an implementation of the STAR abstraction method. It is drawn from a single CellGraph reference.
	
	//Neighbourhoods of states (open cells) are grouped together. Each group makes a state in a new abstract space, with in the new space
	//	begin the min-cost edge between any state in one abstract group to any state in the other group/
	
	//These groups are again grouped together to make groups in a further abstract space, and so on until the top abstract level
	//	has no edges between groups.
	
	//Intended for ranking open cells by descending out degree, then ascending index of cell.
	struct HubPrioComp {
		bool operator()(std::pair<unsigned, unsigned> const& a, std::pair<unsigned, unsigned> const& b) const {
			return a.first != b.first ? a.first > b.first : a.second < b.second;
		}
	};
	
	
	//Destination/cost pair, representing an edge on some abstract level.
	template<typename CellGraph_t>
	struct StarAbt_Edge {
		unsigned dst;
		typename CellGraph_t::Cost_t cst;		
	};
	
	//List of edges, for one state.
	template<typename CellGraph_t>
	using StarAbt_StateInfo = std::vector<StarAbt_Edge<CellGraph_t>>;
	
	//All StateInfo's at some level. Edges of state i are at stateEdges[i].
	//trns has an entry for each state in the previous level, and maps each surjectively to a state at this level.
	template<typename CellGraph_t>
	struct StarAbt_LevelInfo {
		std::vector<StarAbt_StateInfo<CellGraph_t>> stateEdges;
		std::vector<unsigned> trns;
	};
	
	
	template<typename CG>
	std::vector<std::pair<unsigned, unsigned>> prepHubRankList(StarAbt_LevelInfo<CG> const& lvlinfo) {
		std::vector<std::pair<unsigned, unsigned>> v(lvlinfo.stateEdges.size());
		for(unsigned i=0; i<lvlinfo.stateEdges.size(); i++)
			v[i] = {lvlinfo.stateEdges[i].size(), i};
		
		std::sort(v.begin(), v.end(), HubPrioComp());
		return v;
	}
	
	template<typename CG>
	unsigned doAssignAbtMapping(unsigned s, 
								unsigned a, 
								unsigned curDepth, 
								unsigned maxDepth, 
								StarAbt_LevelInfo<CG> const& curLvl,
								StarAbt_LevelInfo<CG>& abtLvl) {		
		if(abtLvl.trns[s] != (unsigned)-1 || curDepth > maxDepth)
			return 0;
		
		abtLvl.trns[s] = a;
		unsigned c = 1;
		
		for(auto const& e : curLvl.stateEdges[s])
			c += doAssignAbtMapping(e.dst, a, curDepth+1, maxDepth, curLvl, abtLvl);

		return c;
	}
	
	template<typename CG>
	unsigned relabelAbtStates(StarAbt_LevelInfo<CG>& abtLvl) {
		std::map<unsigned, unsigned> rl;
		unsigned c = 0;
		
		for(unsigned i=0; i<abtLvl.trns.size(); i++) {
			unsigned lbl = abtLvl.trns[i];
			
			if(rl.count(lbl) == 0)
				rl[lbl] = c++;
			abtLvl.trns[i] = rl[lbl];
		}
		
		return c;
	}
	
	template<typename CG>
	unsigned assignAbtMapping(unsigned maxDepth, StarAbt_LevelInfo<CG> const& curLvl, StarAbt_LevelInfo<CG>& abtLvl) {
		
		abtLvl.trns.resize(curLvl.stateEdges.size());
		std::fill(abtLvl.trns.begin(), abtLvl.trns.end(), (unsigned)-1);
		
		std::vector<std::pair<unsigned, unsigned>> hublist = prepHubRankList(curLvl);
		
		std::vector<unsigned> singletonStates;
		for(unsigned i=0; i<hublist.size(); i++) {
			if(doAssignAbtMapping(hublist[i].second, hublist[i].second, 0, maxDepth, curLvl, abtLvl) == 1)
				singletonStates.push_back(hublist[i].second);
		}
		
		for(auto ss : singletonStates) {
			for(auto const& e : curLvl.stateEdges[ss]) {
				unsigned dstabt = abtLvl.trns[e.dst];
				abtLvl.trns[ss] = dstabt;
				break;
			}
		}
		
		return relabelAbtStates(abtLvl);
	}
	
	template<typename CG>
	bool mapAbtEdges(unsigned nAbtStates, StarAbt_LevelInfo<CG> const& curLvl, StarAbt_LevelInfo<CG>& abtLvl) {
		std::map<unsigned, std::map<unsigned, typename CG::Cost_t>> abtEdges;
		
		for(unsigned i=0; i<curLvl.stateEdges.size(); i++) {
			unsigned src = abtLvl.trns[i];
			
			for(unsigned j=0; j<curLvl.stateEdges[i].size(); j++) {
				unsigned dst = abtLvl.trns[curLvl.stateEdges[i][j].dst];
				
				if(src == dst)
					continue;
				
				typename CG::Cost_t cst = curLvl.stateEdges[i][j].cst;

				if(abtEdges[src].count(dst) == 0)
					abtEdges[src][dst] = cst;
				
				else if(abtEdges[src][dst] > cst)
					abtEdges[src][dst] = cst;
			}
		}

		bool isTrivial = true;
		abtLvl.stateEdges.resize(nAbtStates);
		
		for(auto abtStateIt=abtEdges.cbegin(); abtStateIt!=abtEdges.cend(); ++abtStateIt) {
			unsigned src = abtStateIt->first;
			
			for(auto const& e : abtStateIt->second) {
				unsigned dst = e.first;
				fast_assert(src < abtLvl.stateEdges.size());
				abtLvl.stateEdges[src].push_back({dst, e.second});
				isTrivial = false;
			}
			//..... abtLvl.stateEdges is not sorted.
		}
		return isTrivial;
	}
	
	
	template<typename CG>
	bool fillAbtLevelInfo(unsigned maxDepth, StarAbt_LevelInfo<CG> const& curLvl, StarAbt_LevelInfo<CG>& abtLvl) {
		fast_assert(abtLvl.stateEdges.empty() && abtLvl.trns.empty());
		
		unsigned nAbtStates = assignAbtMapping(maxDepth, curLvl, abtLvl);
		bool isTrivial = mapAbtEdges(nAbtStates, curLvl, abtLvl);
		
		return isTrivial;
	}


	//Creates 0 or more abstract levels for the given cellgraph and abstraction radius.
	//levelsInfo[0] is an intermediate level.
	
	//Top abstract level is levelsInfo.size()-1.
	//Number of states at level i is levelsInfo[i].stateEdges.size().
	
	//levelsInfo[i].trns maps states of level i-1 to level i.
	
	//To translate base position ([0, cellgraph::size()-1] to its abstract representation at some level lvl>0, 
	//	s = bs
	//	for i=[0,lvl]:
	//		s = levelsInfo[i].trns[s]
	
	template<typename CG>
	struct StarAbt_Stack {
		std::vector<StarAbt_LevelInfo<CG>> levelsInfo;
		
		StarAbt_Stack(CG const& pCellGraph, unsigned pAbtRadius) :
			mHeight(pCellGraph.getHeight()),
			mWidth(pCellGraph.getWidth()),
			mParam_abtRadius(pAbtRadius)
		{
			levelsInfo.emplace_back();
			levelsInfo[0].trns.resize(pCellGraph.size());
			std::fill(levelsInfo[0].trns.begin(), levelsInfo[0].trns.end(), (unsigned)-1);
			
			unsigned c = 0;
			for(unsigned i=0; i<pCellGraph.size(); i++) {
				if(!pCellGraph.isOpen(i))
					continue;
				levelsInfo[0].trns[i] = c++;
			}
			
			bool isTrivial = true;
			levelsInfo[0].stateEdges.resize(c);
			for(unsigned i=0; i<pCellGraph.size(); i++) {
				if(!pCellGraph.isOpen(i))
					continue;
				unsigned src = levelsInfo[0].trns[i];
				
				for(auto edgeIt = pCellGraph.getAdjEdges(i); !edgeIt.finished(); edgeIt.next()) {
					unsigned dst = levelsInfo[0].trns[edgeIt.state()];
					
					StarAbt_Edge<CG> abtedge{dst, edgeIt.cost()};
					levelsInfo[0].stateEdges[src].push_back(abtedge);
					isTrivial = false;
				}
			}
			
			if(isTrivial)
				return;
				
			while(true) {
				levelsInfo.emplace_back();
				if(fillAbtLevelInfo(mParam_abtRadius, *(levelsInfo.end()-2), levelsInfo.back()))
					break;
			}
		}
		
		std::vector<StarAbt_LevelInfo<CG>> const& getLevelsInfo() const {
			return levelsInfo;
		}
		
		Json report() const {
			Json j;
			j["top_level"] = levelsInfo.size() - 1;
			for(unsigned i=0; i<levelsInfo.size(); i++) {
				j[std::to_string(i) + "_states"] = levelsInfo[i].stateEdges.size();
			}
			return j;
			
		}
		
		unsigned abstractBaseState(unsigned bs, unsigned lvl) const {
			if(lvl == 0)
				return bs;

			unsigned s = bs;
			
			for(unsigned i=0; i<=lvl; i++) {
				s = levelsInfo.at(i).trns.at(s);
			}
			return s;
		}
		
		unsigned abstractBaseStateMax(unsigned bs) const {
			return abstractBaseState(bs, levelsInfo.size()-1);
		}

		
		void drawAll(std::ostream& out) const {
			std::vector<unsigned> curState(levelsInfo[0].trns.size());
			for(unsigned i=0; i<mHeight*mWidth; i++) {
				if(levelsInfo[0].trns[i] != (unsigned)-1)
					curState[i] = i;
				else
					curState[i] = (unsigned)-1;
			}

			drawGraph(curState, out);
			
			for(unsigned lvl=0; lvl<levelsInfo.size(); lvl++) {
				auto const& li = levelsInfo[lvl];
				
				for(unsigned i=0; i<curState.size(); i++)
					if(curState[i] != (unsigned)-1)
						curState[i] = li.trns.at(curState[i]);
				
				out << "\n\n\n";
				drawGraph(curState, out);
				drawEdges(li, out);			
			}
		}
		
		void drawGraph(std::vector<unsigned> const& v, std::ostream& out) const {
			for(unsigned h=0; h<mHeight; h++) {
				for(unsigned w=0; w<mWidth; w++) {
					if(v.at(h*mWidth + w) == (unsigned)-1)
						out << ".";
					else
						out << v.at(h*mWidth + w) % 10;
					if((w+1) % 5 == 0)
						out << " ";
				}
				out << "\n";
				if((h+1) % 5 == 0)
					out << "\n";
			}
		}
		
		void drawEdges(StarAbt_LevelInfo<CG> const& li, std::ostream& out) const {
			for(unsigned i=0; i<li.stateEdges.size(); i++) {
				out << i << ": ";
				
				for(unsigned j=0; j<li.stateEdges[i].size(); j++) {
					out << "(" << li.stateEdges[i][j].dst << "," << li.stateEdges[i][j].cst << ") ";
				}
				
				out << "\n";
			}
		}

		const unsigned mHeight, mWidth, mParam_abtRadius;
	};



	
	
	
	template<typename CG>
	struct GridNav_StarAbtDomain {
		public:
		using State = unsigned;
		using PackedState = unsigned;
		using Cost = typename CG::Cost_t;
		
		static const bool Is_Perfect_Hash = true;


		struct AdjEdgeIterator {
			AdjEdgeIterator(StarAbt_StateInfo<CG> const& pEdges) :
				mPos(0),
				mEdges(pEdges)
			{}
			
			bool finished() const {
				return mPos == mEdges.size();
			}
			
			unsigned state() const {
				slow_assert(!finished());
				return mEdges[mPos].dst;
			}
			
			void next() {
				slow_assert(!finished());
				mPos++;
			}
			
			typename CG::Cost_t cost() const {
				slow_assert(!finished());
				return mEdges[mPos].cst;
			}
			
			unsigned mState;
			unsigned mPos;
			StarAbt_StateInfo<CG> const& mEdges;
		};
		
		
		
		GridNav_StarAbtDomain(StarAbt_LevelInfo<CG> const& pLevelInfo, unsigned pGoalState) :
			mLevelInfo(pLevelInfo),
			mGoalState(pGoalState)
		{}
		
		
		unsigned abstractParentState(unsigned bs) const {
			slow_assert(bs < mLevelInfo.trns.size());
			return mLevelInfo.trns[bs];
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

		AdjEdgeIterator getAdjEdges(unsigned s) const {
			slow_assert(s < mLevelInfo.stateEdges.size());
			return AdjEdgeIterator(mLevelInfo.stateEdges[s]);
		}

		size_t hash(PackedState pPacked) const {
			return pPacked;
		}
		
		bool checkGoal(unsigned pState) const {
			return pState == mGoalState;
		}

		bool compare(unsigned a, unsigned b) const {
			return a == b;
		}

		void prettyPrintState(State const& s, std::ostream& out) const {
			out << s;
		}

		private:
		StarAbt_LevelInfo<CG> const& mLevelInfo;
		const unsigned mGoalState;
		
	};
	
	
	
	
	
	template<typename CG, unsigned Top_Abt>
	class GridNav_StarAbtStack {
		public:
		static const unsigned Top_Abstract_Level = Top_Abt;
		using Stack_t = GridNav_StarAbtStack<CG, Top_Abt>;
		
		template<unsigned L, typename = void>
		struct Domain : public GridNav_StarAbtDomain<CG> {
			
			Domain(Stack_t const& pStack) :
				GridNav_StarAbtDomain<CG>(pStack.mStackInfo.levelsInfo.at(L), pStack.mStackInfo.abstractBaseState(pStack.mGoalState, L)),
				mLevelInfo(pStack.mStackInfo.levelsInfo.at(L))
			{}
			
			unsigned abstractParentState(unsigned bs) const {
				slow_assert(mLevelInfo.trns.size() > bs);
				return mLevelInfo.trns[bs];
			}
			
			StarAbt_LevelInfo<CG> const& mLevelInfo;
		};
		
		template<typename Ign>
		struct Domain<1,Ign> : public GridNav_StarAbtDomain<CG> {
			
			Domain(Stack_t& pStack) :
				GridNav_StarAbtDomain<CG>(pStack.mStackInfo.levelsInfo.at(1), pStack.mStackInfo.abstractBaseState(pStack.mGoalState, 1)),
				mBaseLevelInfo(pStack.mStackInfo.levelsInfo.at(0)),
				mCurLevelInfo(pStack.mStackInfo.levelsInfo.at(1))
			{}
			
			unsigned abstractParentState(unsigned bs) const {
				return mCurLevelInfo.trns[mBaseLevelInfo.trns[bs]];
			}
			
			StarAbt_LevelInfo<CG> const& mBaseLevelInfo, mCurLevelInfo;
		};	
		
		
		
		template<typename Ign>
		struct Domain<0, Ign> : public GridNav_BaseDomain<CG> {
			Domain(Stack_t const& pStack) :
				GridNav_BaseDomain<CG>(pStack.mCellGraph, pStack.mGoalState)
			{}
		};
		
		unsigned getInitState() const {
			fast_assert(mInitState != (unsigned)-1);
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

				if(mStackInfo.abstractBaseStateMax(s0) != mStackInfo.abstractBaseStateMax(sg)) {
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
			mInitState(-1),
			mGoalState(-1),
			mHeight(jConfig.at("height")),
			mWidth(jConfig.at("width")),
			mCellGraph(mHeight, mWidth, jConfig.at("map")),
			mStackInfo(mCellGraph, jConfig.at("abt_radius"))
		{
			fast_assert(Top_Abstract_Level == mStackInfo.levelsInfo.size()-1);
		}
		
		

		unsigned mInitState, mGoalState;
		const unsigned mHeight, mWidth;
		CG mCellGraph;
		StarAbt_Stack<CG> mStackInfo;
		
	};
}}}