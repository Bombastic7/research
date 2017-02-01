#pragma once

#include <array>
#include <iostream>
#include <random>
#include <vector>
#include <cmath>
#include <queue>
#include <map>

#include "util/debug.hpp"
#include "util/math.hpp"

#include <cstdio>

namespace mjon661 { namespace gridnav { namespace blocked {


	enum struct Cell_t {
		Open, Blocked, Null
	};
	
	enum struct CardDir_t {
		N, S, W, E, NW, NE, SW, SE
	};
	
	const unsigned Null_Idx = (unsigned)-1;
	
	
	template<typename = void>
	struct FourWayFuncs {
		using Cost_t = int;
		
		struct AdjacentCells : public std::array<unsigned, 4> {
			AdjacentCells() {
				this->fill(Null_Idx);
			}
		}; 
		
		static const unsigned Max_Adj = 4;

		static void getAllEdges(unsigned pHeight, unsigned pWidth, unsigned i, AdjacentCells& pEdges) {
			pEdges.fill(Null_Idx);
			
			if(i >= pWidth) 			pEdges[0] = i-pWidth;
			if(i < (pHeight-1)*pWidth) 	pEdges[1] = i+pWidth;
			if(i % pWidth != 0) 		pEdges[2] = i-1;
			if((i+1) % pWidth != 0) 	pEdges[3] = i+1;
		}
		
		static const char* getOpName(unsigned i) {
			const char* opnames[4] = {"N", "S", "W", "E"};
			return opnames[i];
		}

		static Cost_t getMoveCost(unsigned i) {
			return 1;
		}
	};
	
	template<typename = void>
	struct EightWayFuncs {
		using Cost_t = float;
		
		struct AdjacentCells : public std::array<unsigned, 8> {
			AdjacentCells() {
				this->fill(Null_Idx);
			}
		}; 
		
		static constexpr Cost_t Diag_Mv_Cost = 1.41421356237309504880168872420969807857;
		static const unsigned Max_Adj = 8;
		
		static void getAllEdges(unsigned pHeight, unsigned pWidth, unsigned i, AdjacentCells& pEdges) {
			pEdges.fill(Null_Idx);

			if(i >= pWidth) 			pEdges[0] = i-pWidth;
			if(i < (pHeight-1)*pWidth) 	pEdges[1] = i+pWidth;
			if(i % pWidth != 0) 		pEdges[2] = i-1;
			if((i+1) % pWidth != 0) 	pEdges[3] = i+1;

			if(pEdges[0] != Null_Idx && pEdges[2] != Null_Idx) pEdges[4] = i-pWidth - 1;
			if(pEdges[0] != Null_Idx && pEdges[3] != Null_Idx) pEdges[5] = i-pWidth + 1;
			if(pEdges[1] != Null_Idx && pEdges[2] != Null_Idx) pEdges[6] = i+pWidth - 1;
			if(pEdges[1] != Null_Idx && pEdges[3] != Null_Idx) pEdges[7] = i+pWidth + 1;
		}
		
		static const char* getOpName(unsigned i) {
			const char* opnames[8] = {"N", "S", "W", "E", "NW", "NE", "SW", "SE"};
			return opnames[i];
		}
		
		static Cost_t getMoveCost(unsigned i) {
			return i >= 4 ? Diag_Mv_Cost : 1;
		}
	};
	
	


	template<typename BaseFuncs, bool Use_LC>
	struct CellMap {
		
		using Cost_t = typename BaseFuncs::Cost_t;
		using AdjacentCells = typename BaseFuncs::AdjacentCells;

		static const unsigned Max_Adj = BaseFuncs::Max_Adj;
		
