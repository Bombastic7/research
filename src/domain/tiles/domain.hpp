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

		
		template<bool, typename = void>
		struct StateImpl : public BoardStateV<H,W> {
			using BoardStateV<H,W>::BoardStateV;
			
			cost_t get_h() const {return 0;} 
			cost_t get_d() const {return 0;} 
			void set_h(cost_t) {} 
			void set_d(cost_t) {}
		};
		
		template<typename Ign>
		struct StateImpl<true, Ign> : public BoardStateV<H,W> {
			using BoardStateV<H,W>::BoardStateV;
			
			cost_t get_h() const {return h;} 
			cost_t get_d() const {return d;} 
			void set_h(cost_t ph) {h=ph;} 
			void set_d(cost_t pd) {d=pd;}
			
			cost_t h, d;
		};
		
		

		
		using state_t = StateImpl<Use_H>;

		using packed_t = typename state_t::packed_t;
		
		static const size_t Max_Packed = state_t::Max_Packed;
		
		
		CompleteTilesBase(BoardStateV<H,W> const& pInitState, BoardStateV<H,W> const& pGoalState) :
			mManhattan(pGoalState),
			mInitState(pInitState),
			mGoalState(pGoalState)
		{
			cost_t cst_h, cst_d;
			
			mManhattan.eval(mInitState, cst_h, cst_d);
			mInitState.set_h(cst_h);
			mInitState.set_d(cst_d);
			
			mManhattan.eval(mGoalState, cst_h, cst_d);
			mGoalState.set_h(cst_h);
			mGoalState.set_d(cst_d);
			
			fast_assert(cst_h == cst_d  && cst_d == 0);
		}
		
		state_t doCreateState() const {
			return mInitState;
		}
		
		void doUnpackState(state_t& pState, packed_t const& pkd) const {
			pState.fromPacked(pkd);
			
			cost_t cst_h, cst_d;
			
			mManhattan.eval(pState, cst_h, cst_d);
			pState.set_h(cst_h);
			pState.set_d(cst_d);
		}

		cost_t getMoveCost(state_t& pState, idx_t op) const {
			return Use_Weight ? op : 1;
		}
		
		void prettyPrint(state_t const& pState, std::ostream& out) const {
			pState.prettyPrint(out);
			out << "(h, d): (" << pState.get_h() << ", " << pState.get_d() << ")\n";
		}
		
		size_t doHash(packed_t const& pkd) const {
				return pkd;
		}
		
		void performMove(state_t& pState, idx_t op) const {
			if(Use_H) {
				cost_t dh, dd;
				mManhattan.increment(op, pState.getBlankPos(), pState[op], dh, dd);
				
				pState.set_h(pState.get_h() + dh);
				pState.set_d(pState.get_d() + dd);
			}
			pState.moveBlank(op);
		}

		
		cost_t getVal_h(state_t const& pState) const {
			return pState.get_h();
		}
		
		cost_t getVal_d(state_t const& pState) const {
			return pState.get_d();
		}
		
		bool doCheckGoal(state_t const& pState) const {
			return pState == mGoalState;
		}
		

		private:
		const Manhattan<H,W,Use_Weight> mManhattan;
		state_t mInitState, mGoalState;
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

		cost_t getMoveCost(state_t& pState, idx_t op) const {
			
			return Use_Weight ? op : 1;
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
	struct TilesDomainBase<H, W, Sz, Use_Weight, Use_H, true> : public SubsetTilesBase<H, W, Sz, Use_Weight> {

		TilesDomainBase(BoardStateV<H,W> const& pInitState, 
						BoardStateV<H,W> const& pGoalState,
						IndexMap<H*W, Sz> const& pMap) :
						
			SubsetTilesBase<H, W, Sz, Use_Weight>(pGoalState, pMap)
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
			
			base_t		(pInitState, pGoalState, pMap),
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
			cost_t edgeCost = base_t::getMoveCost(pState, op);
			
			slow_assert(pState.getBlankPos() != op);
			slow_assert(edgeCost > 0 && (unsigned)edgeCost < Board_Size, "%d", edgeCost);
			
			
			base_t::performMove(pState, op);
			
			return Edge(pState, edgeCost, parentOp);
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
