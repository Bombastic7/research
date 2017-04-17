#pragma once

#include <utility>
#include <cmath>
#include <array>
#include <algorithm>

#pragma once

#include <iostream>

#include "util/debug.hpp"
#include "util/math.hpp"
#include "util/json.hpp"

#include <boost/operators.hpp>

#include "domain/gridnav/dim2/common.hpp"


namespace mjon661 { namespace gridnav { namespace dim2 { namespace eightway {
	
	struct Cost_t : public boost::totally_ordered<Cost_t>, boost::additive<Cost_t> {
		static constexpr double Diag_Mv_Cost = 1.41421356237309504880168872420969807857;
		
		unsigned short dg, st;
			
		Cost_t() = default;
		
		template<typename T>
		Cost_t(T t) : Cost_t(0,0) {
			slow_assert(t == 0);
		}
		
		Cost_t(unsigned pDg, unsigned pSt) :
			dg(pDg), st(pSt)
		{}
		
		bool operator<(Cost_t const& o) const {
			return costVal() < o.costVal(); 
		}
		
		bool operator==(Cost_t const& o) const {
			return dg == o.dg && st == o.st;
		}
		
		Cost_t operator+=(Cost_t const& o) {
			dg += o.dg;
			st += o.st;
			return *this;
		}
		
		Cost_t operator-=(Cost_t const& o) {
			dg -= o.dg;
			st -= o.st;
			slow_assert(dg >= 0 && st >= 0);
			return *this;
		}
		
		double costVal() const {
			return dg * Diag_Mv_Cost + st;
		}
	};
	
	
	std::ostream& operator<<(std::ostream& out, Cost_t const& cst) {
		out << "(" << cst.dg << "," << cst.st << "," << cst.costVal() << ")";
		return out;
	}
	
	
	
	
	template<bool Use_LC>
	struct BaseEdgeIterator {
		enum {D_U, D_D, D_L, D_R, D_UL, D_UR, D_DL, D_DR, D_End};
		
		BaseEdgeIterator(CellMap2D<> const& pCellMap, unsigned pPos) :
			mCellMap(pCellMap),
			mPos(pPos),
			mCost(Use_LC ? pPos / mCellMap.getWidth() : 1),
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
			return mAdjPos[mCurDir];
		}
		
		Cost_t cost() const {
			slow_assert(!finished());
			if(mCurDir >= D_UL)
				return Cost_t(mCost, 0);
			return Cost_t(0, mCost);
		}
		
		void next() {
			mCurDir++;
			adv();
		}
		
		
		private:

		void adv() {
			for(; mCurDir<D_End; mCurDir++) {
				if(mAdjPos[mCurDir] != (unsigned)-1) {
					return;
				}
			}
		}
		
		void fillAdjPos() {
			bool oU = false, oD = false, oL = false, oR = false;
			
			std::fill(mAdjPos.begin(), mAdjPos.end(), (unsigned)-1);
			
			if(mPos >= mCellMap.getWidth()) {
				mAdjPos[D_U] = mPos - mCellMap.getWidth();
				oU = true;
			}
			if(mPos < (mCellMap.getHeight()-1) * mCellMap.getWidth()) {
				mAdjPos[D_D] = mPos + mCellMap.getWidth();
				oD = true;
			}
			if(mPos % mCellMap.getWidth() != 0) {
				mAdjPos[D_L] = mPos - 1;
				oL = true;
			}
			if((mPos+1) % mCellMap.getWidth() != 0) {
				mAdjPos[D_R] = mPos + 1;
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
					if(mCellMap.cells()[mAdjPos[i]] != CellMap2D<>::Cell_t::Open)
						mAdjPos[i] = (unsigned)-1;
			}
		}
		

		
		CellMap2D<> const& mCellMap;
		const unsigned mPos;
		const unsigned mCost;
		int mCurDir;
		std::array<unsigned, 8> mAdjPos;
	};
	
	
	
	
	
	void unitCostHeuristics(unsigned pPos, unsigned pGoal, unsigned pWidth, Cost_t& out_h, Cost_t& out_d) {		
		int dx = std::abs(pPos % pWidth - pGoal % pWidth), dy = std::abs(pPos / pWidth - pGoal / pWidth);
			
		out_h = Cost_t(mathutil::min(dx, dy), std::abs(dx-dy));
		out_d = Cost_t(0, mathutil::max(dx, dy));
	}
	
	void lifeCostHeuristics(unsigned pPos, unsigned pGoal, unsigned pWidth, Cost_t& out_h, Cost_t& out_d) {		
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
