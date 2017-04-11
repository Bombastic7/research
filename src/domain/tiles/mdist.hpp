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
		
		Manhattan(BoardState<H, W> const& pGoalState) :
			mGoalState(pGoalState)
		{
			
			for(idx_t src=0; src<Board_Size; src++) {
				for(tile_t t=0; t<Board_Size; t++) {
					int src_row = src / W, src_col = src % W;
					int goal_row = mGoalState.find(t) / W, goal_col = mGoalState.find(t) % W;
					
					mMinDistance[t][src] = std::abs(src_row - goal_row) + std::abs(src_col - goal_col);
					
					if(Use_Weight) {
						unsigned vertcost = 0;
						if(src_row < goal_row) {
							for(int y=src_row+1; y<=goal_row; y++)
								vertcost += 1 + y;
						}
						else if(src_row > goal_row) {
							for(int y=src_row-1; y>=goal_row; y--)
								vertcost += 1 + y;
						}
						
						unsigned horzcost = mathutil::abs(src_col, goal_col);
						if(src_row < goal_row)
							horzcost *= (1 + src_row);
						else
							horzcost *= (1 + goal_row);
						
						mMinCost[t][src] = vertcost + horzcost;
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
		
		
		void evalDump(BoardState<H, W> const& pBoard, std::ostream& out) const {
			for(int h=0; h<H; h++) {
				for(int w=0; w<W; w++) {
					int i = h*W + w;
					if(pBoard[i] == 0)
						out << "0 ";
					else
						out << mMinCost[pBoard[i]][i] << " ";
				}
				out << "\n";
			}
			out << "\n";
			
			for(tile_t t = 1; t<H*W; t++) {
				out << t << ": " << mMinCost[t][pBoard.find(t)] << "\n";
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
				out << "\n\n";
				
				for(int h=0; h<H; h++) {
					for(int w=0; w<W; w++)
						out << mMinCost[t][h*W+w] << " ";
				
					out << "\n";
				}
				out << "\n\n\n";
			}
		}
		
		//MD for state
		void eval(BoardState<H, W> const& pBoard, int& out_h, int& out_d) const {
			
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
		
		//t = tile
		//s = tile's position
		//g = cost from t's goal position to s.
		
		//If this is the cheapest we've moved t to s, update mMinCost[t][s].
		//For all adjacent positions, recursively call functions again.
		//~ void weightedCostRec(tile_t t, idx_t s, cost_t g) {
			//~ if(mMinCost[t][s] <= g)
				//~ return;
			
			//~ mMinCost[t][s] = g;
			
			//~ //If Use_Weight = true, edge cost is 1 + row of blank pos.
			//~ if(s >= W)
				//~ weightedCostRec(t, s-W, g + 1 + (s-W)/W);
			//~ if(s < (H-1)*W)
				//~ weightedCostRec(t, s+W, g + 1 + (s+W)/W);
			//~ if(s % W != 0)
				//~ weightedCostRec(t, s-1, g + 1 + (s-1)/W);
			//~ if((s+1) % W != 0)
				//~ weightedCostRec(t, s+1, g + 1 + (s+1)/W);
		//~ }
		
		std::array<std::array<cost_t, Board_Size>, Board_Size> mMinCost, mMinDistance;
		std::array<std::array<std::array<int, Board_Size>, Board_Size>, Board_Size> mCostInc, mDistInc;
		const BoardState<H, W> mGoalState;
	};
}}
