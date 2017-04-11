#pragma once

#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "util/debug.hpp"
#include "util/math.hpp"
#include "util/json.hpp"
#include "util/permutation.hpp"
#include "search/solution.hpp"


#include "domain/tiles/common.hpp"
#include "domain/tiles/defs.hpp"
#include "domain/tiles/mdist.hpp"
#include "domain/tiles/mdist.hpp"
#include "domain/tiles/board_state.hpp"



namespace mjon661 { namespace tiles {

	template<unsigned H, unsigned W, bool Use_Hr, bool Use_Weight>
	struct CompleteTilesDomain;

	
	template<unsigned H, unsigned W, bool Use_Weight>
	struct CompleteTilesDomain<H,W,true,Use_Weight> {

		using Cost = cost_t;
		
		static const bool Is_Perfect_Hash = H*W <= 11; //11! ~= 40e6


		struct State : public BoardState<H,W> {
			State() = default;
			
			State(BoardState<H,W> const& o) :
				BoardState<H,W>(o)
			{}
			
			bool operator==(State const& o) {
				bool b = *this == o;
				if(b) slow_assert(h == o.h && d == o.d);
				return b;
			}
			
			cost_t h, d;
		};

		using PackedState = typename State::packed_t;
		
		
		
		struct AdjEdgeIterator {
			
			enum {
				Op_Start, Op_Up, Op_Down, Op_Left, Op_Right, Op_Null
			};
			
			AdjEdgeIterator(State& pState, Manhattan<H,W,Use_Weight> const& pManhat) :
				mAdjState(pState),
				mManhat(pManhat),
				mLastOp(Op_Start),
				mOrigBlankPos(pState.getBlankPos()),
				mOrig_h(pState.h),
				mOrig_d(pState.d),
				mTest_origState(pState)
			{
				next();
			}
			
			~AdjEdgeIterator() {
				if(!finished()) {
					mAdjState.moveBlank(mOrigBlankPos);
					mAdjState.h = mOrig_h;
					mAdjState.d = mOrig_d;
				}
				slow_assert(mTest_origState == mAdjState);
			}
			
			bool finished() const {
				return mLastOp == Op_Null;
			}
			
			State& state() const {
				slow_assert(!finished());
				return mAdjState;
			}
			
			Cost cost() const {
				slow_assert(!finished());
				return Use_Weight ? 1 + mOrigBlankPos / W : 1;
			}
			
			void next() {
				slow_assert(!finished());
				
				if(mLastOp != Op_Start) {
					mAdjState.moveBlank(mOrigBlankPos);
					mAdjState.h = mOrig_h;
					mAdjState.d = mOrig_d;
				}
				
				mLastOp++;
				
				for(int op = mLastOp; op<Op_Null; op++) {
					if(tryOp(op))
						break;
					mLastOp++;
				}
			}

			
			bool tryOp(int op) {
				unsigned newBlankPos = -1;
				
				if(op == Op_Up && mOrigBlankPos >= W) 					newBlankPos = mOrigBlankPos - W;
				else if(op == Op_Down && mOrigBlankPos < (H-1)*W) 		newBlankPos	= mOrigBlankPos + W;
				else if(op == Op_Left && mOrigBlankPos % W != 0)		newBlankPos	= mOrigBlankPos - 1;
				else if(op == Op_Right && (mOrigBlankPos+1) % W != 0)	newBlankPos = mOrigBlankPos + 1;
				else 													return false;

				mAdjState.moveBlank(newBlankPos);
				
				//Cost dh, dd;
				//mManhat.increment(newBlankPos, mOrigBlankPos, mAdjState[newBlankPos], dh, dd);
				mManhat.eval(mAdjState, mAdjState.h, mAdjState.d);
				
				
				//mAdjState.h += dh;
				//mAdjState.d += dd;
				slow_assert(mAdjState.valid());
				
				return true;
			}
			
			
			State& mAdjState;
			Manhattan<H,W,Use_Weight> const& mManhat;
			int mLastOp;
			const unsigned mOrigBlankPos;
			const Cost mOrig_h, mOrig_d;
			const State mTest_origState;
		};
		
		

		CompleteTilesDomain(BoardState<H,W> const& pGoalState) :
			mManhattan(pGoalState),
			mGoalState(pGoalState)
		{
			mManhattan.eval(mGoalState, mGoalState.h, mGoalState.d);
		}

