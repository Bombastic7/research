
#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <vector>
#include <utility>

#include "util/debug.hpp"
#include "util/permutation.hpp"
#include "util/hash.hpp"

#include "domain/pancake/defs.hpp"
#include "domain/pancake/pancake_stack.hpp"

#include "domain/pancake/abstractor.hpp"

namespace mjon661 { namespace pancake {
	
	
	//Pancake domain, No heuristics

	template<unsigned N, bool Use_Hr, bool Use_Weight>
	class BaseDomain {
		
		public:
		
		using Cost = cost_t;
		
		using State = PancakeStack<N>;
		using PackedState = typename State::packed_t;
		
		static const unsigned Npancakes = N;
		static const bool Is_Perfect_Hash = false;
		
		
		struct AdjEdgeIterator {
			
			AdjEdgeIterator(State& pState) :
				mAdjState(pState),
				mLastOp(1),
				mTest_origState(pState)
			{
				mAdjState.flip(1);
			}
			
			~AdjEdgeIterator() {
				if(!finished())
					mAdjState.flip(mLastOp);
					
				slow_assert(mAdjState == mTest_origState);
			}
			
			State& state() const {
				slow_assert(!finished());
				return mAdjState;
			}
			
			Cost cost() const {
				slow_assert(!finished());
				return Use_Weight ? mLastOp : 1;
			}
			
			bool finished() const {
				return mLastOp == (unsigned)-1;
			}
			
			void next() {
				slow_assert(!finished());
				mAdjState.flip(mLastOp);
				mLastOp++;
				if(mLastOp == N) {
					mLastOp = (unsigned)-1;
					slow_assert(mTest_origState == mAdjState);
					return;
				}
				mAdjState.flip(mLastOp);
			}
			
			State& mAdjState;
			unsigned mLastOp;
			
			const State mTest_origState;
		};

			
		void packState(State const& pState, PackedState& pPacked) const {
			pPacked = pState.getPacked();
		}
		
		void unpackState(State& pState, PackedState const& pPacked) const {
			pState.fromPacked(pPacked);
		}
		
		AdjEdgeIterator getAdjEdges(State& pState) const {
			return AdjEdgeIterator(pState);
		}

		size_t hash(PackedState const& pPacked) const {
			return State::doHash(pPacked);
		}

		Cost costHeuristic(State const& pState) const {
			return 0;
		}
		
		Cost distanceHeuristic(State const& pState) const {
			return heuristicValue(pState);
		}
		
		std::pair<Cost,Cost> pairHeuristics(State const& pState) {
			return {costHeuristic(pState), distanceHeuristic(pState)};
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
		
		void prettyPrintState(State const& s, std::ostream& out) const {
			s.prettyPrint(out);
		}
		
		void initialiseState(State& s) const {}
	};
	
	

	

	
	
	//Pancake domain, gap heuristic
	
	template<unsigned N, bool Use_Weight>
	struct BaseDomain<N, true, Use_Weight> : public BaseDomain<N, false, Use_Weight> {
		static_assert(!Use_Weight, "");
		
		using base_t = Domain_NoH<N, false>;
		using base_t::Cost;
		using base_t::PackedState;
		
		
		struct State : public base_t::State {
			State() = default;
			
			State(typename base_t::State const& o) :
				base_t::State(o)
			{}
			
			bool operator==(State const& o) const {
				bool b = *this == o;
				if(b) slow_assert(hrVal == o.hrVal);
				return b;
			
			Cost hrVal;
		};


		struct AdjEdgeIterator {
			AdjEdgeIterator(State& pState) :
				mAdjState(pState),
				mLastOp(1),
				mLastHrVal(pState.hrVal),
				mTest_origState(pState)
			{
				bool gap_before = hasGap(mAdjState, mLastOp);
				mAdjState.flip(mLastOp);
				bool gap_after = hasGap(mAdjState, mLastOp);
		
				if (gap_before && !gap_after)
					mAdjState.hrVal--;
				else if (!gap_before && gap_after)
					mAdjState.hrVal++;
			}
			
			~AdjEdgeIterator() {
				if(!finished()) {
					mAdjState.hrVal = mOldHrVal;
					mAdjState.flip(mLastOp);
				}
					
				slow_assert(mAdjState == mTest_origState);
			}
			
