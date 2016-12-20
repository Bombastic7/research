#pragma once

#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "util/debug.hpp"
#include "util/math.hpp"
#include "util/json.hpp"
#include "util/permutation.hpp"
#include "search/solution.hpp"


#include "domain/tiles/common.hpp"
#include "domain/tiles/defs.hpp"
#include "domain/tiles/mdist.hpp"
#include "domain/tiles/index_map.hpp"
#include "domain/tiles/mdist.hpp"
#include "domain/tiles/board_state.hpp"



namespace mjon661 { namespace tiles {

	
	template<unsigned H, unsigned W, bool Use_Weight, bool Use_H>
	struct CompleteTilesBase {
		
		static_assert(!(Use_Weight && Use_H), "");
		

		template<bool, typename = void>
		struct StateImpl : public BoardStateV<H, W> {
			cost_t getHval() const {return 0;}
			void setHval(cost_t) {}
			
			StateImpl() = default;
			StateImpl(BoardStateV<H,W> const& o) : BoardStateV<H,W>(o) {}
			StateImpl(BoardStateV<H,W> const& o, cost_t) : BoardStateV<H,W>(o) {}
		};
		
		template<typename Ign>
		struct StateImpl<true, Ign> : public BoardStateV<H, W> {
			cost_t getHval() const {return hval;}
			void setHval(cost_t h) {hval = h;}
			StateImpl() = default;
			StateImpl(BoardStateV<H,W> const& o) : BoardStateV<H,W>(o) {}
			StateImpl(BoardStateV<H,W> const& o, cost_t pH) : BoardStateV<H,W>(o) {setHval(pH);}
			
			bool operator==(StateImpl<true, Ign> const& o) {
				return BoardStateV<H, W> ::operator==(o);
			}
			
			cost_t hval;
		};

		
		using state_t = StateImpl<Use_H>;

		using packed_t = typename state_t::packed_t;
		
		static const size_t Max_Packed = state_t::Max_Packed;
		
		
		CompleteTilesBase(BoardStateV<H,W> const& pInitState, BoardStateV<H,W> const& pGoalState) :
			mManhattan(pGoalState),
			mInitState(pInitState, mManhattan.eval(pInitState)),
			mGoalState(pGoalState, mManhattan.eval(mGoalState))
		{
			fast_assert(mManhattan.eval(mGoalState) == 0);
		}
		
		state_t doCreateState() const {
			return mInitState;
		}
		
		void doUnpackState(state_t& pState, packed_t const& pkd) const {
			pState.fromPacked(pkd);
			pState.setHval(mManhattan.eval(pState));
		}

		cost_t getMoveCost(idx_t op) const {
			return Use_Weight ? op / W : 1;
		}
		
		void prettyPrint(state_t const& pState, std::ostream& out) const {
			pState.prettyPrint(out);
			out << "H: " << pState.getHval() << "\n";
		}
		
		size_t doHash(packed_t const& pkd) const {
				return pkd;
		}
		
		void performMove(state_t& pState, idx_t op) const {
			if(Use_H)
				pState.setHval(pState.getHval() + mManhattan.increment(op, pState.getBlankPos(), pState[op]));
			
			pState.moveBlank(op);
		}

		
		cost_t getVal_h(state_t const& pState) const {
			if(!Use_H)
				return 0;
			
			return pState.getHval();
		}
		
		cost_t getVal_d(state_t const& pState) const {
			return getVal_h(pState);
		}
		
		bool doCheckGoal(state_t const& pState) const {
			return pState == mGoalState;
		}
		

		private:
		const Manhattan<H,W> mManhattan;
		const state_t mInitState, mGoalState;
	};
	
	
	
	
	template<unsigned H, unsigned W, unsigned Sz, bool Use_Weight>
	struct SubsetTilesBase {
		
		using state_t = BoardStateP<H, W, Sz>;

		using packed_t = typename state_t::packed_t;
		
		static const size_t Max_Packed = state_t::Max_Packed;

		
		SubsetTilesBase(BoardStateV<H,W> const& pGoalStateV, IndexMap<H*W, Sz> const& pMap) :
			mGoalState(prepGoalState(pGoalStateV, pMap)),
			mMap(pMap)
		{}
		
		state_t doCreateState() const {
			gen_assert(false);
			return state_t();
		}
		
		void doUnpackState(state_t& pState, packed_t const& pkd) const {
			pState.fromPacked(pkd);
		}

		cost_t getMoveCost(idx_t op) const {
			return Use_Weight ? op / W : 1;
		}
		
		void prettyPrint(state_t const& pState, std::ostream& out) const {
			pState.prettyPrint(mMap, out);
		}
		
		size_t doHash(packed_t const& pkd) const {
				return pkd;
		}
		
		void performMove(state_t& pState, idx_t op) const {
			pState.moveBlank(op);
		}

		
		cost_t getVal_h(state_t const& pState, idx_t op) const {
			return 0;
		}
		
		cost_t getVal_d(state_t const& pState, idx_t op) const {			
			return 0;
		}
		
		bool doCheckGoal(state_t const& pState) const {
			return pState == mGoalState;
		}
		
		
		private:

