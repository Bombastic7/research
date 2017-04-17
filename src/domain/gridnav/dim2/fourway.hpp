#pragma once

#include <utility>

#include "util/debug.hpp"
#include "util/math.hpp"
#include "util/json.hpp"

#include "domain/gridnav/dim2/common.hpp"


namespace mjon661 { namespace gridnav { namespace dim2 { namespace fourway {
	
	using Cost_t = unsigned;
	
	template<bool Use_LC>
	struct BaseEdgeIterator {
		enum {D_Up, D_Down, D_Left, D_Right, D_End};
		
		BaseEdgeIterator(CellMap2D<> const& pCellMap, unsigned pPos) :
			mCellMap(pCellMap),
			mPos(pPos),
			mCost(Use_LC ? pPos / pCellMap.getWidth() : 1),
			mCurDir(D_Up),
			mCurAdjPos()
		{
			adv();
		}
		
		bool finished() const {
			return mCurDir == D_End;
		}
		
		unsigned state() const {
			slow_assert(!finished());
			return mCurAdjPos;
		}
		
		Cost_t cost() const {
			slow_assert(!finished());
			return mCost;
		}
		
		void next() {
			mCurDir++;
			adv();
		}
		
		
		private:
		
		void adv() {
			for(int i=mCurDir; i<D_End; i++) {
				unsigned adjPos;
				
				if(i == D_Up && mPos >= mCellMap.getWidth()) adjPos = mPos - mCellMap.getWidth();
				else if(i == D_Down && mPos < (mCellMap.getHeight()-1) * mCellMap.getWidth()) adjPos = mPos + mCellMap.getWidth();
				else if(i == D_Left && mPos % mCellMap.getWidth() != 0) adjPos = mPos - 1;
				else if(i == D_Right && (mPos+1) % mCellMap.getWidth() != 0) adjPos = mPos + 1;
				else continue;
				
				slow_assert(adjPos < mCellMap.size());
				if(mCellMap.cells()[adjPos] != CellMap2D<>::Cell_t::Open) continue;
				
				mCurAdjPos = adjPos;
				mCurDir = i;
				return;
			}
		}
		

		
		CellMap2D<> const& mCellMap;
		const unsigned mPos;
		const Cost_t mCost;
		int mCurDir;
		unsigned mCurAdjPos;
	};
	
	
	
	
	
	void unitCostHeuristics(unsigned pPos, unsigned pGoal, unsigned pWidth, Cost_t& out_h, Cost_t& out_d) {
		out_h = out_d = manhat(pPos, pGoal, pWidth);
	}
	
	void lifeCostHeuristics(unsigned pPos, unsigned pGoal, unsigned pWidth, Cost_t& out_h, Cost_t& out_d) {		
		int x = pPos % pWidth, y = pPos / pWidth;
		int gx = pGoal % pWidth, gy = pGoal / pWidth;
		
		int dx = std::abs(x-gx), miny = mathutil::min(y, gy);
		
		// Horizontal segment at the cheaper of y/gy. Vertical segment straight from y to goaly.
		int p1 = dx * miny + verticalPathFactor(y, gy);
		
		// From (x,y) to (x,0), then to (gx, 0), then to (gx, gy). Note that horizontal segment is free (row 0).
		int p2 = verticalPathFactor(y, 0) + verticalPathFactor(0, gy);
		
		if(p1 < p2) {
			out_h = p1;
			out_d = dx + std::abs(y - gy);
		} else {
			out_h = p2;
			out_d = dx + y + gy;
		}
	}
	

	
	template<bool Use_LC, bool Use_Hr>
	class BaseDomain {
		public:
		using State = unsigned;
		using PackedState = unsigned;
		using Cost = Cost_t;
		
		static const bool Is_Perfect_Hash = true;
		
		using AdjEdgeIterator = BaseEdgeIterator<Use_LC>;
		
		BaseDomain(CellMap2D<> const& pCellMap) :
			mCellMap(pCellMap),
			mGoalState(-1)
		{}

		void setGoalState(unsigned s) {
			mGoalState = s;
		}

		void packState(State const& pState, PackedState& pPacked) const {
			pPacked = pState;
		}
		
		void unpackState(State& pState, PackedState const& pPacked) const {
			pState = pPacked;
		}

		AdjEdgeIterator getAdjEdges(unsigned s) const {
			slow_assert(mCellMap.cells().at(s) == CellMap2D<>::Cell_t::Open);
			return AdjEdgeIterator(mCellMap, s);
		}

		size_t hash(PackedState pPacked) const {
			return pPacked;
		}
		
		Cost costHeuristic(unsigned pState) const {
			if(~Use_Hr)
				return 0;
			
			Cost h, d;
			if(Use_LC)
				lifeCostHeuristics(pState, mGoalState, mCellMap.getWidth(), h, d);
			else
				unitCostHeuristics(pState, mGoalState, mCellMap.getWidth(), h, d);
			return h;
		}
		
		Cost distanceHeuristic(unsigned pState) const {
			if(~Use_Hr)
				return 0;
			
			Cost h, d;
			if(Use_LC)
				lifeCostHeuristics(pState, mGoalState, mCellMap.getWidth(), h, d);
			else
				unitCostHeuristics(pState, mGoalState, mCellMap.getWidth(), h, d);
			return d;
		}
		
		std::pair<Cost, Cost> pairHeuristics(unsigned pState) const {
			if(~Use_Hr)
				return {0,0};
			
			Cost h, d;
			if(Use_LC)
				lifeCostHeuristics(pState, mGoalState, mCellMap.getWidth(), h, d);
			else
				unitCostHeuristics(pState, mGoalState, mCellMap.getWidth(), h, d);
			return {h,d};
		}
		
		bool checkGoal(unsigned pState) const {
			return pState == mGoalState;
		}

		bool compare(unsigned a, unsigned b) const {
			return a == b;
		}

		void prettyPrintState(State const& s, std::ostream& out) const {
			out << "[" << s << ", " << s%mCellMap.getWidth() << ", " << s/mCellMap.getWidth() << "]";
		}


		private:
		CellMap2D<> const& mCellMap;
		unsigned mGoalState;
	};

}}}}
