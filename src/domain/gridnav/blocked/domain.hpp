#pragma once

#include <array>
#include <vector>
#include <iostream>
#include <string>

#include "util/json.hpp"
#include "util/debug.hpp"
#include "util/math.hpp"

#include "domain/gridnav/blocked/graph.hpp"


namespace mjon661 { namespace gridnav { namespace blocked {
	
	
	template<typename CellMap_t, bool Use_H, size_t Suggested_Hash_Range>
	struct GridNav_BaseDom {
		using State = unsigned;
		using PackedState = unsigned;
		using Cost = typename CellMap_t::Cost_t;
		using Operator = short;
		using AdjacentCells = typename CellMap_t::AdjacentCells;

		static const size_t Hash_Range = Suggested_Hash_Range;
		
		Operator noOp;

		struct OperatorSet {
			unsigned size() {
				return mN;
			}
			
			Operator operator[](unsigned i) {
				return mValidOps[i];
			}
			
			OperatorSet(AdjacentCells const& pAdj) :
				mN(0)
			{
				for(unsigned i=0; i<pAdj.size(); i++)
					if(pAdj[i] != Null_Idx) {
						mValidOps[mN] = i;
						mN++;
					}
			}
			
			private:
			unsigned mN;
			std::array<Operator, 8> mValidOps;
		};
		
		
		struct Edge {
			Edge(State pState, Cost pCost, Operator pParentOp) :
				mState(pState),
				mCost(pCost),
				mParentOp(pParentOp)
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
			
			State mState;
			Cost mCost;
			Operator mParentOp;			
		};
		

		GridNav_BaseDom(CellMap_t const& pCellMap, State pInitState, State pGoalState) :
			noOp(-1),
			mCachedState(Null_Idx),
			mCached_h(0),
			mCached_d(0),
			mCellMap(pCellMap),
			mInitState(pInitState),
			mGoalState(pGoalState)
		{
			gen_assert(pCellMap.mSize <= Hash_Range);
		}

		State createState() const {
			return mInitState;
		}
			
		void packState(State const& pState, PackedState& pPacked) const {
			pPacked = pState;
		}
		
		void unpackState(State& pState, PackedState const& pPacked) const {
			pState = pPacked;
		}
		
		Edge createEdge(State& pState, Operator op) const {
			AdjacentCells const& adjcells = mCellMap.getAdjCells(pState);
			return Edge(adjcells[op], mCellMap.getOpCost(pState, op), mCellMap.reverseOp(op));
		}
		
		void destroyEdge(Edge&) const {
		}
		
		OperatorSet createOperatorSet(State const& pState) const {
			return OperatorSet(mCellMap.getAdjCells(pState));
		}
		
		size_t hash(PackedState const& pPacked) const {
			return pPacked;
		}
		
		Cost heuristicValue(State const& pState) const {
			if(!Use_H)
				return 0;
			
			if(mCachedState != pState) {
				mCachedState = pState;
				Cost cached_h, cached_d;
				mCellMap.getHeuristicValues(pState, mGoalState, cached_h, cached_d);
				mCached_h = cached_h;
				mCached_d = cached_d;
			}
			
			return mCached_h;	
		}
		
		Cost distanceValue(State const& pState) const {
			if(!Use_H)
				return 0;
			
			if(mCachedState != pState) {
				mCachedState = pState;
				Cost cached_h, cached_d;
				mCellMap.getHeuristicValues(pState, mGoalState, cached_h, cached_d);
				mCached_h = cached_h;
				mCached_d = cached_d;
			}
			
			return mCached_d;
		}
		
		bool checkGoal(State const& pState) const {
			return pState == mGoalState;
		}

		bool compare(State const& a, State const& b) const {
			return a == b;
		}

		//~ bool compare(PackedState const& a, PackedState const& b) const {
			//~ return a == b;
		//~ }
		
		void prettyPrint(State const& s, std::ostream& out) const {
			mCellMap.prettyPrintIndex(s, out);
		}
		
		void prettyPrint(Operator const& op, std::ostream &out) const {
			mCellMap.prettyPrintDir(op, out);
		}


		private:
		
		mutable State mCachedState;
		mutable Cost mCached_h, mCached_d;
		
		CellMap_t const& mCellMap;
		const State mInitState, mGoalState;
	};







	template<typename NavMap_t, size_t Suggested_Hash_Range>
	struct GridNav_AbtDom {

		using State = unsigned;
		using PackedState = unsigned;
		using Cost = typename NavMap_t::Cost_t;
		using Operator = short;

		using InterGroupEdge = typename NavMap_t::InterGroupEdge;

		static const size_t Hash_Range = Suggested_Hash_Range;
		
		Operator noOp;

		struct OperatorSet {
			unsigned size() {
				return mEdges.size();
			}
			
			Operator operator[](unsigned i) {
				return i;
			}
			
			OperatorSet(std::vector<InterGroupEdge> const& pEdges) :
				mN(pEdges.size()),
				mEdges(pEdges)
			{}
			
			private:
			const unsigned mN;
			std::vector<InterGroupEdge> const& mEdges;
		};
		
		
		struct Edge {
			
			Edge(State pState, Cost pCost, Operator pParentOp) :
				mState(pState),
				mCost(pCost),
				mParentOp(pParentOp)
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
			
			State mState;
			Cost mCost;
			Operator mParentOp;			
		};
		


		GridNav_AbtDom(std::vector<std::vector<InterGroupEdge>> const& pAllEdges, State pGoalState) :
			noOp(-1),
			mAllEdges(pAllEdges),
			mGoalState(pGoalState)
		{
			gen_assert(pAllEdges.size() <= Hash_Range);
		}

			
		void packState(State const& pState, PackedState& pPacked) const {
			pPacked = pState;
		}
		
		void unpackState(State& pState, PackedState const& pPacked) const {
			pState = pPacked;
		}
		
		Edge createEdge(State& pState, Operator op) const {			
			
			InterGroupEdge const& e = mAllEdges[pState][op];
			
			std::vector<InterGroupEdge> const& dstedges = mAllEdges[e.dst];
			Operator parentOp = noOp;
			
			for(unsigned i=0; i<dstedges.size(); i++) {
				if(dstedges[i].dst != pState)
					continue;
				
				parentOp = i;
				break;
			}
				
			return Edge(e.dst, e.cost, parentOp);
		}
		
		void destroyEdge(Edge&) const {
		}
		
		OperatorSet createOperatorSet(State const& pState) const {
			return OperatorSet(mAllEdges[pState]);
		}
		
		size_t hash(PackedState const& pPacked) const {
			return pPacked;
		}
		
		bool checkGoal(State const& pState) const {
			return pState == mGoalState;
		}

		bool compare(State const& a, State const& b) const {
			return a == b;
		}

		//~ bool compare(PackedState const& a, PackedState const& b) const {
			//~ return a == b;
		//~ }
		
		void prettyPrint(State const& s, std::ostream& out) const {
			out << "( " << s << " (" << (char)(s % 26 + 'a') << ") )\n";
		}
		
		void prettyPrint(Operator const& op, std::ostream &out) const {
			out << op << "\n";
		}
		
		size_t getHashRange() {
			return mAllEdges.size();
		}
		

		private:
		std::vector<std::vector<InterGroupEdge>> const& mAllEdges;
		const State mGoalState;
	};

}}}
