#pragma once

#include <array>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

#include "domain/gridnav/blocked/defs.hpp"
#include "domain/gridnav/blocked/maps.hpp"

#include "util/json.hpp"
#include "util/exception.hpp"
#include "util/debug.hpp"
#include "util/math.hpp"

#include <string>
#include <sstream>


namespace mjon661 { namespace gridnav { namespace blocked {
	
	
	/* returns  sum of contiguous rows from pY to {mGoaly-1 / mGoaly+1} if {pY < mGoalY / pY > mGoalY} */
	template<typename = void>
	int verticalPathFactor(int pY, int goaly) {
		int d = std::abs(goaly - pY);

		if(d == 0)
			return 0;

		int s = (d * (d-1)) / 2;
		
		s += pY < goaly ? 
				 pY * d :
			(goaly+1) * d;

		return s;
	}
	
	
	
	template<unsigned H, unsigned W, bool Use_LC, bool Use_H>
	struct FourWayBase {
		
		using cost_t = int;
		using OpSetBase = FourWayMoves<H,W>;
		using state_t = StateImpl<Use_H, cost_t>;
		
		static const unsigned Height = H, Width = W;
		
		
		FourWayBase(idx_t pGoal) :
			mGoalx(pGoal % W),
			mGoaly(pGoal / W)
		{}
		
		cost_t getMoveCost(idx_t pPos, MoveDir pDir, unsigned pBaseRow) const {
			return Use_LC ? pBaseRow : 1;
		}
		
		void getHeuristicValues(idx_t pPos, state_t& pState) const {
			if(!Use_H)
				return;
			
			else if(!Use_LC) {
				cost_t c = manhat(pPos);
				pState.set_h(c);
				pState.set_d(c);
			}
			
			else {
				cost_t h, d;
				lifeCostHeuristics(pPos, h, d);
				pState.set_h(h);
				pState.set_d(d);
			}
		}
		
		
		private:
		
		int manhat(idx_t pState) const {
			int x = pState % W, y = pState / W;
			return std::abs(mGoalx - x) + std::abs(mGoaly - y);
		}
		
		void lifeCostHeuristics(idx_t pPos, int& out_h, int& out_d) const {
			int x = pPos % W, y = pPos / W;
			
			int dx = std::abs(x-mGoalx), miny = mathutil::min(y, mGoaly);
			
			// Horizontal segment at the cheaper of y/gy. Vertical segment straight from y to goaly.
			int p1 = dx * miny + verticalPathFactor(y, mGoaly);
			
			// From (x,y) to (x,0), then to (gx, 0), then to (gx, gy). Note that horizontal segment is free (row 0).
			int p2 = verticalPathFactor(y, 0) + verticalPathFactor(0, mGoaly);
			
			if(p1 < p2) {
				out_h = p1;
				out_d = dx + std::abs(y - mGoaly);
			} else {
				out_h = p2;
				out_d = dx + y + mGoaly;
			}
		}

		const int mGoalx, mGoaly;
		
	};
	
	
	
	
	/**
	 * Use_LC ? Unit cost : life cost
	 * Use_H ?  Manhattan distance based heuristics : none
	 */
	
	template<unsigned H, unsigned W, bool Use_LC, bool Use_H>
	struct EightWayBase {
		
		using cost_t = float;
		using OpSetBase = EightWayMoves<H,W>;
		using state_t = StateImpl<Use_H, cost_t>;
		
		static const unsigned Height = H, Width = W;
		
		
		EightWayBase(idx_t pGoal) :
			mGoalx(pGoal % W),
			mGoaly(pGoal / W)
		{}
		
		cost_t getMoveCost(idx_t pPos, MoveDir pDir, unsigned pBaseRow) const {
			float c = isDiagDir(pDir) ? SQRT2 : 1;
			
			if(Use_LC)
				c *= pBaseRow;
			
			return c;
		}
		
		void getHeuristicValues(idx_t pPos, state_t& pState) const {
			if(!Use_H)
				return;
			
			if(!Use_LC) {
				int dx = std::abs(pPos % W - mGoalx), dy = std::abs(pPos / W - mGoaly);
				
				pState.set_h(std::abs(dx-dy) + mathutil::min(dx, dy) * SQRT2);
				pState.set_d(mathutil::max(dx, dy));
			}
			
			else {
				cost_t h, d;
				lifeCostHeuristics(pPos, h, d);
				pState.set_h(h);
				pState.set_d(d);
			}
		}
		

		private:
		

