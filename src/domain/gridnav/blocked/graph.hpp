#pragma once

#include <array>
#include <iostream>
#include <vector>
#include <cmath>

#include "util/debug.hpp"
#include "util/math.hpp"


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
			pEdges = {Null_Idx};
			
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
		
		struct AdjacentCells : public std::array<unsigned, 8> {
			AdjacentCells() {
				this->fill(Null_Idx);
			}
		}; 
		
		static const Cost_t Diag_Mv_Cost = std::sqrt(2);
		static const unsigned Max_Adj = 8;
		
		static void getAllEdges(unsigned pHeight, unsigned pWidth, unsigned i, AdjacentCells& pEdges) {
			pEdges = {Null_Idx};

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
	struct CellMap {
		
		using Cost_t = BaseFuncs::Cost_t;
		using AdjacentCells = BaseFuncs::AdjacentCells;
		
		static const Cost_t Null_Cost = -1;
		static const Max_Adj = BaseFuncs::Max_Adj;
		
		CellMap(unsigned pHeight, unsigned pWidth, std::istream& in) :
			mHeight(pHeight),
			mWidth(pWidth),
			mSize(pHeight * pWidth),
			mRowMul(1),
			mAdjList(mSize)
		{
			gen_assert(mSize > 0);
			
			std::vector<Cell_t> cellMap;
			cellMap.reserve(mSize);
			
			for(unsigned i=0; i<mSize; i++) {
				std::string s = in.read();
				Cell_t c = std::strtol(s.begin(), nullptr, 10);
				
				gen_assert(c == Cell_t::Open || c == Cell_t::Blocked);
				cellMap.push_back((c);
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
		}
		
		AdjacentCells const& getAdjCells(unsigned idx) {
			slow_assert(idx < mSize);
			return mAdjList[idx];
		}
		
		static Cost_t getOpCost(unsigned idx, unsigned op) {
			slow_assert(idx < mSize);
			slow_assert(op < Max_Adj);
			return Use_LC ? (idx/mWidth) * BaseFuncs::getMoveCost(op) : BaseFuncs::getMoveCost(op);
		}
		
		
		unsigned const mHeight, mWidth, mSize;
		unsigned const mRowMul;
		
		private:
		std::vector<AdjacentCells> mAdjList;
	};




	template<typename BaseFuncs, bool Use_LC, unsigned H_, unsigned W_>
	struct AbstractCellMap {
		
		using Cost_t = BaseFuncs::Cost_t;
		using AdjacentCells = typename BaseFuncs::AdjacentCells;
		
		struct BestHorizontalRows {
			unsigned left, right;
		};
		
		private:
		
		template<typename BaseMap_t>
		AbstractCellMap(BaseMap_t const& pBaseMap) :
			mBaseHeight(pBaseMap.mHeight),
			mBaseWidth(pBaseMap.mWidth),
			mBaseSize(pBaseMap.mSize),
			mHeight(mBaseHeight / H_ + (mBaseHeight % H_ == 0 ? 0 : 1)),
			mWidth(mBaseWidth / W_ + (mBaseWidth % W_ == 0 ? 0 : 1)),
			mSize(mHeight * mWidth),
			mRowMul(H_ * pBaseMap.mRowMul),
			mBaseRowMul(pBaseMap.mRowMul),
			mAdjList(mSize),
			mBestHorzRow()
		{
			if(Use_LC)
				mBestHorzRow.resize(mSize);
			prepAdjList(pBaseMap);
		}
		
		
		public:
		
		AbstractCellMap(CellMap<BaseFuncs, Use_LC> const& pBaseMap) :
			AbstractCellMap<CellMap<BaseFuncs, Use_LC>>(pBaseMap)
		{}
		
		AbstractCellMap(AbstractCellMap<BaseFuncs, Use_LC, H_, W_> const& pBaseMap) :
			AbstractCellMap<AbstractCellMap<BaseFuncs, Use_LC, H_, W_>>(pBaseMap)
		{}
		
		
		AdjacentCells const& getAdjCells(unsigned idx) {
			return mAdjList[idx];
		}
		
		Cost_t getOpCost(unsigned idx, unsigned op) {
			slow_assert(idx < mSize);
			slow_assert(op < BaseFuncs::Max_Adj);
			
			Cost_t c = BaseFuncs::getMoveCost(op);
			
			if(Use_LC) {
				if(op == CardDir_t::W)
					c *= mBestHorzRow[idx].left;
				else if(op == CardDir_t::E)
					c *= mBestHorzRow[idx].right;
				
				else if(op == CardDir_t::N || op == CardDir_t::NW || op == CardDir_t::NE)
					c *= idx/mWidth * mRowMul;
				
				else
					c *= (idx/mWidth+1) * mRowMul - 1;
			}
			return c;
		}
		
		void prepAdjList(std::vector<AdjacentCells> const& pBaseMap) {
			for(unsigned y=0; y<mHeight; y++) {
				for(unsigned x=0; x<mWidth; x++) {
					
					unsigned selfi = y*mWidth + x;
					unsigned bx0 = x * W_;
					unsigned bx1 = mathutil::min((x+1)*W_-1, mBaseWidth-1);
					unsigned by0 = y * H_;
					unsigned by1 = mathutil::min((y+1)*H_-1, mBaseHeight-1);
					
					for(unsigned i=bx0; i<=bx1; i++) {
						unsigned p = by0*mBaseWidth + i;
					
						if(pBaseMap.getAdjCells(p)[0] != Null_Idx) {
							mAdjList[selfi][0] = selfi - mWidth;
							break;
						}
					}
					
					for(unsigned i=bx0; i<=bx1; i++) {
						unsigned p = by1*mBaseWidth + i;
					
						if(pBaseMap.getAdjCells(p)[1] != Null_Idx) {
							mAdjList[selfi][1] = selfi + mWidth;
							break;
						}
					}

					for(unsigned i=by0; i<=by1; i++) {
						unsigned p = i*mBaseWidth + bx0;
					
						if(pBaseMap.getAdjCells(p)[2] != Null_Idx) {
							mAdjList[selfi][2] = selfi - 1;
							mBestHorzRow[selfi].left = mBaseRowMul * i;
							break;
						}
					}
					
					for(unsigned i=by0; i<=by1; i++) {
						unsigned p = i*mBaseWidth + bx1;
					
						if(pBaseMap.getAdjCells(p)[3] != Null_Idx) {
							mAdjList[selfi][3] = selfi + 1;
							mBestHorzRow[selfi].right = mBaseRowMul * i;
							break;
						}
					}
					
					if(!Use_LC)
						continue;
					
					if(pBaseMap.getAdjCells(by0*mBaseWidth+bx0)[4] != Null_Idx)
						mAdjList[selfi][4] = selfi - mWidth - 1;
					
					if(pBaseMap.getAdjCells(by0*mBaseWidth+bx1)[5] != Null_Idx)
						mAdjList[selfi][5] = selfi - mWidth + 1;

					if(pBaseMap.getAdjCells(by1*mBaseWidth+bx0)[6] != Null_Idx)
						mAdjList[selfi][6] = selfi + mWidth - 1;
						
					if(pBaseMap.getAdjCells(by1*mBaseWidth+bx1)[7] != Null_Idx)
						mAdjList[selfi][7] = selfi + mWidth + 1;
				}
			}
		}
		


		
		unsigned const mBaseHeight, mBaseWidth, mBaseSize;
		unsigned const mHeight, mWidth, mSize;
		unsigned const mRowMul, mBaseRowMul;
		std::vector<AdjacentCells> mAdjList;
		std::vector<BestHorizontalRows> mBestHorzRow;
	};

}}}
