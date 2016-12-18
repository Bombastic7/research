#pragma once

#include <algorithm>
#include <random>
#include <vector>

#include "util/debug.hpp"
#include "util/permutation.hpp"

#include "domain/tiles/defs.hpp"
#include "domain/tiles/board_state.hpp"

namespace mjon661 { namespace tiles {
	
	enum struct MoveDir {
		Up, Down, Left, Right, Size
	};


	
	//Try move blank from pPos in direction pDir.
	//If able, out = position blank would be moved to, and returns true.
	template<int H, int W>
	inline bool tryMove(idx_t pPos, MoveDir pDir, idx_t& out) {
		if(pDir == MoveDir::Up && pPos >= W)
			out = pPos - W;
		
		else if(pDir == MoveDir::Down && pPos < W*(H-1))
			out = pPos + W;
		
		else if(pDir == MoveDir::Left && pPos % W != 0)
			out = pPos - 1;
		
		else if(pDir == MoveDir::Right && (pPos+1) % W != 0)
			out = pPos + 1;
		
		else
			return false;
		
		return true;
	}
	
	template<int H, int W>
	struct MoveLookup {

		using Table_t = std::array<std::array<idx_t, 5>, H*W>;
		
		MoveLookup() {
			
			for(int i=0; i<H*W; i++) {
				int n = 0;
				idx_t dest = -1;
				
				if(tryMove<H,W>(i, MoveDir::Up, dest))
					mMoves[i][++n] = dest;
				
				if(tryMove<H,W>(i, MoveDir::Down, dest))
					mMoves[i][++n] = dest;
				
				if(tryMove<H,W>(i, MoveDir::Left, dest))
					mMoves[i][++n] = dest;
				
				if(tryMove<H,W>(i, MoveDir::Right, dest))
					mMoves[i][++n] = dest;
				
				fast_assert(n >= 2 && n <= 4);
				mMoves[i][0] = n;
			}
		}
		
		
		std::array<idx_t, 5> const& get(idx_t pPos) const {
			return mMoves[pPos];
		}
		
		private:		
		
		Table_t mMoves;
	};
	
	
	

	template<int H, int W>
	bool getParity(BoardStateV<H,W> const& pBoard) {
		
		int nInv = permInversions_ignoreZero(pBoard);
		
		if(W % 2 == 1)
			return nInv % 2 == 0;

		int blankRow = pBoard.find(0) / W;
		
		return blankRow % 2 == nInv % 2;
	}

	template<int H, int W>
	bool isSolvable(BoardStateV<H,W> const& pInitBoard, BoardStateV<H,W> const& pGoalBoard) {
	
		return getParity<H,W>(pInitBoard) == getParity<H,W>(pGoalBoard);

	}
	
	
	template<int H, int W>
	struct ProblemGenerator {
		ProblemGenerator(BoardStateV<H,W> const& pGoalBoard) :
			mGoalBoard(pGoalBoard)
		{
			std::random_device rd;
			mRandGen.seed(rd());
		}
		
		BoardStateV<H,W> operator()() {
			do {
				std::shuffle(mInitBoard.begin(), mInitBoard.end(), mRandGen);
			} while(!isSolvable<H,W>(mInitBoard, mGoalBoard));

			return mInitBoard;
		}
		
		private:
		std::mt19937 mRandGen;
		BoardStateV<H,W> mInitBoard;
		BoardStateV<H,W> const& mGoalBoard;
	};

}}
