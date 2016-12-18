#pragma once

#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>
#include "domain/tiles/common.hpp"
#include "domain/tiles/defs.hpp"
#include "domain/tiles/mdist.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/math.hpp"
#include "util/json.hpp"


namespace mjon661 { namespace tiles {

	/*
	 * Common types for value-based states (i.e. storing tiles by face value, with position implied by 
	 * array index).
	 * 
	 * Max number of tiles supported is 20 if using Lehmer code, else is 16.
	 *
	 * Use_Lehmer: true - PackedState is lehmer code. Gives perfect hash, probably slower.
	 * 			   false - PackedState is array of 4-bit values representing tile value.
	 * 
	 * Use_MD_Heuristic: true - Methods::heuristicValue/distanceValue gives Manhattan distance.
	 * 					 false - all heuristic functions return 0.
	 * 
	 * Weighted_Cost: true - Move cost equals the position the blank tile is moving to.
	 * 				  false - Move cost is always 1.
	 */
	 

	template<unsigned H, unsigned W, bool Use_Lehmer, bool Use_MD_Heuristic, bool Weighted_Cost>
	class Tiles64_basic {
		
		public:
		
		static const unsigned Board_Size = H*W;
		
		static const size_t Hash_Range = Use_Lehmer ? factorial64(Board_Size) : std::numeric_limits<size_t>::max();
		using Cost = cost_t;
		using Operator = pos_t;
		
		
		using State = ValueState<Board_Size>;
		
		using PackedState = uint64_t;
		
		struct OperatorSet {
			using MoveDesc_t = typename BlankMoveDirectory<H,W>::MoveDesc_t;
		
			public:
			OperatorSet(MoveDesc_t const& pMoves) : mMoves(pMoves) {}
		
			unsigned size() const {
				return mMoves[0];
			}
		
			pos_t operator[](unsigned i) const {
				return mMoves[i+1];
			}
		
			private:
			MoveDesc_t const& mMoves;
		};
	
		struct Edge {
			Edge(cost_t pCost, State& pState, pos_t pParentOp) :
				mCost(pCost), mState(pState), mParentOp(pParentOp) {}
				
			State& state() {
				return mState;
			}
			
			cost_t cost() {
				return mCost;
			}
			
			pos_t parentOp() {
				return mParentOp;
			}
			
			cost_t mCost;
			State& mState;
			pos_t mParentOp;
		};
		
		
		struct Methods {
			const pos_t noOp;
			
			Methods(BoardValues<Board_Size> const& pInitBoard,
					BoardValues<Board_Size> const& pGoalBoard)
					:
					noOp		(Null_Pos),
					mInitBoard	(pInitBoard),
					mGoalBoard	(pGoalBoard),
					mOpDir		(),
					mMdist		(pGoalBoard)
			{}		
			
			State createState() const {
				State s;
				s = mInitBoard;
				s.initBlankPos();
				return s;
			}
				
			void packState(State const& pState, uint64_t& pPacked) const {
				if(Use_Lehmer)
					pPacked = toLehmerCode<Board_Size, uint64_t>(pState);
				else {
					pPacked = 0;
					for(unsigned i=0; i<Board_Size; i++)
						pPacked += pState[i] << i * 4u;
				}
			}
			
			void unpackState(State& pState, uint64_t const& pPacked) const {
				if(Use_Lehmer)
					fromLehmerCode<Board_Size, uint64_t>(pPacked, pState);
				else
					for(unsigned i=0; i<Board_Size; i++)
						pState[i] = (pPacked >> i*4) & 0xF;

				pState.initBlankPos();
			}
			
			Edge createEdge(State& pState, pos_t op) const {
				pos_t parentOp = pState.mBlankPos;
				pState.moveBlank(op);
				return Edge(moveCost(Weighted_Cost, op), pState, parentOp);
			}
			
			void destroyEdge(Edge& pEdge) const {
				pEdge.mState.moveBlank(pEdge.mParentOp);
			}
			
			OperatorSet createOperatorSet(State const& pState) const {
				return OperatorSet(mOpDir.at(pState.mBlankPos));
			}
			
			size_t hash(uint64_t pPacked) const {
				return pPacked;
			}
			
			cost_t heuristicValue(State const& pState) const {
				if(Use_MD_Heuristic)
					return mMdist(pState);
				else
					return 0;
			}
			
			cost_t distanceValue(State const& pState) const {
				return heuristicValue(pState);
			}
			
			bool checkGoal(State const& pState) const {
				return pState == mGoalBoard;
			}

			bool compare(State const& a, State const& b) const {
				return a == b.mTiles;
			}
			
			bool compare(uint64_t const& a, uint64_t const& b) const {
				return a == b;
			}
			
			void prettyPrint(State const& s, std::ostream& out) const {
				for(unsigned i=0; i<H; i++) {
					for(unsigned j=0; j<W; j++)
						out << s[i*W + j] << " ";
					out << "\n";
				}
			}
			
			void prettyPrint(Operator const& op, std::ostream &out) const {
				out << op << "\n";
			}
			
			Json report() {
				Json j;
				j["class"] = "Tiles64_basic<" + std::to_string(H) + ", " + std::to_string(W) + ">";
				j["State size"] = sizeof(State);
				j["Packed size"] = sizeof(PackedState);
				j["init state"] = mInitBoard;
				j["goal state"] = mGoalBoard;
				j["hash range"] = Hash_Range;
				return j;
			}
			
			private:

			BoardValues<Board_Size> const& mInitBoard;
			BoardValues<Board_Size> const& mGoalBoard;
			const BlankMoveDirectory<H,W> mOpDir;
			const MdistEval<H,W> mMdist; 
		};
	};
	
