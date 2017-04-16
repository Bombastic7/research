#pragma once

#include <utility>
#include <cmath>
#include <array>
#include <algorithm>

#include "util/debug.hpp"
#include "util/math.hpp"
#include "util/json.hpp"

#include <boost/operators.hpp>

#include "domain/gridnav/dim2/common.hpp"


namespace mjon661 { namespace gridnav { namespace dim2 { namespace fourway {
	
	struct Cost_t : public boost::totally_ordered<Cost_t> {
		static constexpr double Diag_Mv_Cost = 1.41421356237309504880168872420969807857;
		
		unsigned short dg, st;
			
		Cost_t() = default;
		
		Cost_t(unsigned pDg, unsigned pSt) :
			dg(pDg), st(pSt)
		{}
		
		bool operator<(Cost_t const& o) const {
			return costVal() < o.costVal(); 
		}
		
		bool operator==(Cost_t const& o) const {
			return dg == o.dg && st == o.st;
		}
		
		double costVal() const {
			return dg * Diag_Mv_Cost + st;
		}
	};
	
	
	
	
	template<bool Use_LC>
	struct BaseEdgeIterator {
		enum {D_U, D_D, D_L, D_R, D_UL, D_UR, D_DL, D_DR, D_End};
		
		BaseEdgeIterator(CellMap2D<> const& pCellMap, unsigned pPos) :
			mCellMap(pCellMap),
			mPos(pPos),
			mRow(pPos / mCellMap.getWidth()),
			mCurDir(D_U),
			mAdjPos()
		{
			fillAdjPos();
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
			if(mCurDir >= D_UL)
				return Cost_t(Use_LC ? mRow : 1, 0);
			return Cost_t(0, Use_LC ? mRow : 1);
		}
		
		void next() {
			mCurDir++;
			adv();
		}
		
		
		private:

		void adv() {
			for(int i=mCurDir; i<D_End; i++) {
				if(mAdjPos[mCurDir] != (unsigned)-1)
					mCurDir = i;
					return;
				}
			}
		}
		
		void fillAdjPos() {
			bool oU = false, oD = false, oL = false, oR = false;
			
			std::fill(mAdjPos.begin(), mAdjPos.end(), (unsigned)-1);
			
			if(i == D_Up && i >= mCellMap.getWidth()) {
				mAdjPos[D_U] = i - mCellMap.getWidth();
				oU = true;
			}
			if(i == D_Down && i < (mCellMap.getHeight()-1) * mCellMap.getWidth()) {
				mAdjPos[D_D] = i + mCellMap.getWidth();
				oD = true;
			}
			if(i == D_Left && i % mCellMap.getWidth() != 0) {
				mAdjPos[D_L] = i - 1;
				oL = true;
			}
			if(i == D_Right && (i+1) % mCellMap.getWidth() != 0) {
				mAdjPos[D_R] = i + 1;
				oR = true;
			}
			
			if(oU && oL)
				mAdjPos[D_UL] = mAdjPos[D_U] - 1;
			if(oU && oR)
				mAdjPos[D_UR] = mAdjPos[D_U] + 1;
			if(oD && oL)
				mAdjPos[D_DL] = mAdjPos[D_D] - 1;
			if(oD && oR)
				mAdjPos[D_DR] = mAdjPos[D_D] + 1;
			
			for(unsigned i=0; i<mAdjPos.size(); i++) {
				if(mAdjPos[i] != (unsigned)-1)
					if(mCellMap.cells()[mAdjPos[i]] != Cell_t::Open)
						mAdjPos[i] = (unsigned)-1;
			}
		}
		

		
		CellMap2D<> const& mCellMap;
		const unsigned mPos;
		int mCurDir;
		std::array<unsigned, 8> mCurAdjPos;
	};
	
	
	
	
	
	void unitCostHeuristics(unsigned pPos, unsigned pGoal, Cost_t& out_h, Cost_t& out_d) const {
		unsigned pWidth = this->getWidth();
		
		int dx = std::abs(pPos % pWidth - pGoal % pWidth), dy = std::abs(pPos / pWidth - pGoal / pWidth);
			
		out_h = Cost_t(mathutil::min(dx, dy), std::abs(dx-dy));
		out_d = Cost_t(0, mathutil::max(dx, dy));
	}
	
	void lifeCostHeuristics(unsigned pPos, unsigned pGoal, Cost_t& out_h, Cost_t& out_d) const {
		unsigned pWidth = this->getWidth();
		
		int x = pPos % pWidth, y = pPos / pWidth;
		int gx = pGoal % pWidth, gy = pGoal / pWidth;
		
		int dx = std::abs(x - gx);
		int dy = std::abs(y - gy);
		
		if(dx <= dy) {
			out_h = Cost_t(0, verticalPathFactor(pPos, gy));
			out_d = Cost_t(0, dy);
			return;
		}
		
		//int maxdown = min(y, mGoaly);
		int extra = dx - dy;
		
		int down = mathutil::min(mathutil::min(y, gy), (dx-dy)/2);
		int botRow = mathutil::min(y, y) - down;
		int across = extra - 2*down;
		
		out_h = Cost_t(0, verticalPathFactor(y, botRow) + across * botRow + verticalPathFactor(botRow, gy));
		out_d = Cost_t(0, dx);
	}

	
	template<bool Use_LC, bool Use_Hr>
	class BaseDomain {
		public:
		using State = unsigned;
		using PackedState = unsigned;
		using Cost = Cost_t;
		
		static const bool Is_Perfect_Hash = true;
		
		using AdjEdgeIterator = BaseEdgeIterator;
		
		BaseDomain(CellMap2D const& pCellMap) :
			mCellGraph(pCellGraph),
			mGoalState(-1)
		{}

		State getGoalState() const {
			return mGoalState;
		}

		void packState(State const& pState, PackedState& pPacked) const {
			pPacked = pState;
		}
		
		void unpackState(State& pState, PackedState const& pPacked) const {
			pState = pPacked;
		}

		BaseEdgeIterator getAdjEdges(unsigned s) const {
			slow_assert(mCellGraph.isOpen(s));
			return BaseEdgeIterator(mCellmap, s);
		}

		size_t hash(PackedState pPacked) const {
			return pPacked;
		}
		
		Cost costHeuristic(unsigned pState) const {
			if(~Use_Hr)
				return 0;
			
			Cost h, d;
			if(Use_LC)
				lifeCostHeuristics(pState, mGoalState, h, d);
			else
				unitCostHeuristics(pState, pGoal, h, d);
			return h;
		}
		
		Cost distanceHeuristic(unsigned pState) const {
			if(~Use_Hr)
				return 0;
			
			Cost h, d;
			if(Use_LC)
				lifeCostHeuristics(pState, mGoalState, h, d);
			else
				unitCostHeuristics(pState, pGoal, h, d);
			return d;
		}
		
		std::pair<Cost, Cost> pairHeuristics(unsigned pState) const {
			if(~Use_Hr)
				return {0,0};
			
			Cost h, d;
			if(Use_LC)
				lifeCostHeuristics(pState, mGoalState, h, d);
			else
				unitCostHeuristics(pState, pGoal, h, d);
			return {h,d};
		}
		
		bool checkGoal(unsigned pState) const {
			return pState == mGoalState;
		}

		bool compare(unsigned a, unsigned b) const {
			return a == b;
		}

		void prettyPrintState(State const& s, std::ostream& out) const {
			out << "[" << s << ", " << s%mCellGraph.getWidth() << ", " << s/mCellGraph.getWidth() << "]";
		}


		private:
		CellMap2D<> const& mCellGraph;
		const unsigned mGoalState;
	};

}}}
