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
#include "domain/tiles/index_map.hpp"
#include "domain/tiles/mdist.hpp"
#include "domain/tiles/board_state.hpp"



namespace mjon661 { namespace tiles {

	
	template<unsigned H, unsigned W, bool Use_Weight>
	struct CompleteTilesDomain_Manhat {

		using Cost = cost_t;
		using Operator = unsigned;
		
		static const bool Is_Perfect_Hash = true;


		struct State : public BoardState<H,W> {
			State() = default;
			
			State(BoardState<H,W> const& o) :
				BoardState<H,W>(o)
			{}
			
			cost_t get_h() const {return h;} 
			cost_t get_d() const {return d;} 
			void set_h(cost_t ph) {h=ph;} 
			void set_d(cost_t pd) {d=pd;}
			
			cost_t h, d;
		};

		using PackedState = typename State::packed_t;
		
		struct OperatorSet {
			OperatorSet(unsigned i) :
				n(0),
				mvs()
			{
				if(i >= W) mvs[n++] = i - W;
				if(i < (H-1)*W) mvs[n++] = i + W;
				if(i % W != 0) mvs[n++] = i-1;
				if((i+1) % W != 0) mvs[n++] = i+1;
			}
			
			unsigned size() {
				return n;
			}
			
			unsigned operator[](unsigned i) {
				return mvs[i];
			}
			
			private:
			unsigned n;
			std::array<unsigned, 4> mvs;
		};
		
		struct Edge {
			Edge(State& pState, Cost pCost, Operator pParentOp, Cost dh, Cost dd) :
				mState(pState), mCost(pCost), mParentOp(pParentOp), mdh(dh), mdd(dd)
			{}
				
			State& state() {
				return mState;
			}
			
			Cost cost() {
				return mCost;
			}
			
			Operator parentOp() {
				return mParentOp;
			}
			
			State& mState;
			Cost mCost;
			Operator mParentOp;
			Cost mdh, mdd;
		};
		

		CompleteTilesDomain_Manhat(BoardState<H,W> const& pGoalState) :
			mManhattan(pGoalState),
			mGoalState(pGoalState)
		{
			cost_t cst_h, cst_d;
			mManhattan.eval(mGoalState, cst_h, cst_d);
			mGoalState.set_h(cst_h);
			mGoalState.set_d(cst_d);
			
			fast_assert(cst_h == cst_d  && cst_d == 0);
		}
		
		
		Operator getNoOp() const {
			return -1;
		}
		
		void packState(State const& pState, PackedState& pkd) const {
			pkd = pState.getPacked();
		}
		
		void unpackState(State& pState, PackedState const& pkd) const {
			pState.fromPacked(pkd);
			
			cost_t cst_h, cst_d;
			
			mManhattan.eval(pState, cst_h, cst_d);
			pState.set_h(cst_h);
			pState.set_d(cst_d);
		}
		
		OperatorSet createOperatorSet(State const& s) const {
			return OperatorSet(s.getBlankPos());
		}
		
		Edge createEdge(State& s, Operator op) const {
			Cost dh, dd;
			mManhattan.increment(op, s.getBlankPos(), s[op], dh, dd);
			
			s.set_h(s.get_h() + dh);
			s.set_d(s.get_d() + dd);

			Operator oldblank = s.getBlankPos();
			s.moveBlank(op);
			
			return Edge(s, Use_Weight ? oldblank : 1, oldblank, dh, dd);
		}
		
		void destroyEdge(Edge& e) const {
			e.mState.set_h(e.mState.get_h() - e.mdh);
			e.mState.set_d(e.mState.get_d() - e.mdd);
			e.mState.moveBlank(e.mParentOp);
		}
		
		void prettyPrint(State const& pState, std::ostream& out) const {
			pState.prettyPrint(out);
			out << "(h, d): (" << pState.get_h() << ", " << pState.get_d() << ")\n";
		}
		
		void prettyPrint(Operator op, std::ostream& out) const {
			out << op;
		}
		
		size_t hash(PackedState const& pkd) const {
				return pkd;
		}
		
		bool checkGoal(State const& pState) const {
			return pState == mGoalState;
		}

		Cost costHeuristic(State const& pState) const {
			return pState.get_h();
		}
		
		Cost distanceHeuristic(State const& pState) const {
			return pState.get_d();
		}

		bool compare(State const& a, State const& b) const {
			return a == b;
		}

		bool compare(PackedState const& a, PackedState const& b) const {
			return a == b;
		}
		
		std::pair<Cost, Cost> pairHeuristics(State const& pState) const {
			return std::pair<Cost, Cost>(pState.get_h(), pState.get_d());
		}
		

		private:
		const Manhattan<H,W,Use_Weight> mManhattan;
		State mGoalState;
	};
	
	
	
	
	template<unsigned H, unsigned W, bool Use_Weight>
	struct CompleteTilesDomain_NoH {
		
		using Cost = cost_t;
		using Operator = unsigned;
		
		static const bool Is_Perfect_Hash = true;


		using State = BoardState<H,W>;
		using PackedState = typename State::packed_t;
		
		struct OperatorSet {
			OperatorSet(unsigned i) :
				n(0),
				mvs()
			{
				if(i >= W) mvs[n++] = i - W;
				if(i < (H-1)*W) mvs[n++] = i + W;
				if(i % W != 0) mvs[n++] = i-1;
				if((i+1) % W != 0) mvs[n++] = i+1;
			}
			