	template<unsigned H, unsigned W, unsigned N_Checked, bool Weighted_Cost>
	class Tiles64Relaxed_basic {
		public:
		static const unsigned Board_Size = H*W;
		using Cost = cost_t;
		using Operator = pos_t;
		
		static const size_t Hash_Range = pow<size_t>(Board_Size, N_Checked);

		using State = PositionState<N_Checked>;
		
		using PackedState = uint64_t;
		
		struct OperatorSet {
			using MoveDesc_t = typename BlankMoveDirectory<H,W>::MoveDesc_t;
		
			public:
			OperatorSet(MoveDesc_t const& pMoves) : mMoves(pMoves) {}
		
			unsigned size() const {
				return mMoves[0];
			}
		
			pos_t operator[](unsigned i) const {
				return mMoves[i+1];
			}
		
			private:
			MoveDesc_t const& mMoves;
		};
	
		struct Edge {
			Edge(cost_t pCost, State& pState, pos_t pParentOp) :
				mCost(pCost), mState(pState), mParentOp(pParentOp) {}
				
			State& state() {
				return mState;
			}
			
			cost_t cost() {
				return mCost;
			}
			
			pos_t parentOp() {
				return mParentOp;
			}
			
			cost_t mCost;
			State& mState;
			pos_t mParentOp;
		};
		
		struct Methods {
			const pos_t noOp;
			
			Methods(BoardValues<Board_Size> const& pInitBoard,
					BoardValues<Board_Size> const& pGoalBoard,
					CheckedTilesSpec<Board_Size, N_Checked> const& pCheckedTiles)
					:
					noOp(Null_Pos),
					mCheckedTiles(pCheckedTiles)
			{
				fast_assert(pCheckedTiles.front() == 0);
				
				for(unsigned i=0; i<N_Checked; i++) {
					mInitPositions[i] = pInitBoard.find(pCheckedTiles[i]);
					mGoalPositions[i] = pGoalBoard.find(pCheckedTiles[i]);
				}
			}
			/*
			State createState(BoardValues<Board_Size> const& v) const {
				State s;
				for(unsigned i=0; i<N_Checked; i++)
					s.mPositions[i] = s.mTiles.find(mCheckedTiles[i]);
			}
			
			State createState(BoardPositions<N_Checked+1> const& p) const {
				State s;
				for(unsigned i=0; i<N_Checked; i++)
					s.mPositions[i] = p[i];
				return s;
			}*/
			
			State createState() const {
				State s;
				s = mInitPositions;
				return s;
			}
				
			void packState(State const& pState, uint64_t& pPacked) const {
				pPacked = 0;
				uint64_t radix = Board_Size;
				
				pPacked += pState[0];
				
				for(unsigned i=1; i<N_Checked; i++) {
					pPacked += pState[i] * radix;
					radix *= Board_Size;
				}
			}
			
			void unpackState(State& pState, uint64_t const& pPacked) const {
				uint64_t pkd = pPacked;
				
				for(unsigned i=0; i<N_Checked; i++) {
					pState[i] = pkd % Board_Size;
					pkd /= Board_Size;
				}
			}
			
			Edge createEdge(State& pState, pos_t op) const {
				pos_t parentOp = pState[0];
				pState.moveBlank(op);
				return Edge(moveCost(Weighted_Cost, op), pState, parentOp);
			}
			
			void destroyEdge(Edge& pEdge) const {
				pEdge.mState.moveBlank(pEdge.mParentOp);
			}
			
			OperatorSet createOperatorSet(State const& pState) const {
				return OperatorSet(mOpDir.at(pState[0]));
			}
			
			size_t hash(uint64_t pPacked) const {
				return pPacked;
			}
			
			cost_t heuristicValue(State const& pState) const {
				return 0;
			}
			
			bool checkGoal(State const& pState) const {
				return pState == mGoalPositions;
			}

			bool compare(State const& a, State const& b) const {
				return a == b;
			}
			
			bool compare(uint64_t const& a, uint64_t const& b) const {
				return a == b;
			}
			
			void prettyPrint(State const& s, std::ostream& out) const {
				BoardValues<Board_Size> board;
				
				for(unsigned i=0; i<Board_Size; i++)
					board[i] = Null_Tile;
				
				for(unsigned i=0; i<N_Checked; i++)
					board[s[i]] = mCheckedTiles[i];
				
				for(unsigned i=0; i<H; i++) {
					for(unsigned j=0; j<W; j++) {
						if(board[i*W + j] != Null_Pos)
							out << board[i*W + j] << " ";
						else
							out << "* ";
					}
					out << "\n";
				}
			}
			
			void prettyPrint(Operator const& op, std::ostream &out) const {
				out << op << "\n";
			}
			
			Json report() {
				Json j;
				j["class"] = "Tiles64relaxed_basic<" + std::to_string(H) + ", " + std::to_string(W) + ">";
				j["State size"] = sizeof(State);
				j["Packed size"] = sizeof(PackedState);
				j["init state"] = mInitPositions;
				j["goal state"] = mGoalPositions;
				j["checked tiles"] = mCheckedTiles;
				j["hash range"] = Hash_Range;
				return j;
			}
			
			private:
			BoardPositions<N_Checked> mInitPositions;
			BoardPositions<N_Checked> mGoalPositions;
			CheckedTilesSpec<Board_Size, N_Checked> mCheckedTiles;
			const BlankMoveDirectory<H,W> mOpDir;
		};
	};
}}