		void packState(State const& pState, PackedState& pkd) const {
			pkd = pState.getPacked();
		}
		
		void unpackState(State& pState, PackedState const& pkd) const {
			pState.fromPacked(pkd);
			mManhattan.eval(pState, pState.h, pState.d);
		}
		
		AdjEdgeIterator getAdjEdges(State& pState) const {
			return AdjEdgeIterator(pState, mManhattan);
		}
		
		void prettyPrintState(State const& pState, std::ostream& out) const {
			pState.prettyPrint(out);
			out << "(h, d): (" << pState.h << ", " << pState.d << ")\n";
		}

		
		size_t hash(PackedState const& pkd) const {
				return pkd;
		}
		
		bool checkGoal(State const& pState) const {
			return pState == mGoalState;
		}

		Cost costHeuristic(State const& pState) const {
			return pState.h;
		}
		
		Cost distanceHeuristic(State const& pState) const {
			return pState.d;
		}
		
		std::pair<Cost,Cost> pairHeuristics(State const& pState) const {
			return {pState.h, pState.d};
		}

		bool compare(State const& a, State const& b) const {
			return a == b;
		}

		bool compare(PackedState const& a, PackedState const& b) const {
			return a == b;
		}
		
		void initialiseState(State& s) const {
			mManhattan.eval(s, s.h, s.d);
			s.initBlankPos();
		}

		//private:
		const Manhattan<H,W,Use_Weight> mManhattan;
		State mGoalState;
	};
	
	
	
	
	
	
	template<unsigned H, unsigned W, bool Use_Weight>
	struct CompleteTilesDomain<H,W,false,Use_Weight> {
		
		using Cost = cost_t;		
		static const bool Is_Perfect_Hash = H*W <= 11; //11! ~= 40e6

		using State = BoardState<H,W>;
		using PackedState = typename State::packed_t;
		
		
		struct AdjEdgeIterator {
			enum {
				Op_Start, Op_Up, Op_Down, Op_Left, Op_Right, Op_Null
			};
			
			AdjEdgeIterator(State& pState) :
				mAdjState(pState),
				mLastOp(Op_Start),
				mOrigBlankPos(pState.getBlankPos()),
				mTest_origState(pState)
			{
				next();
			}
			
			~AdjEdgeIterator() {
				if(!finished()) {
					mAdjState.moveBlank(mOrigBlankPos);
				}
				slow_assert(mTest_origState == mAdjState);
			}
			
			bool finished() const {
				return mLastOp == Op_Null;
			}
			
			State& state() const {
				slow_assert(!finished());
				return mAdjState;
			}
			
			Cost cost() const {
				slow_assert(!finished());
				return Use_Weight ? 1 + mOrigBlankPos / W : 1;
			}
			
			void next() {
				slow_assert(!finished());
				mAdjState.moveBlank(mOrigBlankPos);
				
				mLastOp++;
				
				for(int op = mLastOp; op<Op_Null; op++) {
					if(tryOp(op))
						break;
					mLastOp++;
				}
			}
			
			bool tryOp(int op) {
				unsigned newBlankPos = -1;
				
				if(op == Op_Up && mOrigBlankPos >= W) 					newBlankPos = mOrigBlankPos - W;
				else if(op == Op_Down && mOrigBlankPos < (H-1)*W) 		newBlankPos	= mOrigBlankPos + W;
				else if(op == Op_Left && mOrigBlankPos % W != 0)		newBlankPos	= mOrigBlankPos - 1;
				else if(op == Op_Right && (mOrigBlankPos+1) % W != 0)	newBlankPos = mOrigBlankPos + 1;
				else 													return false;
				
				mAdjState.moveBlank(newBlankPos);
				return true;
			}

			State& mAdjState;
			int mLastOp;
			const idx_t mOrigBlankPos;
			const State mTest_origState;
		};
		
		
		CompleteTilesDomain(BoardState<H,W> const& pGoalState) :
			mGoalState(pGoalState)
		{}
		
		void packState(State const& pState, PackedState& pkd) const {
			pkd = pState.getPacked();
		}
		
		void unpackState(State& pState, PackedState const& pkd) const {
			pState.fromPacked(pkd);
		}
		
		AdjEdgeIterator getAdjEdges(State& pState) const {
			return AdjEdgeIterator(pState);
		}		
		
		void prettyPrintState(State const& pState, std::ostream& out) const {
			pState.prettyPrint(out);
		}
		
