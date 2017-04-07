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
	};
	
	
	template<typename CellGraph_t>
	class GridNav_BaseDomain {
		public:
		using State = unsigned;
		using PackedState = unsigned;
		using Cost = typename CellGraph_t::Cost_t;
		
		static const bool Is_Perfect_Hash = true;


		struct AdjEdgeIterator {
			AdjEdgeIterator(unsigned s, CellGraph_t const& pCellGraph) :
				mState(s),
				mPos(0),
				mAdjCells(pCellGraph.getAdjacentCells(s)),
				mCellGraph(pCellGraph)
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
			
			typename CellGraph_t::Cost_t cost() const {
				slow_assert(!finished());
				slow_assert(mPos < mAdjCells.n);
				return mCellGraph.getMoveCost(mState, mAdjCells.adjCells[mPos]);
			}
			
			unsigned mState;
			unsigned mPos;
			const typename CellGraph_t::AdjacentCells mAdjCells;
			CellGraph_t mCellGraph;
		};
		
		
		
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

		AdjEdgeIterator getAdjEdges(unsigned s) const {
			slow_assert(mCellGraph.isOpen(s));
			return AdjEdgeIterator(s, mCellGraph);
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
	
	
	
	template<typename CellGraph_t>
	struct StarAbtStackGenerator {
		using Cost = typename CellGraph_t::Cost_t;
		
		StarAbtStackGenerator(unsigned pHeight, unsigned pWidth, unsigned pAbtRadius, const std::string& pMapStr) :
			mHeight(pHeight),
			mWidth(pWidth),
			mMapStr(pMapStr),
			mCellGraph(pHeight, pWidth, pMapStr),
			mParam_abtRadius(pAbtRadius)
		{
			makeLevels();
			fast_assert(mTrns.size() == mEdges.size());
		}	

		void makeLevels() {
			{
				GridNav_BaseDomain<CellGraph_t> dom(mCellGraph, (unsigned)-1);
				
				mTrns.push_back(std::vector<unsigned>(mCellGraph.size()));
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
		const std::string mMapStr;
		const CellGraph_t mCellGraph;
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
			mParam_abtRadius(pAbtRadius)
		{
			StarAbtStackGenerator<CellGraph_t> abtgen(mHeight, mWidth, mParam_abtRadius, pMapStr);
			mTrns = abtgen.mTrns;
			mEdges = abtgen.mEdges;
		}
		
		StarAbtStackInfo(std::string const& pDumpFileStr) {
			Json j;
			std::ifstream ifs(pDumpFileStr);
			fast_assert(ifs);
			j << ifs;
			
			mHeight = j.at("base_height");
			mWidth = j.at("base_width");
			mParam_abtRadius = j.at("abt_radius");
			
			for(unsigned i=0; i<j.at("n_levels"); i++) {
				mTrns.push_back(j.at("trns").at(i).get_ref<std::vector<unsigned>&>());
				
				std::vector<std::vector<unsigned>> const& edgesDst = j.at("edges_dst").at(i).get_ref<std::vector<std::vector<unsigned>>&>();
				std::vector<std::vector<unsigned>> const& edgesCst = j.at("edges_cst").at(i).get_ref<std::vector<std::vector<std::string>>&>();
		
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