		state_t prepGoalState(BoardStateV<H,W> const& pGoalStateV, IndexMap<H*W, Sz> const& pMap) {
			
			BoardStateP<H,W,Sz> goalState;
			
			for(unsigned i=0; i<Sz; i++)
				goalState[i] = pGoalStateV.find(pMap.tileAt(i));
			
			return goalState;
		}


		const state_t mGoalState;
		const IndexMap<H*W, Sz> mMap;
	};
	
	
	

	
	template<unsigned H, unsigned W, unsigned Sz, bool Use_Weight, bool Use_H, bool Is_Subset>
	struct TilesDomainBase : public CompleteTilesBase<H, W, Use_Weight, Use_H> {
	
		TilesDomainBase(BoardStateV<H,W> const& pInitState, 
						BoardStateV<H,W> const& pGoalState,
						IndexMap<H*W, Sz> const& pMap) :
		
			CompleteTilesBase<H, W, Use_Weight, Use_H>(pInitState, pGoalState)
		{}
	
	};
	
	
	template<unsigned H, unsigned W, unsigned Sz, bool Use_Weight, bool Use_H>
	struct TilesDomainBase<H, W, Sz, Use_Weight, Use_H, true> : public SubsetTilesBase<H, W, Sz, Use_Weight, Use_H> {

		TilesDomainBase(BoardStateV<H,W> const& pInitState, 
						BoardStateV<H,W> const& pGoalState,
						IndexMap<H*W, Sz> const& pMap) :
						
			SubsetTilesBase<H, W, Sz, Use_Weight, Use_H>(pGoalState, pMap)
		{}
	
	};
	






	template<unsigned H, unsigned W, unsigned Sz, bool Use_Weight, bool Use_H>
	class TilesDomain : public TilesDomainBase<H, W, Sz, Use_Weight, Use_H, (Sz < H*W)> {
		
		public:

		using base_t = TilesDomainBase<H, W, Sz, Use_Weight, Use_H, (Sz < H*W)>;

		using State = typename base_t::state_t;
		using PackedState = typename base_t::packed_t;
		using Operator = idx_t;
		using Cost = cost_t;

		
		static const size_t Hash_Range = base_t::Max_Packed+1;
		static const unsigned Board_Size = H*W;
		

		struct OperatorSet {

			public:
			OperatorSet(std::array<tile_t, 5> const& pMoves) :
				mMoves(pMoves)
			{}
		
			unsigned size() const {
				return mMoves[0];
			}
		
			idx_t operator[](unsigned i) const {
				return mMoves[i+1];
			}
		
			private:
			std::array<tile_t, 5> const& mMoves;
		};
	
		struct Edge {
			Edge(State& pState, Cost pCost, idx_t pParentOp) :
				mState(pState), mCost(pCost), mParentOp(pParentOp)
			{}
				
			State& state() {
				return mState;
			}
			
			cost_t cost() {
				return mCost;
			}
			
			idx_t parentOp() {
				return mParentOp;
			}
			
			State& mState;
			cost_t mCost;
			idx_t mParentOp;

		};
		
		

		const idx_t noOp;
		
		TilesDomain(BoardStateV<H,W> const& pInitState, 
					BoardStateV<H,W> const& pGoalState,
					IndexMap<H*W, Sz> const& pMap) :
			
			base_t(pInitState, pGoalState, pMap),
			noOp		(-1),
			mOpLookup	()
		{}
		
		
		State createState() const {
			return base_t::doCreateState();
		}
		
		void packState(State const& pState, PackedState& pPacked) const {
			pPacked = pState.getPacked();
		}
		
		void unpackState(State& pState, PackedState const& pPacked) const {
			base_t::doUnpackState(pState, pPacked);
		}
		
		Edge createEdge(State& pState, idx_t op) const {
			idx_t parentOp = pState.getBlankPos();
			
			base_t::performMove(pState, op);
			
			return Edge(pState, base_t::getMoveCost(op), parentOp);
		}
		
		void destroyEdge(Edge& pEdge) const {
			base_t::performMove(pEdge.state(), pEdge.parentOp());
		}
		
		OperatorSet createOperatorSet(State const& pState) const {
			return OperatorSet(mOpLookup.get(pState.getBlankPos()));
		}
		
		size_t hash(PackedState pPacked) const {
			return base_t::doHash(pPacked);
		}
		
		Cost heuristicValue(State const& pState) const {
			return base_t::getVal_h(pState);
		}
		
		Cost distanceValue(State const& pState) const {
			return base_t::getVal_d(pState);
		}
		
		bool checkGoal(State const& pState) const {
			return base_t::doCheckGoal(pState);
		}

		bool compare(State const& a, State const& b) const {
			return a == b;
		}
		
		bool compare(PackedState const& a, PackedState const& b) const {
			return a == b;
		}
		
		void prettyPrint(State const& s, std::ostream& out) const {
			base_t::prettyPrint(s, out);
		}
		
		void prettyPrint(Operator const& op, std::ostream &out) const {
			out << op << "\n";
		}

		
		private:
		
		const MoveLookup<H,W> mOpLookup;

	};

}}
