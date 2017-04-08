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
	class CellGraph_8 : public CellMapBlocked<> {
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
			Cost_t cst = Use_LifeCost ? src/mWidth : 1;

			if(src%mWidth != dst%mWidth && src/mWidth != dst/mWidth)
				return {1,0};
			
			return {0,1};
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
	
	
	
	

	template<unsigned Nways, bool Use_LifeCost, bool Use_Hr>
	class CellGraph : public CellGraph_<Nways, Use_LifeCost, Use_Hr> {
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
			const typename mSelf::AdjacentCells mAdjCells;
			self_t const& mSelf;
		};
		
		AdjEdgeIterator getAdjEdges(unsigned s) const {
			return AdjEdgeIterator(s, *this);
		}
		
	};
	
	
	template<typename CellGraph_t>
	class GridNav_BaseDomain {
		public:
		using State = unsigned;
		using PackedState = unsigned;
		using Cost = typename CellGraph_t::Cost_t;
		
		static const bool Is_Perfect_Hash = true;
		
		
		
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
	
	
	
	
	struct HubPrioComp {
		bool operator()(std::pair<unsigned, unsigned> const& a, std::pair<unsigned, unsigned> const& b) const {
			return a.first != b.first ? a.first > b.first : a.second < b.second;
		}
	};
	
	
	
	
	
	
	template<typename CellGraph_t>
	struct StarAbt_Edge {
		unsigned dst;
		typename CellGraph_t::Cost cst;		
	};
	
	template<typename CellGraph_t>
	using StarAbt_StateInfo = std::vector<StarAbt_Edge<CellGraph_t>>;
	
	template<typename CellGraph_t>
	struct StarAbt_LevelInfo {
		std::vector<StarAbt_StateInfo<CellGraph_t>> stateEdges;
		std::vector<unsigned> trns;
	}
	
	
	template<typename CG>
	std::vector<std::pair<unsigned, unsigned>> prepHubRankList(StarAbt_LevelInfo<CG> const& lvlinfo) {
		std::vector<std::pair<unsigned, unsigned>> v(lvlinfo.trns.size());
		for(unsigned i=0; i<lvlinfo.trns.size(); i++)
			v[i] = {lvlinfo.statEdges[i].size(); i};
		
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
			c += doAssignAbtMapping(e.first, a, curDepth+1, maxDepth, curLvl, abtLvl);
		
		return c;
	}
	
	template<typename CG>
	void relabelAbtStates(StarAbt_LevelInfo<CG>& abtLvl) {
		std::map<unsigned, unsigned> rl;
		unsigned c = 0;
		
		for(unsigned i=0; i<abtLvl.trns.size(); i++) {
			if(rl.count(i) == 0)
				rl[i] = c++;
			abtLvl[i] = rl[abtLvl[i]];			
		}
	}
	
	template<typename CG>
	void assignAbtMapping(unsigned maxDepth, StarAbt_LevelInfo<CG> const& curLvl, StarAbt_LevelInfo<CG>& abtLvl) {
		
		abtLvl.trns.resize(curLvl.stateEdges.size());
		std::fill(abtLvl.trns.begin(), abtLvl.trns.end(), (unsigned)-1);
		
		std::vector<std::pair<unsigned, unsigned>> hublist = prepHubRankList(curLvl);
		
		std::vector<unsigned> singletonStates;
		for(unsigned i=0; i<hublist.size(); i++) {
			if(doAssignAbtMapping(hublist[i].second, hublist[i].second, 0, maxDepth, curLvl, abtLvl) == 1)
				singletonStates.push_back(i);
		}
		
		for(auto ss : singletonStates) {
			for(auto const& e : curLvl.stateEdges[ss]) {
				dstabt = abtLvl.trns[e.first];
				abtLvl[ss] = dstabt;
				break;
			}
		}
		
		relabelAbtStates(abtLvl);
	}
	
	template<typename CG>
	bool mapAbtEdges(StarAbt_LevelInfo<CG> const& curLvl, StarAbt_LevelInfo<CG>& abtLvl) {
		std::map<std::map<unsigned, typename CG::Cost_t>> abtEdges;
		
		for(unsigned i=0; i<curLvl.stateEdges.size(); i++) {
			unsigned src = abtLvl.trns[i];
			
			for(unsigned j=0; j<curLvl.stateEdges[i]; j++) {
				unsigned dst = abtLvl.trns[curLvl.statesEdges[j].first];
				
				if(src == dst)
					continue;
				
				typename CG::Cost_t cst = curLvl.statesEdges[j].second;

				if(abtEdges[src][dst] > cst)
					abtEdges[src][dst] = cst;
			}
		}

		bool isTrivial = true;
		unsigned nAbtStates = abtEdges.size();
		abtLvl.stateEdges.resize(nAbtStates);
		
		for(auto abtStateIt=abtEdges.cbegin(), abtStateIt!=abtEdges.cend(), ++it) {
			unsigned src = abtStateIt.first;
			
			for(auto const& e : abtStateIt.second) {
				unsigned dst = e.first;
				abtLvl.stateEdges[src].push_back({dst, e.second});
				isTrivial = false;
			}
		}
		return isTrivial;
	}
	
	
	template<typename CG>
	bool fillAbtLevelInfo(unsigned maxDepth, StarAbt_LevelInfo<CG> const& curLvl, StarAbt_LevelInfo<CG>& abtLvl) {
		fast_assert(abtLvl.stateEdges.empty() && abtLvl.trns.empty());
		
		assignAbtMapping(maxDepth, curLvl, abtLvl);
		return mapAbtEdges(curLvl, abtLvl);		
	}


	template<typename CG>
	struct StarAbt_Stack {
		std::vector<StarAbt_Level<CG>> levelsInfo;		
		
		StarAbt_Stack(CG const& pCellGraph) {
			levelsInfo.emplace_back();
			levelsInfo[0].trns.resize(pCellGraph.size());
			std::fill(levelsInfo[0].trns.begin(), levelsInfo.trns.end(), (unsigned)-1);
			
			unsigned c = 0;
			for(unsigned i=0; i<pCellGraph.size(); i++) {
				if(!pCellGraph.isOpen(i))
					continue;
				levelsInfo[0].trns[i] = c++;
			}
			
			bool isTrivial = true;
			levelsInfo[0].stateEdges.resize(c);
			for(unsigned i; i<pCellGraph.size(); i++) {
				if(!pCellGraph.isOpen(i))
					continue;
				unsigned src = levelsInfo[0].trns[i];
				
				for(auto e : pCellGraph.getAdjEdges(i)) {
					unsigned dst = levelsInfo[0].trns[e.first];
					
					StarAbt_Edge<CG> abtedge{dst, e.second};
					levelsInfo[0].statsEdges[src].push_back(abtedge);
					isTrivial = false;
				}
			}
			
			if(isTrivial)
				return;
				
			while(true) {
				levelsInfo.emplace_back();
				if(fillAbtLevelInfo(2, levelsInfo.end()-2, levelsInfo.back()))
					break;
			}
		}
		
		std::vector<StarAbt_Level<CG>> const& getLevelsInfo() {
			return levelsInfo;
		}
	};
	

	
	
	
	template<typename CellGraph_t>
	struct StarAbtStackGenerator {
		using Cost = typename CellGraph_t::Cost_t;
		
		StarAbtStackGenerator(unsigned pAbtRadius, CellGraph_t const& pCellGraph) :
			mHeight(pCellGraph.getHeight()),
			mWidth(pCellGraph.getWidth()),
			mCellGraph(pCellGraph),
			mParam_abtRadius(pAbtRadius)
		{
			makeLevels();
			fast_assert(mTrns.size() == mEdges.size());
		}	

		void makeLevels() {
			{
				GridNav_BaseDomain<CellGraph_t> dom(mCellGraph, (unsigned)-1);
				
				mTrns.push_back(std::vector<unsigned>(mCellGraph.size()));
				std::fill(mTrns[0].begin(), mTrns[0].end(), (unsigned)-1);
				
				unsigned c = 0;
				
				for(unsigned i=0; i<mCellGraph.size(); i++)
					if(mCellGraph.isOpen(i))
						mTrns[0][i] = c++;

				bool isTrivial = true;
				mEdges.push_back(std::vector<std::vector<std::pair<unsigned, Cost>>>(c));
				
				for(unsigned i=0; i<mCellGraph.size(); i++) {
					if(!mCellGraph.isOpen(i))
						continue;
					
					for(auto edgeIt = dom.getAdjEdges(i); !edgeIt.finished(); edgeIt.next()) {
						unsigned dst = mTrns[0].at(edgeIt.state());
						Cost cst = edgeIt.cost();

						mEdges.back().at(mTrns[0][i]).push_back({dst,cst});
						isTrivial = false;
					}
				}
				if(isTrivial)
					return;
			}
			
			//~ for(unsigned bs=0; bs<mCellGraph.size(); bs++) {
				//~ if(!mCellGraph.isOpen(bs))
					//~ continue;
				//~ std::cout << bs << " -> " << mTrns[0][bs] << "\n";
				//~ std::cout << "\t";
				
				//~ for(auto const& e : mEdges[0][mTrns[0][bs]]) {
					//~ std::cout << e.first << " ";
				//~ }
				//~ std::cout << "\n";
			//~ }
			//~ return;
			
			//mTrns[0]: size=baseHeight*baseWidth. Bijection of base state to its 'normalised' 
			//	version (i.e. [0,basesize-1] to [0, no. of open cells-1]).
			//mEdges[0]: size=no. of open cells. Element i is a vector of edges for normalised state i, with form {dst norm state,cost}.
			
			while(true) {
				{
					std::vector<std::vector<std::pair<unsigned,Cost>>> const& curEdges = mEdges.back();
					
					mTrns.push_back(std::vector<unsigned>(curEdges.size()));
					std::vector<unsigned>& abtTrns = mTrns.back();
					std::fill(abtTrns.begin(), abtTrns.end(), (unsigned)-1);				
					
					std::vector<std::pair<unsigned, unsigned>> hubRankVec;
					for(unsigned i=0; i<curEdges.size(); i++)
						hubRankVec.push_back({curEdges[i].size(), i});
					
					std::sort(hubRankVec.begin(), hubRankVec.end(), HubPrioComp());
					
					std::vector<unsigned> singletonStates;
					
					for(unsigned i=0; i<hubRankVec.size(); i++)
						if(doAssignAbtMapping(hubRankVec[i].second, i, 0) == 1)
							singletonStates.push_back(hubRankVec[i].second);
					
					slow_assert(std::find(abtTrns.begin(), abtTrns.end(), (unsigned)-1) == abtTrns.end());
					
					slow_assert(mathutil::uniqueElements(singletonStates));
					
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
					
					mEdges.push_back(std::vector<std::vector<std::pair<unsigned,Cost>>>(nAbtStates));
				}
				
				
				std::vector<unsigned> const& abtTrns = mTrns.back();
				std::vector<std::vector<std::pair<unsigned,Cost>>>& abtEdges = mEdges.back();
				std::vector<std::vector<std::pair<unsigned,Cost>>> const& curEdges = mEdges.at(mEdges.size()-2);
				
				bool isTrivial = true;
				
				for(unsigned i=0; i<curEdges.size(); i++) {
					unsigned src = abtTrns.at(i);

					for(auto const& e : curEdges.at(i)) {
						unsigned dst = abtTrns.at(e.first);
						Cost cst = e.second;

						if(src == dst)
							continue;
						
						isTrivial = false;
						
						bool newEntry = true;
						for(auto& e : abtEdges.at(src))
							if(e.first == dst && e.second > cst) {
								e.second = cst;
								newEntry = false;
							}
						
						if(newEntry)
							abtEdges.at(src).push_back({dst, cst});
					}
					std::sort(abtEdges.at(src).begin(), abtEdges.at(src).end());
				}

				if(isTrivial)
					break;
			}
		}
		
		
		
		unsigned doAssignAbtMapping(unsigned s, unsigned a, unsigned curDepth) {
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
		CellGraph_t const& mCellGraph;
		const unsigned mParam_abtRadius;
	};
	
	
	template<typename CellGraph_t>
	struct StarAbtStackInfo {
		using Cost = typename CellGraph_t::Cost_t;
		
		StarAbtStackInfo() {
			mHeight = mWidth = mParam_abtRadius = (unsigned)-1;
		}
		
		StarAbtStackInfo(unsigned pHeight, unsigned pWidth, unsigned pAbtRadius, const std::string& pMapStr) :
			mHeight(pHeight),
			mWidth(pWidth),
			mParam_abtRadius(pAbtRadius),
			mCellGraph(mHeight, mWidth, pMapStr)
		{
			fast_assert(pHeight == mCellGraph.getHeight() && pWidth == mCellGraph.getWidth());
			
			StarAbtStackGenerator<CellGraph_t> abtgen(mParam_abtRadius, mCellGraph);
			mTrns = abtgen.mTrns;
			mEdges = abtgen.mEdges;
		}
		
		//~ StarAbtStackInfo(std::string const& pDumpFileStr) {
			//~ Json j;
			//~ std::ifstream ifs(pDumpFileStr);
			//~ fast_assert(ifs);
			//~ j << ifs;
			
			//~ mHeight = j.at("base_height");
			//~ mWidth = j.at("base_width");
			//~ mParam_abtRadius = j.at("abt_radius");
			//~ mCellGraph = mCellGraph(mHeight, mWidth, j.at("map"));
			
			//~ for(unsigned i=0; i<j.at("n_levels").get<unsigned>(); i++) {
				//~ mTrns.push_back(j.at("trns").at(i).get<std::vector<unsigned>>());
				
				//~ Json const& levelEdges_dst = j.at("edges_dst").at(i);
				//~ Json const& levelEdges_cst = j.at("edges_cst").at(i);
				
				//~ mEdges.emplace_back();
				
				//~ for(unsigned j=0; j<levelEdges_dst.size(); j++) {
					//~ Json const& stateEdges_dst = levelEdges_dst.at(j);
					//~ Json const& stateEdges_cst = levelEdges_cst.at(j);
					
					//~ mEdges.back().emplace_back();
					//~ for(unsigned k=0; k<stateEdges_dst.size(); k++) {
						//~ mEdges.back().back().push_back({stateEdges_dst[k], CellGraph_t::costFromString(stateEdges_cst[k])});
					//~ }
				//~ }
			//~ }
		//~ }
		
		unsigned getNlevels() const {
			return mEdges.size();
		}
		
		std::vector<unsigned> const& getTrns(unsigned i) const {
			return mTrns.at(i);
		}
		
		std::vector<std::vector<std::vector<std::pair<unsigned,Cost>>>> const& getEdges(unsigned i) const {
			return mEdges.at(i);
		}
		
		Json prepJsonObj() const {
			Json jOut;
			
			jOut["n_levels"] = getNlevels();
			jOut["base_height"] = mHeight;
			jOut["base_width"] = mWidth;
			jOut["abt_radius"] = mParam_abtRadius;
			
			for(unsigned i=0; i<getNlevels(); i++) {
				jOut["trns"].push_back(mTrns[i]);
				
				jOut["edges_dst"].push_back(Json::array_t());
				jOut["edges_cst"].push_back(Json::array_t());
				
				for(unsigned s=0; s<mEdges.at(i).size(); s++) {
					jOut.at("edges_dst").back().push_back(Json::array_t());
					jOut.at("edges_cst").back().push_back(Json::array_t());
					
					for(auto const& e : mEdges.at(i).at(s)) {
						jOut.at("edges_dst").back().back().push_back(e.first);
						jOut.at("edges_cst").back().back().push_back(CellGraph_t::costToString(e.second));
					}
				}
			}
			return jOut;
		}
		
		bool operator==(StarAbtStackInfo<CellGraph_t> const& o) const {
			Json a = prepJsonObj(), b = o.prepJsonObj();
			return mCellGraph == o.mCellGraph && std::hash<Json>{}(a) == std::hash<Json>{}(b);
		}
		
		size_t hash() const {
			return std::hash<Json>{}(prepJsonObj());
		}
			
		void dumpToFile(std::string const& pOutStr) const {
			Json jOut = prepJsonObj();
			std::ofstream ofs(pOutStr);
			fast_assert(ofs);
			
			ofs << jOut.dump(2);
		}
		
		void drawAll(std::ostream& out) const {
			std::vector<unsigned> curState(mHeight*mWidth);
			for(unsigned i=0; i<curState.size(); i++)
				curState[i] = mTrns.at(0).at(i);
			
			unsigned lvl = 0;
			while(true) {
				drawGraph(curState, out);
				drawEdges(lvl, out);
				std::cout << "\n";
				
				lvl++;
				
				if(lvl == getNlevels())
					break;
				
				for(unsigned i=0; i<curState.size(); i++)
					if(curState[i] != (unsigned)-1)
						curState[i] = mTrns.at(lvl).at(curState[i]);
				
				
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
		
		void drawEdges(unsigned pLevel, std::ostream& out) const {
			for(unsigned i=0; i<mEdges.at(pLevel).size(); i++) {
				std::cout << i << ": ";
				
				for(unsigned j=0; j<mEdges.at(pLevel).at(i).size(); j++) {
					std::cout << "(" << mEdges.at(pLevel).at(i).at(j).first << "," << mEdges.at(pLevel).at(i).at(j).second << ") ";
				}
				
				std::cout << "\n";
			}
		}
		

		std::vector<std::vector<unsigned>> mTrns;
		std::vector<std::vector<std::vector<std::pair<unsigned,Cost>>>> mEdges;
		unsigned mHeight, mWidth;
		unsigned mParam_abtRadius;
		const CellGraph_t mCellGraph;
	};




	template<typename CellGraph_t>
	struct GridNavTestStack {
		
		static const unsigned Top_Abstract_Level = 0;
		
		template<unsigned L>
		struct Domain : GridNav_BaseDomain<CellGraph_t> {
			Domain(GridNavTestStack<CellGraph_t> const& pStack) :
				GridNav_BaseDomain<CellGraph_t>(pStack.mCellGraph, pStack.mGoalState)
			{}
		};
		
		unsigned getInitState() const {
			return mInitState;
		}
		
		GridNavTestStack(Json const& jConfig) :
			mCellGraph(jConfig.at("height"), jConfig.at("width"), jConfig.at("map"))
		{
			mInitState = 0;
			mGoalState = 0;
			while(!mCellGraph.isOpen(mInitState)) mInitState++; //if mInitState is greater than map size, isOpen will throw.
			while(!mCellGraph.isOpen(mGoalState)) mGoalState++;
		}
		
		CellGraph_t mCellGraph;
		unsigned mInitState, mGoalState;
	};
	
	
	
	
	template<typename CellGraph_t>
	struct GridNav_StarAbtDomain {
		public:
		using State = unsigned;
		using PackedState = unsigned;
		using Cost = typename CellGraph_t::Cost_t;
		
		static const bool Is_Perfect_Hash = true;


		struct AdjEdgeIterator {
			AdjEdgeIterator(std::vector<std::pair<unsigned,Cost>> const& pEdges) :
				mPos(0),
				mEdges(pEdges)
			{}
			
			bool finished() const {
				return mPos == mEdges.size();
			}
			
			unsigned state() const {
				slow_assert(!finished());
				return mEdges[mPos].first;
			}
			
			void next() {
				slow_assert(!finished());
				mPos++;
			}
			
			typename CellGraph_t::Cost_t cost() const {
				slow_assert(!finished());
				return mEdges[mPos].second;
			}
			
			unsigned mState;
			unsigned mPos;
			std::vector<std::pair<unsigned,Cost>> const& mEdges;
		};
		
		
		
		GridNav_StarAbtDomain(std::vector<std::pair<unsigned,Cost>> const& pLevelEdges, unsigned pGoalState) :
			mLevelEdges(pLevelEdges),
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

		AdjEdgeIterator getAdjEdges(unsigned s) const {
			return AdjEdgeIterator(s, mLevelEdges[s]);
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
		std::vector<std::pair<unsigned,Cost>> const& mLevelEdges;
		const unsigned mGoalState;
		
	};
	
	
	
	
	
	template<typename CellGraph_t, unsigned Top_Abt>
	class GridNav_StarAbtStack {
		public:
		using BaseDomain_t = GridNav_BaseDomain<CellGraph_t>;
		using Stack_t = GridNav_StarAbtStack<CellGraph_t, Top_Abt>;		
		static const unsigned Top_Abstract_Level = Top_Abt;
		static const unsigned Null_Idx = (unsigned)-1;
		

		template<unsigned L, typename = void>
		struct Domain : public GridNav_StarAbtDomain<CellGraph_t> {
			using Cost = typename CellGraph_t::Cost_t;
			
			Domain(Stack_t& pStack) :
				GridNav_StarAbtDomain<CellGraph_t>(pStack.mStackInfo.mEdges.at(L), pStack.abstractBaseState(pStack.mGoalState, L)),
				mLvlTrns(pStack.mStackInfo.mTrns.at(L))
			{}
			
			unsigned abstractParentState(unsigned bs) const {
				return mLvlTrns[bs];
			}
			
			std::vector<unsigned> const& mLvlTrns;
		};
		
		template<typename Ign>
		struct Domain<1,Ign> : public GridNav_StarAbtDomain<CellGraph_t> {
			using Cost = typename CellGraph_t::Cost_t;
			
			Domain(Stack_t& pStack) :
				GridNav_StarAbtDomain<CellGraph_t>(pStack.mStackInfo.mEdges.at(1), pStack.abstractBaseState(pStack.mGoalState, 1)),
				mTrns(pStack.mStackInfo.mTrns)
			{}
			
			unsigned abstractParentState(unsigned bs) const {
				return mTrns[1][mTrns[0][bs]];
			}
			
			std::vector<std::vector<unsigned>> const& mTrns;
		};	
		
		
		
		template<typename Ign>
		struct Domain<0, Ign> : public BaseDomain_t {
			Domain(Stack_t const& pStack) :
				BaseDomain_t(pStack.mStackInfo.mCellGraph, pStack.mGoalState)
			{}
		};
		
		unsigned abstractBaseState(unsigned bs, unsigned lvl) const {
			slow_assert(mStackInfo.mCellGraph.isOpen(bs));
			unsigned s = mStackInfo.mTrns.at(0).at(bs);
			
			for(unsigned i=1; i<=lvl; i++)
				s = mStackInfo.mTrns.at(i).at(s);
			
			return s;
		}
		
		bool baseStatesConnected(unsigned a, unsigned b) const {
			return abstractBaseState(a, Top_Abstract_Level) == abstractBaseState(b, Top_Abstract_Level);
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
				
				if(!mStackInfo.mCellGraph.isOpen(s0) || !mStackInfo.mCellGraph.isOpen(sg)) {
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
			mStackInfo(jConfig.at("height"), jConfig.at("width"), jConfig.at("abt_radius"), jConfig.at("map")),
			mInitState(-1),
			mGoalState(-1),
			mHeight(jConfig.at("height")),
			mWidth(jConfig.at("width"))
		{
		
		}
		
		
		StarAbtStackInfo<CellGraph_t> mStackInfo;
		unsigned mInitState, mGoalState;
		const unsigned mHeight, mWidth;
		
	};
}}}
