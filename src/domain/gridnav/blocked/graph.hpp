#pragma once

#include <array>
#include <iostream>
#include <vector>
#include <cmath>



namespace mjon661 { namespace gridnav { namespace blocked {


	enum struct Cell_t {
		Open, Blocked, Null
	};
	
	enum struct CardDir_t {
		N, S, E, W, NW, NE, SW, SE
	};
	
	const unsigned Null_Idx = (unsigned)-1;
	
	
	template<typename = void>
	struct FourWayFuncs {
		using Cost_t = int;
		using AdjacentCells = std::array<unsigned, 4>; 
	
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
		using AdjacentCells = std::array<unsigned, 8>; 
		
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
		
		Cost_t getOpCost(unsigned idx, unsigned op) {
			slow_assert(idx < mSize);
			slow_assert(op < Max_Adj);
			return Use_LC ? idx/mWidth * BaseFuncs::getMoveCost(op) : BaseFuncs::getMoveCost(op);
		}
		
		
		unsigned const mHeight, mWidth, mSize;
		
		private:
		std::vector<AdjacentCells> mAdjList;
	};



	template<bool Use_LC, unsigned H_, unsigned W_>
	struct AbstractCellMap4 {
		
		using Cost_t = BaseFuncs::Cost_t;
		using AdjacentCells = BaseFuncs::AdjacentCells;
		
		template<typename BaseMap_t>
		void prep4(BaseMap_t const& pBaseMap) {

			unsigned selfHeight = pBaseMap.mHeight / H_;
			unsigned selfWidth = pBaseMap.mWidth / W_;
			
			if(selfHeight % H_ != 0) selfHeight++;
			if(selfWidth % W_ != 0) selfWidth++;
			
			for(unsigned y=0; y<selfHeight; y++) {
				for(unsigned x=0; x<selfWidth; x++) {
					
					unsigned startidx = mBaseWidth*y+x;
					unsigned endidx = mBaseWidth*y + (x+W_ >= mBaseWidth ? mBaseWidth : x+W_);
					
					for(unsigned i=startidx; i<endidx; i++) {
						if(mBaseMap[i] == Null_Idx)
							continue;
						
						if(mBaseMap.getAdjCells(i)[0] != Null_Idx)
							mSelfMap[mWidth * y + x][0] = mWidth * (y-1) + x;
					}
					
					
					startidx = mBaseWidth*(y+H_-1)+x;
					endidx = mBaseWidth*(y+H_-1) + (x+W_ >= mBaseWidth ? mBaseWidth : x+W_);
					
					for(unsigned i=startidx; i<endidx; i++) {
						if(mBaseMap[i] == Null_Idx)
							continue;
						
						if(mBaseMap.getAdjCells(i)[1] != Null_Idx)
							mSelfMap[mWidth * y + x][1] = mWidth * (y+1) + x;
					}
					
					
					startidx = mBaseWidth*y+x;
					endidx = mBaseWidth * (y+H_-1 >= mBaseHeight ? mBaseHeight-1 : y+H_-1) + x;
					
					for(unsigned i=startidx; i<=endidx; i+=mBaseWidth) {
						if(mBaseMap[i] == Null_Idx)
							continue;
						
						if(mBaseMap.getAdjCells(i)[2] != Null_Idx)
							mSelfMap[mWidth * y + x][2] = mWidth * y + x - 1;
					}
					
					startidx = mBaseWidth*y + (x+W_-1 >= mBaseWidth ? mBaseWidth-1 : x+W_-1);
					endidx = startidx + mBaseWidth * (y+H_-1 >= mBaseHeight ? mBaseHeight-1 : y+H_-1);
					
					for(unsigned i=startidx; i<=endidx; i+=mBaseWidth) {
						if(mBaseMap[i] == Null_Idx)
							continue;
						
						if(mBaseMap.getAdjCells(i)[3] != Null_Idx)
							mSelfMap[mWidth * y + x][3] = mWidth * y + x + 1;
					}
				}
			}
		}

		void prep8(CellMap<EightWayFuncs, Use_LC> const& pBaseMap) {
			
			prep4(pBaseMap);
			
			for(unsigned y=0; y<mHeight; y++) {
				for(unsigned x=0; x<mWidth; x++) {
					
					unsigned tl = mBaseWidth*y+x;
					unsigned tr = mBaseWidth*y+(x+W_-1 >= mBaseWidth ? mBaseWidth-1 : x+W_-1);
					unsigned bl = mBaseWidth*(y+H_-1 >= mBaseHeight ? mBaseHeight-1 : y+H_-1) + x;
					unsigned br = mBaseWidth*(y+H_-1 >= mBaseHeight ? mBaseHeight-1 : y+H_-1) + (x+W_-1 >= mBaseWidth ? mBaseWidth-1 : x+W_-1);
					
					if(pBaseMap.getAdjCells(tl)[4] != Null_Idx)
						mSelfMap[mWidth * y + x][4] = mWidth * (y-1) + x - 1;

					if(pBaseMap.getAdjCells(tr)[5] != Null_Idx)
						mSelfMap[mWidth * y + x][5] = mWidth * (y-1) + x + 1;
					
					if(pBaseMap.getAdjCells(br)[6] != Null_Idx)
						mSelfMap[mWidth * y + x][6] = mWidth * (y+1) + x - 1;
						
					if(pBaseMap.getAdjCells(bl)[7] != Null_Idx)
						mSelfMap[mWidth * y + x][7] = mWidth * (y+1) + x + 1;	
				}
			}
		}
	
		
		unsigned const mHeight, mWidth, mSize;
		unsigned const mBaseHeight, mBaseWidth, mBaseSize;
		CellMap<FourWayFuncs, Use_LC> const& mBaseMap;
		std::vector<AdjacentCells> mAdjList;
	};

}}}
