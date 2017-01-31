#pragma once

#include <array>
#include <iostream>
#include <random>
#include <vector>
#include <cmath>
#include <queue>

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
		
		Cost_t getOpCost(unsigned idx, unsigned op) {
			slow_assert(idx < mSize);
			slow_assert(op < Max_Adj);
			return Use_LC ? (idx/mWidth) * BaseFuncs::getMoveCost(op) : BaseFuncs::getMoveCost(op);
		}
		
		void dump(std::ostream& out, bool pIsAdj, bool pCost, std::vector<unsigned> const& pOps) {
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
		
		void drawMap(std::ostream& out) {
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
	
	



	template<unsigned N_Levels>
	struct StarAbtCellMap {
		
		
		struct InterGroupEdge {
			unsigned dst;
			Cost_t cost;
		};
	
		struct OutDegNode {
			unsigned idx, outdegree;
			
			operator()(OutDegNode const& a, OutDegNode const& b) {
				return a.outdegree == b.outdegree ? a.idx < b.idx : a.outdegree > b.outdegree;
			}
		};
		
		unsigned countEdges(AdjacentCells const& adjcells) {
			unsigned n = 0;
			for(unsigned i : adjcells)
				if(i != Null_Idx)
					n++;
			
			return n;
			
		}

		std::vector<std::vector<unsigned>> mCellAbstractGroup;
		std::vector<std::vector<std::vector<InterGroupEdge>>> mGroupEdges; //(level, group) -> vector of edges
		
		
		
		StarAbtCellMap(CellMap<BaseFuncs, Use_LC> const& pBaseMap) {
			
			std::vector<unsigned> cellgroups(pBaseMap.mSize, Null_Idx);
			
			unsigned curgrp = 0;
			for(unsigned i=0; i<mBaseHeight*mBaseWidth; i++) {
				if(pBaseMap.getCells()[i] != Cell_t::Blocked)
					cellgroups[i] = curgrp++;
			}
			
			for(unsigned i=0; i<pBaseMap.mSize; i++) {
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
				
				for(unsigned i=0; mGroupEdges.back().size() > 1; i++)
					prepNextLevel(i);
			}
		}
		
		
		void prepNextLevel(unsigned pLvl) {
			
			const unsigned spaceSz = mGroupEdges.at(pLvl).size();
			
			std::vector<unsigned> groupMembership(baseSz, Null_Idx);
			std::priority_queue<OutDegNode, std::vector<OutDegNode>, OutDegNode> outDegreeQueue;
			
			std::vector<unsigned> singletonIndices;
			
			
			
			for(unsigned i=0; i<baseSz; i++) {
				/*
				if(pBaseMap.getCellMap[i] == Cell_t::Blocked)
					continue;
				*/
				outDegreeQueue.push(OutDegNode{ .idx=i, .outdegree=mGroupEdges[pLvl][i].size() });
			}

			
			//Assign group to each cell
			while(!outDegreeQueue.empty()) {
				unsigned root = outDegreeQueue.top().idx;
				outDegreeQueue.pop();
				
				if(groupMembership[root] != Null_Idx)
					continue;
				
				if(setGroup_rec(root, 0, maxDepth, root, groupMembership, pLvl) == 1)
					singletonIndices.push_back(root);
			}
			
			
			//Remove singleton groups
			for(unsigned i=0; i<singletonIndices.size(); i++) {				
				if(mGroupEdges[pLvl][i].size() > 0)
					groupMembership[i] = groupMembership[mGroupEdges[i][0].dst];
			}
			
			//Normalise group labels
			unsigned curgrp = 0;
			std::unordered_map<unsigned, unsigned> groupRelabel;
			
			for(unsigned i=0; i<pBaseMap.mSize; i++) {
				if(pBaseMap[i] == Cell_t::Blocked)
					continue;
				
				if(groupRelabel.count(groupMembership[i]) == 0)
					groupRelabel[groupMembership[i]] = curgrp++;
				
				groupMembership[i] = groupRelabel[groupMembership[i]];
			}
			
			
			unsigned ngroups = groupRelabel.size();
			

			
			//For all pairs of adjacent groups, find minimum edge cost between them.
			std::unordered_map<unsigned, std::vector<InterGroupEdge>> groupEdges;
			
			for(unsigned i=0; i<ngroups; i++)
				groupEdges[i] = std::vector<InterGroupEdge>();
			
			
			for(unsigned i=0; i<baseSz; i++) {
				for(unsigned j=0; j<mGroupEdges[pLvl][i].size(); j++) {
					
					unsigned srcgrp = groupMembership[i];
					unsigned dstgrp = groupMembership[mGroupEdges[pLvl][i][j].dst];
					Cost_t edgecost = mGroupEdges[pLvl][i][j].cost;
					
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
						groupEdges.at(src).push_back(InterGroupEdge{ .dst = dst, .cost = edgecost });
				}
			}
			
			mCellAbstractGroup.at(pLvl) = groupMembership;
			mGroupEdges.at(pLvl+1) = groupEdges;
		}
		
		unsigned setGroup_rec(unsigned i, unsigned depth, unsigned maxDepth, unsigned curgrp, std::vector<unsigned>& groupMembership) {
			
			if(depth > maxDepth || groupMembership[i] != Null_Idx)
				return;
			
			groupMembership[i] = curgrp;
			
			unsigned ret = 0;
			for(unsigned j : mBaseMap.getAdjCells(i)) {
				if(j == Null_Idx)
					continue;
				ret += setGroup_rec(j, depth+1, maxDepth, curgrp, groupMembership, adjlist);
			}
			return ret + 1;
		}

		void prepHigherLevels(unsigned pLvl) {
			
			
		}
	};

/*
	template<typename BaseFuncs, bool Use_LC, unsigned H_, unsigned W_>
	struct AbstractCellMap {
		
		using Cost_t = typename BaseFuncs::Cost_t;
		using AdjacentCells = typename BaseFuncs::AdjacentCells;
		
		struct BestHorizontalRows {
			unsigned left, right;
		};


		AbstractCellMap(CellMap<BaseFuncs, Use_LC> const& pBaseMap, unsigned pAbtFact) :
			mBaseHeight		(pBaseMap.mHeight),
			mBaseWidth		(pBaseMap.mWidth),
			mBaseSize		(pBaseMap.mSize),
			mBlkH			(pAbtFact * H_),
			mBlkW			(pAbtFact * W_),
			mHeight			(mBaseHeight / mBlkH + (mBaseHeight % mBlkH == 0 ? 0 : 1)),
			mWidth			(mBaseWidth / mBlkW + (mBaseWidth % mBlkW == 0 ? 0 : 1)),
			mSize			(mHeight * mWidth),
			mAbtFact		(pAbtFact),
			mAdjList		(mSize),
			mBestHorzRow	()
		{
			if(Use_LC)
				mBestHorzRow.resize(mSize);
			prepAdjList(pBaseMap.getAdjCellsList());
		}

		
		AdjacentCells const& getAdjCells(unsigned idx) const {
			return mAdjList[idx];
		}
		
		std::vector<AdjacentCells> const& getAdjCellsList() const {
			return mAdjList;
		}

		Cost_t getOpCost(unsigned idx, unsigned opu) {
			slow_assert(idx < mSize);
			slow_assert(opu < BaseFuncs::Max_Adj);
			
			Cost_t c = BaseFuncs::getMoveCost(opu);
			
			CardDir_t op = (CardDir_t)opu;
			
			if(Use_LC) {
				if(op == CardDir_t::W)
					c *= mBestHorzRow[idx].left;
				else if(op == CardDir_t::E)
					c *= mBestHorzRow[idx].right;
				
				else if(op == CardDir_t::N || op == CardDir_t::NW || op == CardDir_t::NE)
					c *= idx/mWidth * mBlkH;
				
				else
					c *= (idx/mWidth+1) * mBlkH - 1;
			}
			return c;
		}
		
		void prepAdjList(std::vector<AdjacentCells> const& pBaseMap) {
			for(unsigned y=0; y<mHeight; y++) {
				for(unsigned x=0; x<mWidth; x++) {
					
					unsigned selfi = y*mWidth + x;
					unsigned bx0 = x * mBlkW;
					unsigned bx1 = mathutil::min((x+1)*mBlkW-1, mBaseWidth-1);
					unsigned by0 = y * mBlkH;
					unsigned by1 = mathutil::min((y+1)*mBlkH-1, mBaseHeight-1);
					
					for(unsigned i=bx0; i<=bx1; i++) {
						unsigned p = by0*mBaseWidth + i;
					
						if(pBaseMap[p][0] != Null_Idx) {
							mAdjList[selfi][0] = selfi - mWidth;
							break;
						}
						if(BaseFuncs::Max_Adj == 8 && i!=bx0 && i!=bx1 && (pBaseMap[p][4] != Null_Idx || pBaseMap[p][5] != Null_Idx)) {
							mAdjList[selfi][0] = selfi - mWidth;
							break;
						}
					}
					
					for(unsigned i=bx0; i<=bx1; i++) {
						unsigned p = by1*mBaseWidth + i;
					
						if(pBaseMap[p][1] != Null_Idx) {
							mAdjList[selfi][1] = selfi + mWidth;
							break;
						}
						if(BaseFuncs::Max_Adj == 8 && i!=bx0 && i!=bx1 && (pBaseMap[p][6] != Null_Idx || pBaseMap[p][7] != Null_Idx)) {
							mAdjList[selfi][1] = selfi + mWidth;
							break;
						}
					}

					for(unsigned i=by0; i<=by1; i++) {
						unsigned p = i*mBaseWidth + bx0;
					
						if(pBaseMap[p][2] != Null_Idx) {
							mAdjList[selfi][2] = selfi - 1;
							if(Use_LC) mBestHorzRow[selfi].left = i;
							break;
						}
						if(BaseFuncs::Max_Adj == 8 &&
							((i!=by0 && pBaseMap[p][4] != Null_Idx) || (i!=by1 && pBaseMap[p][6] != Null_Idx))) {
							mAdjList[selfi][2] = selfi - 1;
							if(Use_LC) mBestHorzRow[selfi].left = i;
							break;
						}
					}
					
					for(unsigned i=by0; i<=by1; i++) {
						unsigned p = i*mBaseWidth + bx1;
					
						if(pBaseMap[p][3] != Null_Idx) {
							mAdjList[selfi][3] = selfi + 1;
							if(Use_LC) mBestHorzRow[selfi].right = i;
							break;
						}
						if(BaseFuncs::Max_Adj == 8 &&
							((i!=by0 && pBaseMap[p][5] != Null_Idx) || (i!=by1 && pBaseMap[p][7] != Null_Idx))) {
							mAdjList[selfi][3] = selfi + 1;
							if(Use_LC) mBestHorzRow[selfi].right = i;
							break;
						}
					}
					
					if(BaseFuncs::Max_Adj != 8)
						continue;
					
					if(pBaseMap[by0*mBaseWidth+bx0][4] != Null_Idx)
						mAdjList[selfi][4] = selfi - mWidth - 1;
					
					if(pBaseMap[by0*mBaseWidth+bx1][5] != Null_Idx)
						mAdjList[selfi][5] = selfi - mWidth + 1;

					if(pBaseMap[by1*mBaseWidth+bx0][6] != Null_Idx)
						mAdjList[selfi][6] = selfi + mWidth - 1;
						
					if(pBaseMap[by1*mBaseWidth+bx1][7] != Null_Idx)
						mAdjList[selfi][7] = selfi + mWidth + 1;
				}
			}
		}
		
		void dump(std::ostream& out, bool pIsAdj, bool pCost, std::vector<unsigned> const& pOps) {
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
		
		void drawMap(std::ostream& out) {
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
		
		
		unsigned const mBaseHeight, mBaseWidth, mBaseSize;
		unsigned const mBlkH, mBlkW;
		unsigned const mHeight, mWidth, mSize;
		unsigned const mAbtFact;
		std::vector<AdjacentCells> mAdjList;
		std::vector<BestHorizontalRows> mBestHorzRow;
	};
*/


	
	struct CompD_Node {
		unsigned idx;
		unsigned outdegree;
	};
	
	struct CompD_ops {
		bool operator()(CompD_Node const& a, CompD_Node const& b) {
			return a.outdegree < b.outdegree;
		}
	};
	
	template<typename BaseFuncs>
	unsigned compD_getOutDeg(std::vector<Cell_t> const& pBaseMap, std::vector<unsigned> const& cellGroup, unsigned pBaseHeight, unsigned pBaseWidth, unsigned idx) {
		unsigned outdeg = 0;
		
		typename BaseFuncs::AdjacentCells adjcells;
		BaseFuncs::getAllEdges(pBaseHeight, pBaseWidth, idx, adjcells);
		
		for(unsigned i=0; i<BaseFuncs::Max_Adj; i++)
			if(adjcells[i] != Null_Idx && cellGroup[adjcells[i]] == Null_Idx && pBaseMap[adjcells[i]] != Cell_t::Blocked)
				outdeg++;
		
		return outdeg;
	}
	
	

	std::vector<unsigned> compressMapD(std::vector<Cell_t> const& pBaseMap, unsigned pBaseHeight, unsigned pBaseWidth) {
	
		using BaseFuncs = FourWayFuncs<>;
		using AdjacentCells = typename BaseFuncs::AdjacentCells;
		
		std::vector<unsigned> cellGroup(pBaseMap.size(), Null_Idx);
		
		unsigned curgrp = 0;
		
		std::mt19937 gen;
		std::uniform_distribution dst(0, pBaseWidth*pBaseHeight);
		
		
		while(true) {
			unsigned root;
			
			while(true) {
				root = dst(gen);
				
			}
			/*
			for(unsigned i=0; i<pBaseHeight*pBaseWidth; i++) {
				if(cellGroup.at(i) == Null_Idx && pBaseMap.at(i) != Cell_t::Blocked) {
					root = i;
					break;
				}
			}
			
			if(root == Null_Idx)
				break;
			
			*/
			
			unsigned failedPicks = 0;
			
			
			cellGroup.at(root) = curgrp++;
			//std::vector<unsigned> frontier = root;
			std::priority_queue<CompD_Node, std::vector<CompD_Node>, CompD_ops> frontier;
			
			frontier.push(CompD_Node{.idx=root, .outdegree=compD_getOutDeg<FourWayFuncs<>>(pBaseMap, cellGroup, pBaseHeight, pBaseWidth, root)});


			unsigned numcells = 0;
			double maxPressure = 0;
			
			while(!frontier.empty()) {
				
				CompD_Node n = frontier.top();
				frontier.pop();
				
				numcells++;
				
				
				
				if(maxoutdeg > 1 && n.outdegree <= 1)
					continue;
				
				if(maxoutdeg < n.outdegree)
					maxoutdeg = n.outdegree;

				//cellGroup[n.idx] = curgrp;
				
				AdjacentCells adjcells;
				BaseFuncs::getAllEdges(pBaseHeight, pBaseWidth, n.idx, adjcells);

				
				for(unsigned i=0; i<BaseFuncs::Max_Adj; i++) {
				
					if(adjcells[i] == Null_Idx || cellGroup[adjcells[i]] != Null_Idx || pBaseMap[adjcells[i]] == Cell_t::Blocked)
						continue;
					
					cellGroup[adjcells[i]] = cellGroup[n.idx];
				
					AdjacentCells adjkid;
					BaseFuncs::getAllEdges(pBaseHeight, pBaseWidth, adjcells[i], adjkid);
				

					unsigned kidoutdeg = compD_getOutDeg<FourWayFuncs<>>(pBaseMap, cellGroup, pBaseHeight, pBaseWidth, adjcells[i]);

					if((kidoutdeg == 1 && maxoutdeg == 1) || kidoutdeg > 1)
						frontier.push(CompD_Node{.idx=adjcells[i], .outdegree=kidoutdeg});
				}
			
				
			}	
		}

		return cellGroup;
	}


/*
	std::vector<unsigned> compressMapE(std::vector<Cell_t> const& pBaseMap, unsigned pBaseHeight, unsigned pBaseWidth) {
	
		using BaseFuncs = FourWayFuncs<>;
		using AdjacentCells = typename BaseFuncs::AdjacentCells;
		
		std::vector<unsigned> cellGroup(pBaseMap.size(), Null_Idx);
		
		unsigned curgrp = 0;
		
		std::priority_queue<CompD_Node, std::vector<CompD_Node>, CompD_ops> mapOutDegree;
		
		for(unsigned i=0; i<pBaseHeight*pBaseWidth; i++) {
			
			mapOutDegree.push(CompD_Node{.idx=i, .outdegree=compD_getOutDeg(pBaseMap, cellGroup, pBaseHeight, pBaseWidth, i)});
		}
		
		
		
		
		while(true) {
			
			CompD_Node n = map
			
		}
		
		
		
		
	}
*/
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