		CellMap(unsigned pHeight, unsigned pWidth, std::istream& in) :
			mHeight(pHeight),
			mWidth(pWidth),
			mSize(pHeight * pWidth),
			mAdjList(mSize),
			mCellMap()
		{
			gen_assert(mSize > 0);
			
			std::vector<Cell_t> cellMap;
			cellMap.reserve(mSize);
			
			for(unsigned i=0; i<mSize; i++) {
				int v;
				Cell_t c;
				in >> v;
				c = (Cell_t)v;
				
				gen_assert(c == Cell_t::Open || c == Cell_t::Blocked);
				cellMap.push_back(c);
			}
			
			for(unsigned i=0; i<mSize; i++) {
				BaseFuncs::getAllEdges(mHeight, mWidth, i, mAdjList[i]);
			}
			
			for(unsigned i=0; i<mSize; i++) {
				for(unsigned j=0; j<Max_Adj; j++) {
					if(mAdjList[i][j] == Null_Idx)
						continue;
						
					if(cellMap[mAdjList[i][j]] == Cell_t::Blocked)
						mAdjList[i][j] = Null_Idx;
				}
			}
			
			mCellMap = cellMap;
		}
		
		AdjacentCells const& getAdjCells(unsigned idx) const {
			slow_assert(idx < mSize);
			return mAdjList[idx];
		}
		
		std::vector<AdjacentCells> const& getAdjCellsList() const {
			return mAdjList;
		}
		
		std::vector<Cell_t> const& getCells() const {
			return mCellMap;
		}
		
		Cost_t getOpCost(unsigned idx, unsigned op) const {
			slow_assert(idx < mSize);
			slow_assert(op < Max_Adj);
			return Use_LC ? (idx/mWidth) * BaseFuncs::getMoveCost(op) : BaseFuncs::getMoveCost(op);
		}
		
		void dump(std::ostream& out, bool pIsAdj, bool pCost, std::vector<unsigned> const& pOps) const {
			for(unsigned i=0; i<mHeight; i++) {
				for(unsigned j=0; j<mWidth; j++) {
					out << "[";
					
					if(pCost) {
						out << getOpCost(i*mWidth+j, pOps[0]);
						
						for(unsigned k=1; k<pOps.size(); k++)
							out << ", " << getOpCost(i*mWidth+j, pOps[k]);
					}
					else if(pIsAdj) {
						out << (bool)(mAdjList[i*mWidth+j].at(pOps[0]) != Null_Idx);
						
						for(unsigned k=1; k<pOps.size(); k++)
							out << ", " << (bool)(mAdjList[i*mWidth+j].at(pOps[k]) != Null_Idx);
					}
					else {
						if(mAdjList[i*mWidth+j].at(pOps[0]) != Null_Idx)
							out << mAdjList[i*mWidth+j].at(pOps[0]);
						else
							out << "*";
						
						for(unsigned k=1; k<pOps.size(); k++) {
							if(mAdjList[i*mWidth+j].at(pOps[k]) != Null_Idx)
								out << ", " << mAdjList[i*mWidth+j].at(pOps[k]);
							else
								out << ", *";
						}
					}
					out << "] ";
				}
				out << "\n";
			}
		}
		
		void drawMap(std::ostream& out) const {
			for(unsigned i=0; i<mHeight; i++) {
				for(unsigned j=0; j<mWidth; j++) {
					unsigned idx = i*mWidth+j;
					
					bool isOpen = i == 0 ? mAdjList.at(idx+mWidth)[0] != Null_Idx : mAdjList.at(idx-mWidth)[1] != Null_Idx;
					
					if(isOpen)
						out << ". ";
					else
						out << "O ";					
				}
				out << "\n";
			}			
		}
		
		
		unsigned const mHeight, mWidth, mSize;
		
		private:
		std::vector<AdjacentCells> mAdjList;
		std::vector<Cell_t> mCellMap;
	};
	
	



	template<typename BaseFuncs, bool Use_LC>
	struct StarAbtCellMap {
		
		using AdjacentCells = typename BaseFuncs::AdjacentCells;
		using Cost_t = typename BaseFuncs::Cost_t;
		
		
		struct InterGroupEdge {
			unsigned dst;
			Cost_t cost;
		};
	
		struct OutDegNode {
			unsigned idx, outdegree;
			
			bool operator()(OutDegNode const& a, OutDegNode const& b) {
				return a.outdegree == b.outdegree ? a.idx < b.idx : a.outdegree > b.outdegree;
			}
		};