			unsigned size() {
				return n;
			}
			
			unsigned operator[](unsigned i) {
				return mvs[i];
			}
			
			private:
			unsigned n;
			std::array<unsigned, 4> mvs;
		};
		
		struct Edge {
			Edge(State& pState, Cost pCost, Operator pParentOp) :
				mState(pState), mCost(pCost), mParentOp(pParentOp)
			{}
				
			State& state() {
				return mState;
			}
			
			Cost cost() {
				return mCost;
			}
			
			Operator parentOp() {
				return mParentOp;
			}
			
			State& mState;
			Cost mCost;
			Operator mParentOp;
		};
		

		CompleteTilesDomain_NoH(BoardState<H,W> const& pGoalState) :
			mGoalState(pGoalState)
		{}
		
		
		Operator getNoOp() const {
			return -1;
		}
		
		void packState(State const& pState, PackedState& pkd) const {
			pkd = pState.getPacked();
		}
		
		void unpackState(State& pState, PackedState const& pkd) const {
			pState.fromPacked(pkd);
		}
		
		OperatorSet createOperatorSet(State const& s) {
			return OperatorSet(s.getBlankPos());
		}
		
		Edge createEdge(State& s, Operator op) const {
			Operator oldblank = s.getBlankPos();
			s.moveBlank(op);
			
			return Edge(s, Use_Weight ? oldblank : 1, oldblank);
		}
		
		void destroyEdge(Edge& e) const {
			e.mState.moveBlank(e.mParentOp);
		}
		
		void prettyPrint(State const& pState, std::ostream& out) const {
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
		
		void prettyPrint(Operator op, std::ostream& out) const {
			out << op;
		}
		private:
		State mGoalState;
	};
	
	
	template<unsigned H, unsigned W, bool Use_Weight, bool Use_H>
	struct CompleteTilesDomain;
	
	template<unsigned H, unsigned W, bool Use_Weight>
	struct CompleteTilesDomain<H, W, Use_Weight, true> : public CompleteTilesDomain_Manhat<H, W, Use_Weight> {
		using CompleteTilesDomain_Manhat<H, W, Use_Weight>::CompleteTilesDomain_Manhat;
	};
	
	template<unsigned H, unsigned W, bool Use_Weight>
	struct CompleteTilesDomain<H, W, Use_Weight, false> : public CompleteTilesDomain_NoH<H, W, Use_Weight> {
		using CompleteTilesDomain_NoH<H, W, Use_Weight>::CompleteTilesDomain_NoH;
	};
	
	
	
	template<unsigned H, unsigned W, unsigned Sz, bool Use_Weight>
	struct SubsetTilesDomain {
		using Cost = cost_t;
		using Operator = unsigned;
		
		using State = SubsetBoardState<H, W, Sz>;
		using PackedState = typename State::packed_t;
		
		static const bool Is_Perfect_Hash = true;
		
				
		struct OperatorSet {
			OperatorSet(unsigned i) :
				n(0),
				mvs()
			{
				if(i >= W) mvs[n++] = i - W;
				if(i < (H-1)*W) mvs[n++] = i + W;
				if(i % W != 0) mvs[n++] = i-1;
				if((i+1) % W != 0) mvs[n++] = i+1;
			}
			
			unsigned size() {
				return n;
			}
			
			unsigned operator[](unsigned i) {
				return mvs[i];
			}
			
			private:
			unsigned n;
			std::array<unsigned, 4> mvs;
		};
		
		
		struct Edge {
			Edge(State& pState, Cost pCost, Operator pParentOp) :
				mState(pState), mCost(pCost), mParentOp(pParentOp)
			{}
				
			State& state() {
				return mState;
			}
			
			Cost cost() {
				return mCost;
			}
			
			Operator parentOp() {
				return mParentOp;
			}
			
			State& mState;
			Cost mCost;
			Operator mParentOp;
		};
		
		
		SubsetTilesDomain(TilesAbtSpec<H*W> const& pAbtSpec, BoardState<H,W> const& pBaseGoalState) :
			mAbtSpec(pAbtSpec),
			mGoalState(pBaseGoalState, mAbtSpec)
		{}
		
		template<typename BS>
		State abstractParentState(BS const& bs) const {
			return State(bs, mAbtSpec);
		}
		
		Operator getNoOp() const {
			return -1;
		}

		void unpackState(State& pState, PackedState const& pkd) const {
			pState.fromPacked(pkd, mAbtSpec);
		}

		void packState(State const& pState, PackedState& pkd) const {
			pkd = pState.getPacked(mAbtSpec);
		}
		
		void prettyPrint(State const& pState, std::ostream& out) const {
			pState.prettyPrint(out);
		}
		
		void prettyPrint(Operator op, std::ostream& out) const {
			out << op;
		}
		
		size_t hash(PackedState const& pkd) const {
				return pkd;
		}
		
		OperatorSet createOperatorSet(State const& pState) const {
			return OperatorSet(pState.getBlankPos());
		}
		
		Edge createEdge(State& pState, Operator pOp) const {
			Operator oldblank = pState.getBlankPos();
			pState.moveBlank(pOp);
			return Edge(pState, Use_Weight ? oldblank : 1, oldblank);
		}
		
		void destroyEdge(Edge& e) const {
			e.mState.moveBlank(e.mParentOp);
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