		size_t hash(PackedState const& pkd) const {
				return pkd;
		}
		
		bool checkGoal(State const& pState) const {
			return pState == mGoalState;
		}

		Cost costHeuristic(State const& pState) const {
			return 0;
		}
		
		Cost distanceHeuristic(State const& pState) const {
			return 0;
		}
		
		bool compare(State const& a, State const& b) const {
			return a == b;
		}

		bool compare(PackedState const& a, PackedState const& b) const {
			return a == b;
		}
		
		std::pair<Cost, Cost> pairHeuristics(State const& pState) const {
			return std::pair<Cost, Cost>(0, 0);
		}
		
		void initialiseState(State& s) const {
			s.initBlankPos();
		}
		
		private:
		State mGoalState;
	};
	

	
	template<unsigned H, unsigned W, unsigned Sz, bool Use_Weight>
	struct SubsetTilesDomain {
		using Cost = cost_t;
		
		using State = SubsetBoardState<H, W, Sz>;
		using PackedState = typename State::packed_t;
		
		static const bool Is_Perfect_Hash = H*W <= 11; //11! ~= 40e6
		
		struct AdjEdgeIterator {
			enum : unsigned {
				Op_Start, Op_Up, Op_Down, Op_Left, Op_Right, Op_Null
			};
			
			AdjEdgeIterator(State& pState) :
				mAdjState(pState),
				mLastOp(Op_Start),
				mOrigBlankPos(pState.getBlankPos()),
				mTest_origState(pState)
			{
				next();
			}
			
			~AdjEdgeIterator() {
				if(!finished()) {
					mAdjState.moveBlank(mOrigBlankPos);
				}
				slow_assert(mTest_origState == mAdjState);
			}
			
			bool finished() const {
				return mLastOp == Op_Null;
			}
			
			State& state() const {
				slow_assert(!finished());
				return mAdjState;
			}
			
			Cost cost() const {
				slow_assert(!finished());
				return Use_Weight ? 1 + mOrigBlankPos / W : 1;
			}
			
			void next() {
				slow_assert(!finished());
				mAdjState.moveBlank(mOrigBlankPos);
				
				mLastOp++;
				
				for(unsigned op = mLastOp; op<Op_Null; op++) {
					if(tryOp(op))
						break;
					mLastOp++;
				}
			}
			
			bool tryOp(unsigned op) {
				unsigned newBlankPos = -1;
				if(op == Op_Up && mOrigBlankPos >= W) 					newBlankPos = mOrigBlankPos - W;
				else if(op == Op_Down && mOrigBlankPos < (H-1)*W) 		newBlankPos	= mOrigBlankPos + W;
				else if(op == Op_Left && mOrigBlankPos % W != 0)		newBlankPos	= mOrigBlankPos - 1;
				else if(op == Op_Right && (mOrigBlankPos+1) % W != 0)	newBlankPos = mOrigBlankPos + 1;
				else 													return false;
				
				mAdjState.moveBlank(newBlankPos);
				return true;
			}

			State& mAdjState;
			int mLastOp;
			const unsigned mOrigBlankPos;
			const State mTest_origState;
		};
		
		
		SubsetTilesDomain(TilesAbtSpec<H*W> const& pAbtSpec, BoardState<H,W> const& pBaseGoalState) :
			mAbtSpec(pAbtSpec),
			mGoalState(pBaseGoalState, mAbtSpec)
		{}
		
		template<typename BS>
		State abstractParentState(BS const& bs) const {
			return State(bs, mAbtSpec);
		}

		void unpackState(State& pState, PackedState const& pkd) const {
			pState.fromPacked(pkd, mAbtSpec);
		}

		void packState(State const& pState, PackedState& pkd) const {
			pkd = pState.getPacked(mAbtSpec);
		}
		
		void prettyPrintState(State const& pState, std::ostream& out) const {
			pState.prettyPrint(out);
		}
		
		size_t hash(PackedState const& pkd) const {
				return pkd;
		}
	
		AdjEdgeIterator getAdjEdges(State& pState) const {
			return AdjEdgeIterator(pState);
		}
		
		bool checkGoal(State const& pState) const {
			return pState == mGoalState;
		}

		bool compare(State const& a, State const& b) const {
			return a == b;
		}

		bool compare(PackedState const& a, PackedState const& b) const {
			return a == b;
		}
		
		private:

		TilesAbtSpec<H*W> const& mAbtSpec;
		const State mGoalState;
	};

}}
