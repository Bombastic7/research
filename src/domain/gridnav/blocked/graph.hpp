#pragma once

#include <array>
#include <iostream>
#include <random>
#include <vector>
#include <cmath>
#include <queue>
#include <string>
#include <fstream>
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
		
		static unsigned reverseOp(unsigned op) {
			if(op == 0 || op == 2)
				return op+1;
			
			return op-1;
		}
		
		static void unitCostHeuristics(unsigned pPos, unsigned pGoal, unsigned pWidth, Cost_t& out_h, Cost_t& out_d) {
			out_h = out_d = manhat(pPos, pGoal, pWidth);
		}
		
		static void lifeCostHeuristics(unsigned pPos, unsigned pGoal, unsigned pWidth, Cost_t& out_h, Cost_t& out_d) {
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
		
		static unsigned reverseOp(unsigned op) {
			if(op == 0 || op == 2)
				return op+1;
			if(op == 1 || op == 3)
				return op-1;
			
			if(op == 4) return 7;
			if(op == 5) return 6;
			if(op == 6) return 5;
			if(op == 7) return 4;
		}
		
		static void unitCostHeuristics(unsigned pPos, unsigned pGoal, unsigned pWidth, unsigned& out_h, unsigned& out_d) {
			int dx = std::abs(pPos % pWidth - pGoal % pWidth), dy = std::abs(pPos / pWidth - pGoal / pWidth);
				
			out_h = std::abs(dx-dy) + mathutil::min(dx, dy) * Diag_Mv_Cost;
			out_d = mathutil::max(dx, dy);
		}
		
		static void lifeCostHeuristics(unsigned pPos, unsigned pGoal, unsigned pWidth, unsigned& out_h, unsigned& out_d) {
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
	
	


	template<typename BaseFuncs, bool Use_LC>
	struct CellMap {
		
		using Cost_t = typename BaseFuncs::Cost_t;
		using AdjacentCells = typename BaseFuncs::AdjacentCells;

		static const unsigned Max_Adj = BaseFuncs::Max_Adj;
		
		CellMap(unsigned pHeight, unsigned pWidth, std::string const& infile) :
			mHeight(pHeight),
			mWidth(pWidth),
			mSize(pHeight * pWidth),
			mAdjList(mSize),
			mCellMap()
		{
			gen_assert(mSize > 0);
			
			std::ifstream in(infile);
			
			gen_assert(in);
			
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
		
		void getHeuristicValues(unsigned pPos, unsigned pGoal, Cost_t& out_h, Cost_t& out_d) const {
			if(Use_LC)
				BaseFuncs::lifeCostHeuristics(pPos, pGoal, mWidth, out_h, out_d);
			else
				BaseFuncs::unitCostHeuristics(pPos, pGoal, mWidth, out_h, out_d);
		}

		unsigned reverseOp(unsigned op) const {
			return BaseFuncs::reverseOp(op);
		}
		
		void prettyPrintIndex(unsigned i, std::ostream& out) const {
			out << "( " << i % mWidth << ", " << i / mWidth << " )\n";
		}
		
		void prettyPrintDir(unsigned i, std::ostream& out) const {
			out << BaseFuncs::getOpName(i) << "\n";
		}

		
		unsigned const mHeight, mWidth, mSize;
		
		private:
		std::vector<AdjacentCells> mAdjList;
		std::vector<Cell_t> mCellMap;
	};
	
	


	//STAR abstraction hierarchy. A CellMap serves as the base space. Abstraction radius (for all levels) specified as pRadius in ctor.
	//Hub/root node selection is by max outdegree, then by min cell/group index.
	//If any singleton groups are produced with the specified radius, they are (deterministically) merged into a neighbouring group.
	//Abstraction levels are created until none of the last level's groups are adjacent, or only one group exists.
	//Level 0 is the base space, but with open cells identified by an index [0, number of open cells - 1].
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
				return a.outdegree == b.outdegree ? a.idx > b.idx : a.outdegree < b.outdegree;
			}
		};


		StarAbtCellMap(CellMap<BaseFuncs, Use_LC> const& pBaseMap, unsigned pRadius) :
			mBaseHeight(pBaseMap.mHeight),
			mBaseWidth(pBaseMap.mWidth),
			mBaseSize(pBaseMap.mSize),
			mRadius(pRadius),
			mBaseGroupLabels(),
			mCellAbstractGroup(),
			mGroupEdges()
		{
			//Assign a unique group to each open cell in the base space.
			std::vector<unsigned> cellgroups(mBaseSize, Null_Idx);
			
			unsigned curgrp = 0;
			for(unsigned i=0; i<mBaseSize; i++) {
				if(pBaseMap.getCells()[i] != Cell_t::Blocked)
					cellgroups[i] = curgrp++;
			}
			
			mBaseGroupLabels = cellgroups;
			
			mGroupEdges.push_back(std::vector<std::vector<InterGroupEdge>>());
			
			//Extract base space edge info, and translate it into the group representation.
			mGroupEdges[0].resize(curgrp);
			
			for(unsigned i=0; i<mBaseSize; i++) {
				if(cellgroups[i] == Null_Idx)
					continue;
				
				//For each open cell, get its neighbours.
				AdjacentCells adjcells = pBaseMap.getAdjCells(i);
				
				std::vector<InterGroupEdge> v;
				
				for(unsigned j=0; j<adjcells.size(); j++) {
					if(adjcells[j] == Null_Idx)
						continue;
					//Record the edge between the cell at index j and it's neighbour, in terms of their assigned groups.
					v.push_back(InterGroupEdge{ .dst=cellgroups[adjcells[j]], .cost=pBaseMap.getOpCost(i, j) });
				}
				
				mGroupEdges.at(0)[cellgroups[i]] = v;
			}

			//mGroupEdges[0] is populated. At level 0 (base space), for any group on that level, we know which groups are connected.
			//Now assign level 1 groups to level 0 groups, record that mapping in mCellAbstractGroup[0][L1 group] -> L2 group.
			//And prepare mGroupEdges[1].
			//Continue until no further abstraction is possible.
			unsigned curlvl = 0;
			while(true) {
				bool reachedLast = true;
				
				for(unsigned i=0; i<mGroupEdges[curlvl].size(); i++)
					if(mGroupEdges[curlvl][i].size() != 0) {
						reachedLast = false;
						break;
					}
				
				if(reachedLast)
					break;
			
				prepNextLevel(curlvl);
				curlvl++;
			}
		}
		
		unsigned abstractBaseCell(unsigned idx, unsigned pLvl) const {
			if(pLvl >= mGroupEdges.size())
				return 0;

			fast_assert(idx < mBaseGroupLabels.size());
			
			unsigned c = mBaseGroupLabels[idx];
			
			fast_assert(c != Null_Idx, "%u %u", idx%mBaseWidth, idx/mBaseWidth);
			
			for(unsigned i=0; i<pLvl; i++) {
				c = mCellAbstractGroup.at(i).at(c);
			}
			return c;
		}
		
		unsigned checkBaseConnected(unsigned a, unsigned b) const {
			if(mBaseGroupLabels.at(a) == Null_Idx || mBaseGroupLabels.at(b) == Null_Idx)
				return false;

			a = abstractBaseCell(a, mGroupEdges.size()-1);
			b = abstractBaseCell(b, mGroupEdges.size()-1);
			return a == b;
		}
		
		unsigned getAbstractGroup(unsigned pGroup, unsigned pLvl) const {
			slow_assert(pLvl != 0 && pLvl < mCellAbstractGroup.size(), "%u %u", pLvl, mCellAbstractGroup.size());
			slow_assert(pGroup < mCellAbstractGroup[pLvl].size());
				
			return mCellAbstractGroup[pLvl][pGroup];
		}

		std::vector<unsigned> getLevelSizes() const {
			std::vector<unsigned> v;
			
			for(unsigned i=0; i<mGroupEdges.size(); i++) {
				v.push_back(mGroupEdges[i].size());
			}
			return v;
		}
		
		std::vector<std::vector<InterGroupEdge>> const& getGroupEdges(unsigned pLvl) const {
			if(pLvl >= mGroupEdges.size())
				return mGroupEdges.back();

			return mGroupEdges[pLvl];
		}
		
		void dump(std::ostream& out, unsigned pLvl) const {
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
						out << " ";
					else
						out << (char)(groupmap[i*mBaseWidth+j] % 26 + 'a');
					}
				out << "\n";
			}
			out << "\n";
		}
		
		void dumpall(std::ostream& out) const {
			for(unsigned i=0; i<mGroupEdges.size(); i++) {
				out << i << "\n";
				dump(out, i);
				out << "\n";
				
				for(unsigned j=0; j<mGroupEdges[i].size(); j++) {
					out << j << ": ";
					
					for(unsigned k=0; k<mGroupEdges[i][j].size(); k++)
						out << mGroupEdges[i][j][k].dst << " ";
					
					out << "\n";
				}
				out << "\n\n";
			}
		}
		
		void writeTopAbstraction(std::ostream& out) {
			dump(out, mGroupEdges.size()-1);
		}
		
		private:
		
		
		//Using the information in mGroupEdges[pLvl], create mCellAbstractGroup[pLvl] and mGroupEdges[pLvl+1].
		void prepNextLevel(unsigned pLvl) {
			
			const unsigned spaceSz = mGroupEdges.at(pLvl).size(); //Number of groups in level pLvl.
			
			std::vector<unsigned> groupMembership(spaceSz, Null_Idx); //Record what group each level pLvl group is abstracted 
																		//to in level pLvl+1.
																		
			std::priority_queue<OutDegNode, std::vector<OutDegNode>, OutDegNode> outDegreeQueue;
			
			
			//Assign pLvl+1 group to each pLvl group.
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
			
			
			//Remove singleton groups if possible, merge with a neighbour.
			for(unsigned i=0; i<singletonIndices.size(); i++) {
				if(mGroupEdges[pLvl][singletonIndices[i]].size() > 0)
					groupMembership[singletonIndices[i]] = groupMembership[mGroupEdges[pLvl][singletonIndices[i]][0].dst];
			}
			
			//Normalise group labels, 0 to [ngroups at pLvl+1] - 1.
			unsigned curgrp = 0;
			std::map<unsigned, unsigned> groupRelabel;
			
			for(unsigned i=0; i<spaceSz; i++) {				
				if(groupRelabel.count(groupMembership[i]) == 0)
					groupRelabel[groupMembership[i]] = curgrp++;
				
				groupMembership[i] = groupRelabel[groupMembership[i]];
			}
			
			
			unsigned ngroups = groupRelabel.size(); //Number of groups at pLvl+1.
			

			
			//Iterate through every edge in level pLvl. If the source and destination are abstracted to different groups in 
			//level pLvl+1, record it if its the cheapest between those groups seen so far.
			
			std::vector<std::vector<InterGroupEdge>> groupEdges; //Edges for groups in level pLvl+1.
			
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


		unsigned mBaseHeight;
		unsigned mBaseWidth;
		unsigned mBaseSize;
		unsigned mRadius;
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
