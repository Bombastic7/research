#pragma once

#include <array>
#include <iostream>
#include <vector>
#include <cmath>



namespace mjon661 { namespace gridnav { namespace blocked {


	enum struct Cell_t {
		Open, Blocked, Null
	};
	
	template<typename = void>
	struct FourWayFuncs {
		using Cost_t = int;
		using AdjacentCells = std::array<unsigned, 4>; 
	
		static const unsigned Max_Adj = 4;
		
		static void getAllEdges(unsigned pHeight, unsigned pWidth, unsigned i, AdjacentCells& pEdges) {
			pEdges = {Cell_t::Null};
			
			if(i >= pWidth) 			pEdge[0] = i-pWidth;
			if(i < (pHeight-1)*pWidth) 	pEdge[1] = i+pWidth;
			if(i % pWidth != 0) 		pEdge[2] = i-1;
			if((i+1) % pWidth != 0) 	pEdge[3] = i+1;
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
		using AdjacentCells = std::array<unsigned, 8>; 
		
		static const Cost_t Diag_Mv_Cost = std::sqrt(2);
		static const unsigned Max_Adj = 8;
		
		static void getAllEdges(unsigned pHeight, unsigned pWidth, unsigned i, AdjacentCells& pEdges) {
			pEdges = {Cell_t::Null};

			if(i >= pWidth) 			pEdge[0] = i-pWidth;
			if(i < (pHeight-1)*pWidth) 	pEdge[1] = i+pWidth;
			if(i % pWidth != 0) 		pEdge[2] = i-1;
			if((i+1) % pWidth != 0) 	pEdge[3] = i+1;
			
			if(pEdge[0] != Cell_t::Null && pEdge[2] != Cell_t::Null) pEdge[4] = i-pWidth - 1;
			if(pEdge[0] != Cell_t::Null && pEdge[3] != Cell_t::Null) pEdge[5] = i-pWidth + 1;
			if(pEdge[1] != Cell_t::Null && pEdge[2] != Cell_t::Null) pEdge[6] = i+pWidth - 1;
			if(pEdge[1] != Cell_t::Null && pEdge[3] != Cell_t::Null) pEdge[7] = i+pWidth + 1;
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
	struct CellGraph {
		
		using Cost_t = BaseFuncs::Cost_t;
		using AdjacentCells = BaseFuncs::AdjacentCells;
		
		static const Cost_t Null_Cost = -1;
		static const Max_Adj = BaseFuncs::Max_Adj;
		
		CellGraph(unsigned pHeight, unsigned pWidth, std::istream& in) :
			mHeight(pHeight),
			mWidth(pWidth),
			mSize(pHeight * pWidth),
			mAdjList(mSize)
		{
			gen_assert(mSize > 0);
			
			std::vector<Cell_t> cellMap;
			cellMap.reserve(mSize);
			
			for(unsigned i=0; i<mSize; i++) {
				int c = in.read();
				gen_assert(c == Cell_t::Open || c == Cell_t::Blocked);
				cellMap.push_back(c);
			}
			
			for(unsigned i=0; i<mSize; i++) {
				BaseFuncs::getAllEdges(mHeight, mWidth, i, mAdjList[i]);
			}
			
			for(unsigned i=0; i<mSize; i++) {
				for(unsigned j=0; j<Max_Adj; j++) {
					if(mAdjList[i][j] == Cell_t::Null)
						continue;
						
					if(cellMap[mAdjList[i][j]] == Cell_t::Blocked)
						mAdjList[i][j] = Cell_t::Null;
				}
			}
		}
		
		AdjacentCells const& getAdjCells(unsigned idx) {
			slow_assert(idx < mSize);
			return mAdjList[idx];
		}
		
		Cost_t getOpCost(unsigned idx, unsigned op) {
			slow_assert(idx < mSize);
			slow_assert(op < Max_Adj);
			return Use_LC ? idx/mWidth * BaseFuncs::getMoveCost(op) : BaseFuncs::getMoveCost(op);
		}
		
		private:
		unsigned const mHeight, mWdith, mSize;
		std::vector<AdjacentCells> mAdjList;
	};


}}}