		StarAbtCellMap(CellMap<BaseFuncs, Use_LC> const& pBaseMap, unsigned pRadius) :
			mBaseHeight(pBaseMap.mHeight),
			mBaseWidth(pBaseMap.mWidth),
			mBaseSize(pBaseMap.mSize),
			mRadius(pRadius),
			mBaseMap(pBaseMap),
			mBaseGroupLabels(),
			mCellAbstractGroup(),
			mGroupEdges()
		{
			
			std::vector<unsigned> cellgroups(mBaseSize, Null_Idx);
			
			unsigned curgrp = 0;
			for(unsigned i=0; i<mBaseSize; i++) {
				if(pBaseMap.getCells()[i] != Cell_t::Blocked)
					cellgroups[i] = curgrp++;
			}
			
			mBaseGroupLabels = cellgroups;
			
			mGroupEdges.push_back(std::vector<std::vector<InterGroupEdge>>());
			
			mGroupEdges[0].resize(curgrp);
			
			for(unsigned i=0; i<mBaseSize; i++) {
				if(cellgroups[i] == Null_Idx)
					continue;
				
				AdjacentCells adjcells = pBaseMap.getAdjCells(i);
				
				std::vector<InterGroupEdge> v;
				
				for(unsigned j=0; j<adjcells.size(); j++) {
					if(adjcells[j] == Null_Idx)
						continue;
					v.push_back(InterGroupEdge{ .dst=cellgroups[adjcells[j]], .cost=pBaseMap.getOpCost(i, j) });
				}
				
				mGroupEdges.at(0)[cellgroups[i]] = v;
			}
			
			
			
			unsigned curlvl = 0;
			while(true) {
				if(mGroupEdges.back().size() == 1)
					break;
					
				prepNextLevel(curlvl);
				curlvl++;
			
				if(mGroupEdges.back().size() == mGroupEdges.at(mGroupEdges.size()-2).size()) {
					mGroupEdges.pop_back();
					mCellAbstractGroup.pop_back();
					break;
				}
			}
		}
		
		
		void prepNextLevel(unsigned pLvl) {
			
			const unsigned spaceSz = mGroupEdges.at(pLvl).size();
			
			std::vector<unsigned> groupMembership(spaceSz, Null_Idx);
			std::priority_queue<OutDegNode, std::vector<OutDegNode>, OutDegNode> outDegreeQueue;
			
			
			//Assign group to each cell
			std::vector<unsigned> singletonIndices;
			
			for(unsigned i=0; i<spaceSz; i++) {
				outDegreeQueue.push(OutDegNode{ .idx=i, .outdegree=(unsigned)mGroupEdges[pLvl][i].size() });
			}
			
			while(!outDegreeQueue.empty()) {
				unsigned root = outDegreeQueue.top().idx;
				outDegreeQueue.pop();
				
				if(groupMembership[root] != Null_Idx)
					continue;
				
				if(setGroup_rec(root, 0, mRadius, root, groupMembership, pLvl) == 1)
					singletonIndices.push_back(root);
			}
			
			
			//Remove singleton groups, merge with a neighbour
			for(unsigned i=0; i<singletonIndices.size(); i++) {
				if(mGroupEdges[pLvl][singletonIndices[i]].size() > 0)
					groupMembership[singletonIndices[i]] = groupMembership[mGroupEdges[pLvl][singletonIndices[i]][0].dst];
			}
			
			//Normalise group labels
			unsigned curgrp = 0;
			std::map<unsigned, unsigned> groupRelabel;
			
			for(unsigned i=0; i<spaceSz; i++) {				
				if(groupRelabel.count(groupMembership[i]) == 0)
					groupRelabel[groupMembership[i]] = curgrp++;
				
				groupMembership[i] = groupRelabel[groupMembership[i]];
			}
			
			
			unsigned ngroups = groupRelabel.size();
			

			
			//For each group, find cheapest edge to each adjacent group.
			std::vector<std::vector<InterGroupEdge>> groupEdges;
			
			groupEdges.resize(ngroups);

			for(unsigned i=0; i<spaceSz; i++) {
				for(unsigned j=0; j<mGroupEdges[pLvl][i].size(); j++) {
					
					unsigned srcgrp = groupMembership[i];
					unsigned dstgrp = groupMembership[mGroupEdges[pLvl][i][j].dst];
					Cost_t edgecost = mGroupEdges[pLvl][i][j].cost;
					
					if(srcgrp == dstgrp)
						continue;
					
					bool createNewEntry = true;
					
					for(InterGroupEdge& e : groupEdges.at(srcgrp)) {
						
						if(e.dst == dstgrp) {
							createNewEntry = false;
							if(e.cost > edgecost)
								e.cost = edgecost;
							break;
						}
					}
					
					if(createNewEntry)
						groupEdges.at(srcgrp).push_back(InterGroupEdge{ .dst = dstgrp, .cost = edgecost });
				}
			}
			
			fast_assert(mCellAbstractGroup.size() == pLvl);
			fast_assert(mGroupEdges.size() == pLvl+1);
			
			mCellAbstractGroup.push_back(groupMembership);
			mGroupEdges.push_back(groupEdges);
		}
		
