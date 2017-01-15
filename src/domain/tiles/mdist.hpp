#pragma once

#include <limits>
#include <cstdlib>
#include <iostream>
#include "domain/tiles/defs.hpp"
#include "domain/tiles/board_state.hpp"
#include "domain/tiles/common.hpp"
#include "util/math.hpp"
#include "util/debug.hpp"


namespace mjon661 { namespace tiles {

	template<int H, int W, bool Use_Weight>
	struct Manhattan {
		static_assert(H > 0 && W > 0, "");
		
		static const int Board_Size = H*W;
		
		Manhattan(BoardStateV<H, W> const& pGoalState) :
			mGoalState(pGoalState),
			mMoveLookup()
		{
			std::vector<idx_t> ancVec;
			
			for(idx_t src=0; src<Board_Size; src++) {
				for(tile_t t=0; t<Board_Size; t++) {
					int src_row = src / W, src_col = src % W;
					int goal_row = mGoalState.find(t) / W, goal_col = mGoalState.find(t) % W;
					
					mMinDistance[t][src] = std::abs(src_row - goal_row) + std::abs(src_col - goal_col);
					
					if(Use_Weight) {
						mMinCost[t].fill(std::numeric_limits<cost_t>::max());
						weightedCostRec(t, mGoalState.find(t), 0);
					}
					else
						mMinCost[t][src] = mMinDistance[t][src];
				}
			}
			
			for(tile_t t=0; t<Board_Size; t++) {
				for(idx_t src=0; src<Board_Size; src++) {
					for(idx_t dst=0; dst<Board_Size; dst++) {
						mCostInc[t][src][dst] = mMinCost[t][dst] - mMinCost[t][src];
						mDistInc[t][src][dst] = mMinDistance[t][dst] - mMinDistance[t][src];
					}
				}
			}
		}
		
		void dump(std::ostream& out) const {
			for(tile_t t=0; t<Board_Size; t++) {
				out << t << ":\n";
				
				for(int h=0; h<H; h++) {
					for(int w=0; w<W; w++)
						out << mMinDistance[t][h*W+w] << " ";
				
					out << "\n";
				}
				out << "\n";
				
				for(int h=0; h<H; h++) {
					for(int w=0; w<W; w++)
						out << mMinCost[t][h*W+w] << " ";
				
					out << "\n";
				}
			}
		}
		
		//MD for state
		void eval(BoardStateV<H, W> const& pBoard, int& out_h, int& out_d) const {
			
			out_h = out_d = 0;
			
			for(unsigned i=0; i<Board_Size; i++) {
				if(pBoard[i] == 0)
					continue;
				
				slow_assert(pBoard[i] >= 0 && pBoard[i] < H*W, "%d", pBoard[i]);

				out_h += mMinCost[pBoard[i]][i];
				out_d += mMinDistance[pBoard[i]][i];
			}
		}
		
		//Change in h/d when pTile moves from pSrc to pDest.
		void increment(idx_t pSrc, idx_t pDest, tile_t pTile, int& out_dh, int& out_dd) const {
			slow_assert(pTile != 0);
			out_dh = mCostInc[pTile][pSrc][pDest];
			out_dd = mDistInc[pTile][pSrc][pDest];
		}
		
		
		private:
		
		
		void weightedCostRec(tile_t t, idx_t s, cost_t g) {
			
			if(mMinCost[t][s] <= g)
				return;
			
			mMinCost[t][s] = g;
			
			std::array<idx_t, 5> const& mvs = mMoveLookup.get(s);
			
			for(int n=0; n<mvs[0]; n++)
				weightedCostRec(t, mvs[n+1], g+s);
		}
		
		std::array<std::array<cost_t, Board_Size>, Board_Size> mMinCost, mMinDistance;
		std::array<std::array<std::array<int, Board_Size>, Board_Size>, Board_Size> mCostInc, mDistInc;
		const BoardStateV<H, W> mGoalState;
		const MoveLookup<H, W> mMoveLookup;
	};
}}
