
#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <vector>

#include "util/debug.hpp"
#include "util/permutation.hpp"
#include "util/hash.hpp"

#include "domain/pancake/defs.hpp"
#include "domain/pancake/pancake_stack.hpp"


namespace mjon661 { namespace pancake {
	
	
	//Pancake domain, No heuristics
	
	template<unsigned N>
	class Domain_NoH {
		
		public:
		
		using Cost = cost_t;
		using Operator = int;
		
		using State = PancakeStack<N>;
		using PackedState = typename PancakeStack<N>::packed_t;
		
		
		static const unsigned Npancakes = N;
		static const size_t Hash_Range = PancakeStack<N>::Hash_Range;
		

		
		struct OperatorSet {
		
			unsigned size() const {
				return N-1;
			}
		
			Operator operator[](unsigned i) const {
				return i+1;
			}
		};
	
		struct Edge {
			Edge(Cost pCost, State& pState, Operator pParentOp) :
				mCost(pCost), mState(pState), mParentOp(pParentOp) {}
				
			State& state() {
				return mState;
			}
			
			Cost cost() {
				return mCost;
			}
			
			Operator parentOp() {
				return mParentOp;
			}
			
			Cost mCost;
			State& mState;
			Operator mParentOp;
		};
		
		
		
		
		Domain_NoH() :
			noOp(0),
			mHasInitState(false)
		{}
		
		Domain_NoH(PancakeStack<N> const& pInit) :
			noOp(0),
			mHasInitState(true),
			mInitState(pInit)
		{}
		
		State createState() const {
			fast_assert(mHasInitState);
			return mInitState;
		}
			
		void packState(State const& pState, PackedState& pPacked) const {
			pPacked = pState.getPacked();
		}
		
		void unpackState(State& pState, PackedState const& pPacked) const {
			pState.fromPacked(pPacked);
		}
		
		
		Edge createEdge(State& pState, Operator op) const {
			pState.flip(op);
			return Edge(1, pState, op);
		}
		
		void destroyEdge(Edge& pEdge) const {
			pEdge.mState.flip(pEdge.mParentOp);
		}
		
		
		OperatorSet createOperatorSet(State const&) const {
			return OperatorSet();
		}
		
		size_t hash(PackedState const& pPacked) const {
			return pPacked;
		}
		
		
		Cost heuristicValue(State const& pState) const {
			return 0;
		}
		
		Cost distanceValue(State const& pState) const {
			return heuristicValue(pState);
		}
		
		
		bool checkGoal(State const& pState) const {
			return pState.isSorted();
		}

		bool compare(State const& a, State const& b) const {
			return a == b;
		}
		
		bool compare(PackedState const& a, PackedState const& b) const {
			return a == b;
		}
		
		void prettyPrint(State const& s, std::ostream& out) const {
			s.prettyPrint(out);
		}
		
		void prettyPrint(Operator const& op, std::ostream &out) const {
			out << op << "\n";
		}

		const Operator noOp;
		
		private:

		const bool mHasInitState;
		PancakeStack<N> mInitState;


	};
	
	
	//Pancake domain, gap heuristic
	
	template<unsigned N>
	struct Domain_GapH : public Domain_NoH<N> {
		
		using base_t = Domain_NoH<N>;
		using base_t::Cost;
		using base_t::Operator;
		using base_t::PackedState;
		
		
		struct State : public base_t::State {
			
			State() = default;
			
			State(typename base_t::State const& o) :
				base_t::State(o)
			{}
			
			typename base_t::Cost hVal;
		};

		
		struct Edge : public base_t::Edge {
			
			Edge(typename base_t::Cost pCost, State& pState, typename base_t::Operator pParentOp, typename base_t::Cost pOldH ) :
				base_t::Edge(pCost, pState, pParentOp),
				mOldH(pOldH),
				mState(pState)
			{}
			
			State& state() {
				return mState;
			}
			
			typename base_t::Cost mOldH;
			State& mState;
		};
		
		Domain_GapH() = default;
		
		Domain_GapH(PancakeStack<N> const& pInit) :
			base_t(pInit)
		{}
		
		
		State createState() const {
			State s0(base_t::createState());
			s0.hVal = computeGapHeuristic(s0);
			return s0;
		}
		
		void unpackState(State& pState, typename base_t::PackedState& pPacked) const {
			base_t::unpackState(pState, pPacked);
			pState.hVal = computeGapHeuristic(pState);
		}

		
		Edge createEdge(State& pState, typename base_t::Operator op) const {
			
			cost_t oldH = pState.hVal;
			
			bool gap_before = hasGap(pState, op);
			pState.flip(op);

			bool gap_after = hasGap(pState, op);
		
			if (gap_before && !gap_after)
				pState.hVal--;
			
			else if (!gap_before && gap_after)
				pState.hVal++;
			
			return Edge(1, pState, op, oldH);
		}
		
		
		void destroyEdge(Edge& pEdge) const {
			pEdge.mState.flip(pEdge.mParentOp);
			pEdge.mState.hVal = pEdge.mOldH;
		}
		
		
		
		cost_t heuristicValue(State const& pState) const {
			return pState.hVal;
		}
		
		cost_t distanceValue(State const& pState) const {
			return pState.hVal;
		}
		
		
		static int computeGapHeuristic(PancakeStack<N> const& pState) {
			int n = 0;

			for (unsigned i = 0; i < N; i++) {
				if (hasGap(pState, i))
					n++;
			}
			return n;
		}
		
		static bool hasGap(PancakeStack<N> const& pState, unsigned idx) {
			slow_assert(idx < N);
			
			if (idx == N-1)
				return pState[N-1] != N-1;
			
			int df = (int)pState[idx] - (int)pState[idx+1];

			if(df == -1)
				df = 1;
			
			return df != 1;
		}
		
		
	};
		
}}