		unsigned setGroup_rec(unsigned i, unsigned depth, unsigned maxDepth, unsigned curgrp, std::vector<unsigned>& groupMembership, unsigned pLvl) {
			
			if(depth > maxDepth || groupMembership[i] != Null_Idx)
				return 0;
			
			groupMembership[i] = curgrp;
			
			unsigned ret = 0;
			for(InterGroupEdge e : mGroupEdges.at(pLvl).at(i)) {
				ret += setGroup_rec(e.dst, depth+1, maxDepth, curgrp, groupMembership, pLvl);
			}
			return ret + 1;
		}
		
		unsigned getAbstractGroup(unsigned pGroup, unsigned pLvl) {
			slow_assert(pLvl < mCellAbstractGroup.size());
			slow_assert(pGroup < mCellAbstractGroup[pLvl].size());
			
			return mCellAbstractGroup[pLvl][pGroup];
		}
		
		unsigned getLevels() {
			return mGroupEdges.size();
		}
		
		
		void dump(std::ostream& out, unsigned pLvl) {
			fast_assert(pLvl < mGroupEdges.size());
			
			std::vector<unsigned> groupmap = mBaseGroupLabels;
			gen_assert(groupmap.size() == mBaseSize);
			
			for(unsigned i=0; i<pLvl; i++) {
				for(unsigned j=0; j<mBaseSize; j++) {
					if(groupmap[j] == Null_Idx)
						continue;

					groupmap[j] = mCellAbstractGroup.at(i).at(groupmap[j]);
				}
			}
			
			for(unsigned i=0; i<mBaseHeight; i++) {
				for(unsigned j=0; j<mBaseWidth; j++) {
					if(groupmap[i*mBaseWidth+j] == Null_Idx)
						std::cout << ". ";
					else
						std::cout << groupmap[i*mBaseWidth+j] << " ";//(char)(groupmap[i*mBaseWidth+j] % 26 + 'A') << " ";
					}
				std::cout << "\n";
			}
			std::cout << "\n";
		}


		unsigned mBaseHeight;
		unsigned mBaseWidth;
		unsigned mBaseSize;
		unsigned mRadius;
		CellMap<BaseFuncs, Use_LC> const& mBaseMap;
		std::vector<unsigned> mBaseGroupLabels;
		std::vector<std::vector<unsigned>> mCellAbstractGroup;
		std::vector<std::vector<std::vector<InterGroupEdge>>> mGroupEdges; //(level, group) -> vector of edges
		
		
	};


	template<typename = void>
	void drawMap(std::vector<Cell_t> const& pMap, unsigned pHeight, unsigned pWidth, std::ostream& out) {
		for(unsigned i=0; i<pHeight; i++) {
			for(unsigned j=0; j<pWidth; j++) {
				unsigned idx = i*pWidth+j;

				if(pMap[idx] == Cell_t::Open)
					out << ". ";
				else if(pMap[idx] == Cell_t::Blocked)
					out << "O ";					
				else
					out << "? ";
			}
			out << "\n";
		}			
	}
}}}