		void lifeCostHeuristics(idx_t pPos, cost_t& out_h, cost_t& out_d) const {
			int x = pPos % W, y = pPos / W;
			
			int dx = std::abs(x - mGoalx);
			int dy = std::abs(x - mGoaly);
			
			if(dx <= dy) {
				out_h = verticalPathFactor(pPos, mGoaly);
				out_d = dy;
				return;
			}
			
			//int maxdown = min(y, mGoaly);
			int extra = dx - dy;
			
			int down = mathutil::min(mathutil::min(y, mGoaly), (dx-dy)/2);
			int botRow = mathutil::min(y, mGoaly) - down;
			int across = extra - 2*down;
			
			out_h = verticalPathFactor(y, botRow) + across * botRow + verticalPathFactor(botRow, mGoaly);
			out_d = dx;
		}
		
		const int mGoalx, mGoaly;
	};
	
	
	
	template<typename CellMap_t, bool Use_H>
	struct GridNav_BaseDom {
		using State = unsigned;
		using PackedState = unsigned;
		using Cost = typename CellMap_t::Cost_t;
		using Operator = unsigned;
		using AdjacentCells = typename CellMap_t::AdjacentCells;
		
		
		Operator noOp;

		struct OperatorSet {
			unsigned size() {
				return mN;
			}
			
			Operator operator[](unsigned i) {
				return i;
			}
			
			OperatorSet(AdjacentCells const& pAdj) :
				mN(0)
			{
				for(unsigned o : pAdj)
					if(o != Null_Idx)
						mN++;
			}
			
			private:
			const unsigned mN;
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
			noOp(Null_Idx),
			mCellMap(pCellMap),
			mInitState(pInitState),
			mGoalState(pGoalState)
		{}

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
			return OperatorSet(mAllEdges[pState]);
		}
		
		size_t hash(PackedState const& pPacked) const {
			return pPacked;
		}
		
		Cost heuristicValue(State const& pState) const {
			return 
		}
		
		Cost distanceValue(State const& pState) const {
			return pState.get_d();
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
		
		void prettyPrint(State const& s, std::ostream& out) const {
			out << "( " << s % mCellmap.mWidth << ", " << s.pos / mCellMap.mWidth << " )\n";
		}
		
		void prettyPrint(Operator const& op, std::ostream &out) const {
			out << op << "\n";
		}
		
		size_t getHashRange() {
			return mCellMap.mSize;
		}
		

		private:	
		CellMap_t const& mCellMap;
		const State mInitState, mGoalState;
	};







	template<typename NavMap_t>
	struct GridNav_AbtDom {

		using State = unsigned;
		using PackedState = unsigned;
		using Cost = typename NavMap_t::Cost_t;
		using Operator = unsigned;

		using InterGroupEdge = typename NavMap_t::InterGroupEdge;
		
		//static const size_t Hash_Range = Height * Width;

		Operator noOp;

		struct OperatorSet {
			unsigned size() {
				return mEdges.size();
			}
			
			Operator operator[](unsigned i) {
				return i;
			}
			
			OperatorSet(std::vector<InterGroupEdge> const& pEdges) :
				mN(pEdges.size())
			{}
			
			private:
			const unsigned mN;
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
		


		GridNav_AbtDom(std::vector<std::vector<InterGroupEdge>> const& pAllEdges, State mGoalState) :
			noOp(Null_Idx),
			mAllEdges(pAllEdges),
			mInitState(pInitState),
			mGoalState(pGoalState)
		{}

			
		void packState(State const& pState, PackedState& pPacked) const {
			pPacked = pState;
		}
		
		void unpackState(State& pState, PackedState const& pPacked) const {
			pState = pPacked;
		}
		
		Edge createEdge(State& pState, Operator op) const {			
			
			InterGroupEdge const& e = pAllEdges[pState][op];
			
			std::vector<InterGroupEdge> const& dstedges = pAllEdges[e.dst];
			Operator parentOp = Null_Idx;
			
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

		bool compare(PackedState const& a, PackedState const& b) const {
			return a == b;
		}
		
		void prettyPrint(State const& s, std::ostream& out) const {
			out << "( " << s % Width << ", " << s.pos / Width << " )\n";
		}
		
		void prettyPrint(Operator const& op, std::ostream &out) const {
			out << op << "\n";
		}
		
		size_t getHashRange() {
			return mAllEdges.size();
		}
		

		private:
		std::vector<std::vector<InterGroupEdge>> const& mAllEdges;
		const State mInitState, mGoalState;
	};

}}}