			State& state() const {
				slow_assert(!finished());
				return mAdjState;
			}
			
			Cost cost() const {
				slow_assert(!finished());
				return Use_Weight ? mLastOp : 1;
			}
			
			bool finished() const {
				return mLastOp == (unsigned)-1;
			}
			
			void next() {
				slow_assert(!finished());
				mAdjState.flip(mLastOp);
				mAdjState.hrVal = mLastHrVal;
				mLastOp++;
				if(mLastOp == N) {
					mLastOp = (unsigned)-1;
					slow_assert(mTest_origState == mAdjState);
					return;
				}
				mAdjState.flip(mLastOp);
				bool gap_before = hasGap(mAdjState, mLastOp);
				mAdjState.flip(mLastOp);
				bool gap_after = hasGap(mAdjState, mLastOp);
		
				if (gap_before && !gap_after)
					mAdjState.hrVal--;
				else if (!gap_before && gap_after)
					mAdjState.hrVal++;
			}
			
			State& mAdjState;
			unsigned mLastOp;
			unsigned mLastHrVal;
			
			const State mTest_origState;
		};

		
		void initialiseState(State& s) const {
			s0.hVal = computeGapHeuristic(s);
		}
		
		void unpackState(State& pState, typename base_t::PackedState& pPacked) const {
			base_t::unpackState(pState, pPacked);
			pState.hVal = computeGapHeuristic(pState);
		}

		
		Cost costHeuristic(State const& pState) const {
			return pState.hVal;
		}
		
		Cost distanceHeuristic(State const& pState) const {
			return pState.hVal;
		}
		
		std::pair<Cost,Cost> pairHeuristics(State const& pState) const {
			return {pState.hval, pState.hval};
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
	
	
	
	//Domain for abstract space, N cakes, Sz which are not ignored.
	template<unsigned N, unsigned Sz, bool Use_Weight>
	struct AbtDomain {
		
		public:
		
		using Cost = cost_t;		
		using State = PartialPancakeStack<N, Sz>;
		using PackedState = typename State::packed_t;
		
		
		static const unsigned Npancakes = N, NKeptPancakes = Sz;
		static const bool Is_Perfect_Hash = false;
		

		struct AdjEdgeIterator {
			
			AdjEdgeIterator(State& pState) :
				mAdjState(pState),
				mLastOp(1),
				mTest_origState(pState)
			{
				mAdjState.flip(1);
			}
			
			~AdjEdgeIterator() {
				if(!finished())
					mAdjState.flip(mLastOp);
					
				slow_assert(mAdjState == mTest_origState);
			}
			
			State& state() const {
				slow_assert(!finished());
				return mAdjState;
			}
			
			Cost cost() const {
				slow_assert(!finished());
				return Use_Weight ? mLastOp : 1;
			}
			
			bool finished() const {
				return mLastOp == (unsigned)-1;
			}
			
			void next() {
				slow_assert(!finished());
				mAdjState.flip(mLastOp);
				mLastOp++;
				if(mLastOp == N) {
					mLastOp = (unsigned)-1;
					slow_assert(mTest_origState == mAdjState);
					return;
				}
				mAdjState.flip(mLastOp);
			}
			
			State& mAdjState;
			unsigned mLastOp;
			
			const State mTest_origState;
		};
		
		
		AbtDomain(std::array<cake_t, N> const& pCakeDropLevel, unsigned pLevel) :
			mAbtor(pCakeDropLevel, pLevel)
		{}

		template<typename BS>
		State abstractParentState(BS const& bs) const {
			return mAbtor(bs);
		}
			
		void packState(State const& pState, PackedState& pPacked) const {
			pPacked = pState.getPacked();
		}
		
		void unpackState(State& pState, PackedState const& pPacked) const {
			pState.fromPacked(pPacked);
		}

		AdjEdgeIterator getAdjEdges(State& pState) const {
			return AdjEdgeIterator(pState)
		}
		

		size_t hash(PackedState const& pPacked) const {
			return State::doHash(pPacked);
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

		const IgnoreCakesAbt<N, Sz> mAbtor;
	};
}}
