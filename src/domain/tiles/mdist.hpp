#pragma once

#include <cstdlib>
#include "domain/tiles/defs.hpp"
#include "util/math.hpp"

namespace mjon661 { namespace tiles {

	template<int H, int W>
	struct Manhattan {
		static_assert(H > 0 && W > 0, "");
		
		static const unsigned Board_Size = H*W;
		
		Manhattan(BoardStateV<H, W> const& pGoalState) :
			mGoalState(pGoalState)
		{
			for(unsigned src=0; src<Board_Size; src++) {
				for(unsigned t=0; t<Board_Size; t++) {
					
					int src_row = src / W, src_col = src % W;
					int goal_row = mGoalState.find(t) / W, goal_col = mGoalState.find(t) % W;
					
					mDistance[src][t] = std::abs(src_row - goal_row) + std::abs(src_col - goal_col);
				}
			}
			
			
			MoveLookup<H, W> moveLookup;
			
			for(tile_t t=0; t<H*W; t++) {
				
				for(idx_t pos=0; pos<H*W; pos++) {
					
					std::array<idx_t, 5> mvs = moveLookup.get(pos);
					
					for(int n=0; n<mvs[0]; n++) {
						
						idx_t dest = mvs[n+1];
						
						mIncrement[t][pos][dest] = mDistance[dest][t] - mDistance[pos][t];
						
						
						fast_assert(std::abs(mIncrement[t][pos][dest]) <= 1);
					}
				}
				
			}
		}
		
		//MD for state
		int eval(BoardStateV<H, W> const& pBoard) const {
			cost_t h = 0;
			
			for(unsigned i=0; i<Board_Size; i++) {
				if(pBoard[i] == 0)
					continue;
				h += mDistance[i][pBoard[i]];
			}
			
			return h;
		}
		
		//MD for one tile from pSrc
		int distance(idx_t pSrc, tile_t pTile) const {
			return mDistance[pSrc][pTile];
		}
		
		//Change in MD when pTile (not the blank) moves from pSrc to pDest in a move operation.
		//i.e. state.h += increment(op, blankpos, state[op]), where op is the index the blank is moving to.
		int increment(idx_t pSrc, idx_t pDest, tile_t pTile) const {
			return mIncrement[pTile][pSrc][pDest];
		}
		
		
		private:
		std::array<std::array<cost_t, Board_Size>, Board_Size> mDistance;
		std::array<std::array<std::array<int, Board_Size>, Board_Size>, Board_Size> mIncrement;
		const BoardStateV<H, W> mGoalState;
	};
}}
