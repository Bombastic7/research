#pragma once

#include <array>
#include <iostream>
#include <random>
#include <vector>
#include <cmath>
#include <queue>
#include <string>
#include <fstream>
#include <unordered_map>
#include <utility>
#include <algorithm>

#include "util/debug.hpp"
#include "util/math.hpp"
#include "util/json.hpp"

#include <cstdio>

namespace mjon661 { namespace gridnav { namespace blocked {


	enum struct Cell_t {
		Open, Blocked, Null
	};
	
	enum struct CardDir_t {
		N, S, W, E, NW, NE, SW, SE
	};
	


	template<bool Use_Nways, bool Use_LifeCost>
	inline unsigned manhat(unsigned pState, unsigned pGoal, unsigned pWidth) {
		int x = pState % pWidth, y = pState / pWidth;
		int gx = pGoal % pWidth, gy = pGoal / pWidth;
		return std::abs(gx - x) + std::abs(gy - y);
	}

	/* returns  sum of contiguous rows from pY to {mGoaly-1 / mGoaly+1} if {pY < mGoalY / pY > mGoalY} */
	inline unsigned verticalPathFactor(int pY, int goaly) {
		int d = std::abs(goaly - pY);

		if(d == 0)
			return 0;

		int s = (d * (d-1)) / 2;
		
		s += pY < goaly ? 
				 pY * d :
			(goaly+1) * d;

		return s;
	}
	
	template<typename = void>
	class CellMap {
		
		public:
		
		class StateIterator {
			public:
			
			StateIterator& operator++() {
				if(mIdx == mInst.getSize())
					return *this;
				do {
					++mIdx;
				} while(mIdx < mInst.getSize() && mInst.isOpen(mIdx));
				
				return *this;
			}
			
			bool operator==(StateIterator const& o) {
				return mIdx == o.mIdx;
			}
			
			bool operator!=(StateIterator const& o) {
				return mIdx != o.mIdx;
			}
			
			unsigned operator*() {
				return mIdx;
			}
			
			private:
			friend CellMap<void>;
			
			StateIterator(CellMap<void> const& pInst, bool pAtEnd) :
				mInst(pInst),
				mIdx(0)
			{
				if(pAtEnd)
					mIdx = mInst.getSize();
			}
			
			CellMap<void> const& mInst;
			unsigned mIdx;
		};
		
		
		CellMap(unsigned pSize, std::string const& pMapFile) :
			mSize(pSize),
			mCells(mSize)
		{
			std::ifstream ifs(pMapFile);
			
			if(!ifs)
				throw std::runtime_error("Could not open map file");

			for(unsigned i=0; i<mSize; i++) {
				int v;
				Cell_t c;
				ifs >> v;
				c = (Cell_t)v;
				
				gen_assert(c == Cell_t::Open || c == Cell_t::Blocked);
				mCells[i] = c;
			}
		}
		
		std::vector<Cell_t> const& cells() {
			return mCells;
		}
		
		unsigned getSize() const {
			return mSize;
		}
		
		bool isOpen(unsigned i) const {
			slow_assert(i < mSize);
			return mCells[i] == Cell_t::Open;
		}
		
		StateIterator stateBegin() const {
			return StateIterator(*this, false);
		}
		
		StateIterator stateEnd() const {
			return StateIterator(*this, true);
		}
		
		private:
		const unsigned mSize;
		std::vector<Cell_t> mCells;
	};
	
	
	
	template<bool Use_LifeCost>
	class CellGraph_4 : public CellMap<> {
		public:
		using Cost_t = int;

		struct AdjacentCells {
			unsigned n;
			std::array<unsigned, 4> adjCells;
		};
		
		CellGraph_4(unsigned pHeight, unsigned pWidth, std::string const& pMapFile) :
			CellMap(pHeight*pWidth, pMapFile),
			mHeight(pHeight),
			mWidth(pWidth)
		{}

		const AdjacentCells getAdjacentCells(unsigned s) const {
			AdjacentCells adj{.n=0};
			
			Cost_t costMul = Use_LifeCost ? s/mWidth : 1;
			
			if(s >= mWidth && this->isOpen(s-mWidth))
				adj.adjCells[adj.n++] = s-mWidth;
			if(s < (mHeight-1)*mWidth && this->isOpen(s+mWidth))
				adj.adjCells[adj.n++] = s+mWidth;
			if(s%mWidth != 0 && this->isOpen(s-1))
				adj.adjCells[adj.n++] = s-1;
			if((s+1)%mWidth != 0 && this->isOpen(s+1))
				adj.adjCells[adj.n++] = s+1;
			return adj;
		}
		
		Cost_t getMoveCost(unsigned src, unsigned dst) const {
			return Use_LifeCost ? src/mWidth : 1;
		}

		unsigned getHeight() const {
			return mHeight;
		}
		
		unsigned getWidth() const {
			return mWidth;
		}
		
		private:
		const unsigned mHeight, mWidth;
	};
	
	
	template<bool Use_LifeCost>
	class CellGraph_8 : public CellMap<> {
		using Cost_t = float;
		
		static constexpr Cost_t Diag_Mv_Cost = 1.41421356237309504880168872420969807857;

		struct AdjacentCells {
			unsigned n;
			std::array<unsigned, 8> adjCells;
		};
		
		CellGraph_8(unsigned pHeight, unsigned pWidth, std::string const& pMapFile) :
			CellMap(pHeight*pWidth, pMapFile),
			mHeight(pHeight),
			mWidth(pWidth)
		{}

		const AdjacentCells getAdjacentCells(unsigned s) const {
			AdjacentCells adj{.n=0};

			bool vn=false, vs=false, ve=false, vw=false;
			
			if(s >= mWidth && this->isOpen(s-mWidth)) {
				adj.adjCells[adj.n++] = s-mWidth;
				vn = true;
			}
			if(s < (mHeight-1)*mWidth && this->isOpen(s+mWidth)) {
				adj.adjCells[adj.n++] = s+mWidth;
				vs = true;
			}
			if(s%mWidth != 0 && this->isOpen(s-1)) {
				adj.adjCells[adj.n++] = s-1;
				vw = true;
			}
			if((s+1)%mWidth != 0 && this->isOpen(s+1)) {
				adj.adjCells[adj.n++] = s+1;
				ve = true;
			}
			if(vn && vw && this->isOpen(s-mWidth-1))
				adj.adjCells[adj.n++] = s-mWidth-1;
			if(vn && ve && this->isOpen(s-mWidth+1))
				adj.adjCells[adj.n++] = s-mWidth+1;
			if(vs && vw && this->isOpen(s+mWidth-1))
				adj.adjCells[adj.n++] = s+mWidth-1;
			if(vs && ve && this->isOpen(s+mWidth+1))
				adj.adjCells[adj.n++] = s+mWidth+1;
			return adj;
		}
		
		Cost_t getMoveCost(unsigned src, unsigned dst) const {
			Cost_t cst = Use_LifeCost ? src/mWidth : 1;

			if(src%mWidth != dst%mWidth && src/mWidth != dst/mWidth)
				cst *= Diag_Mv_Cost;
			
			return cst;
		}
		
		unsigned getHeight() const {
			return mHeight;
		}
		
		unsigned getWidth() const {
			return mWidth;
		}
		
		private:
		const unsigned mHeight, mWidth;
	};
	
	
	template<bool Use_LifeCost, bool Use_Hr>
	class CellGraph_4_hr : public CellGraph_4<Use_LifeCost> {
		public:
		using typename CellGraph_4<Use_LifeCost>::Cost_t;
		
		CellGraph_4_hr(unsigned pHeight, unsigned pWidth, std::string const& pMapFile) :
			CellGraph_4<Use_LifeCost>(pHeight, pWidth, pMapFile)
		{}
		
		void compHrVals(unsigned pPos, unsigned pGoal, Cost_t& out_h, Cost_t& out_d) const {
			if(!Use_Hr) {
				out_h = 0;
				out_d = 0;
				return;
			}
			
			if(Use_LifeCost)
				return lifeCostHeuristics(pPos, pGoal, out_h, out_d);
			return unitCostHeuristics(pPos, pGoal, out_h, out_d);
		}
		
		private:
		void unitCostHeuristics(unsigned pPos, unsigned pGoal, Cost_t& out_h, Cost_t& out_d) const {
			out_h = out_d = manhat(pPos, pGoal, this->getWidth());
		}
		
		void lifeCostHeuristics(unsigned pPos, unsigned pGoal, Cost_t& out_h, Cost_t& out_d) const {
			unsigned pWidth = this->getWidth();
			
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
	};
	

	template<bool Use_LifeCost, bool Use_Hr>
	class CellGraph_8_hr : public CellGraph_8<Use_LifeCost> {
		public:
		using typename CellGraph_8<Use_LifeCost>::Cost_t;
		using CellGraph_8<Use_LifeCost>::Diag_Mv_Cost;
		
		CellGraph_8_hr(unsigned pHeight, unsigned pWidth, std::string const& pMapFile) :
			CellGraph_8<Use_LifeCost>(pHeight, pWidth, pMapFile)
		{}
		
		void compHrVals(unsigned pPos, unsigned pGoal, Cost_t& out_h, Cost_t& out_d) const {
			if(!Use_Hr) {
				out_h = 0;
				out_d = 0;
				return;
			}
			
			if(Use_LifeCost)
				return lifeCostHeuristics(pPos, pGoal, out_h, out_d);
			return unitCostHeuristics(pPos, pGoal, out_h, out_d);
		}
		
		private:
		
		void unitCostHeuristics(unsigned pPos, unsigned pGoal, Cost_t& out_h, Cost_t& out_d) const {
			unsigned pWidth = this->getWidth();
			
			int dx = std::abs(pPos % pWidth - pGoal % pWidth), dy = std::abs(pPos / pWidth - pGoal / pWidth);
				
			out_h = std::abs(dx-dy) + mathutil::min(dx, dy) * Diag_Mv_Cost;
			out_d = mathutil::max(dx, dy);
		}
		
		void lifeCostHeuristics(unsigned pPos, unsigned pGoal, Cost_t& out_h, Cost_t& out_d) const {
			unsigned pWidth = this->getWidth();
			
			int x = pPos % pWidth, y = pPos / pWidth;
			int gx = pGoal % pWidth, gy = pGoal / pWidth;
			
			int dx = std::abs(x - gx);
			int dy = std::abs(y - gy);
			
			if(dx <= dy) {
				out_h = verticalPathFactor(pPos, gy);
				out_d = dy;
				return;
			}
			
			//int maxdown = min(y, mGoaly);
			int extra = dx - dy;
			
			int down = mathutil::min(mathutil::min(y, gy), (dx-dy)/2);
			int botRow = mathutil::min(y, y) - down;
			int across = extra - 2*down;
			
			out_h = verticalPathFactor(y, botRow) + across * botRow + verticalPathFactor(botRow, gy);
			out_d = dx;
		}
	};



	template<unsigned Nways, bool Use_LifeCost, bool Use_Hr>
	class CellGraph_;
	
	template<bool Use_LifeCost, bool Use_Hr>
	class CellGraph_<4, Use_LifeCost, Use_Hr> : public CellGraph_4_hr<Use_LifeCost, Use_Hr> {
		using CellGraph_4_hr<Use_LifeCost, Use_Hr>::CellGraph_4_hr;
	};

	template<bool Use_LifeCost, bool Use_Hr>
	class CellGraph_<8, Use_LifeCost, Use_Hr> : public CellGraph_8_hr<Use_LifeCost, Use_Hr> {
		using CellGraph_8_hr<Use_LifeCost, Use_Hr>::CellGraph_8_hr;
	};
	
	template<unsigned Nways, bool Use_LifeCost, bool Use_Hr>
	class CellGraph : public CellGraph_<Nways, Use_LifeCost, Use_Hr> {
		using CellGraph_<Nways, Use_LifeCost, Use_Hr>::CellGraph_;
	};
	
	
	template<unsigned Nways, bool Use_LifeCost, bool Use_Hr>
	class GridNav_BaseDomain : public CellGraph<Nways, Use_LifeCost, Use_Hr> {
		public:
		
		using Base_t = CellGraph<Nways, Use_LifeCost, Use_Hr>;
		using State = unsigned;
		using PackedState = unsigned;
		using Cost = typename Base_t::Cost_t;
		using Operator = unsigned;
		
		
		struct OperatorSet : public Base_t::AdjacentCells {
			OperatorSet(typename Base_t::AdjacentCells const& o) :
				Base_t::AdjacentCells(o)
			{}
			
			unsigned size() {
				return this->n;
			}
			
			Operator const& operator[](unsigned i) {
				return this->adjCells[i];
			}
		};
		
		struct Edge {
			Edge(unsigned pState, Cost pCost, unsigned pParentOp) :
				mState(pState),
				mCost(pCost),
				mParentOp(pParentOp)
			{}
			
			unsigned state() {
				return mState;
			}
			
			Cost cost() {
				return mCost;
			}
			
			unsigned parentOp() {
				return mParentOp;
			}
			
			const unsigned mState;
			const Cost mCost;
			const unsigned mParentOp;
		};
		
		
		GridNav_BaseDomain(Json const& jConfig) :
			Base_t(jConfig.at("height"), jConfig.at("width"), jConfig.at("map")),
			mGoalState(jConfig.at("goal"))
		{}
		
		Operator getNoOp() {
			return (unsigned)-1;
		}

		void packState(State const& pState, PackedState& pPacked) const {
			pPacked = pState;
		}
		
		void unpackState(State& pState, PackedState const& pPacked) const {
			pState = pPacked;
		}
		
		Edge createEdge(State pState, Operator op) const {
			return Edge(op, this->getMoveCost(pState, op), pState);
		}
		
		void destroyEdge(Edge&) const {
		}
		
		OperatorSet createOperatorSet(unsigned pState) const {
			return OperatorSet(this->getAdjacentCells(pState));
		}
		
		size_t hash(PackedState pPacked) const {
			return pPacked;
		}
		
		bool isPerfectHash() const {
			return true;
		}
		
		Cost costHeuristic(unsigned pState) const {
			Cost h, d;
			this->getHeuristicValues(pState, mGoalState, h, d);
			return h;
		}
		
		Cost distanceHeuristic(unsigned pState) const {
			Cost h, d;
			this->getHeuristicValues(pState, mGoalState, h, d);
			return d;
		}
		
		std::pair<Cost, Cost> pairHeuristics(unsigned pState) const {
			Cost h, d;
			this->getHeuristicValues(pState, mGoalState, h, d);
			return std::pair<Cost, Cost>(h, d);
		}
		
		bool checkGoal(unsigned pState) const {
			return pState == mGoalState;
		}

		bool compare(unsigned a, unsigned b) const {
			return a == b;
		}

		//~ bool compare(PackedState const& a, PackedState const& b) const {
			//~ return a == b;
		//~ }
		
		void prettyPrint(State const& s, std::ostream& out) const {
			out << "[" << s << ", " << s%this->getWidth() << ", " << s/this->getWidth() << "]";
		}
		
		//~ void prettyPrint(Operator const& op, std::ostream &out) const {

		//~ }

		private:
		const unsigned mGoalState;
	};
	
	
	
	template<typename BaseDomain>
	class StarAbtStack {
	
		public:
		using BaseState = typename BaseDomain::State;
		using Cost = typename BaseDomain::Cost;
		using OperatorSet = typename BaseDomain::OperatorSet;
		using Edge = typename BaseDomain::Edge;

		struct GroupEdge {
			unsigned dst;
			Cost cost;
		};
		
		using BaseGroupTrns = std::unordered_map<BaseState, unsigned>;
		using GroupTrns = std::vector<unsigned>;
		
		using GroupAdj = std::vector<GroupEdge>;
		using LevelGroupAdj = std::vector<GroupAdj>;

		static const unsigned Null_Group = (unsigned)-1;


		StarAbtStack(BaseDomain const& pDomain, unsigned pAbtRadius) :
			mDomain(pDomain),
			mAbtRadius(pAbtRadius)
		{
			prepBaseGroups();
			while(prepAbtGroups()) {}
		}

		void prepBaseGroups() {
			unsigned curgrp = 0;
			
			for(auto it = mDomain.stateBegin(); it != mDomain.stateEnd(); ++it) {
				slow_assert(mBaseTrns.count(*it) == 0);
				mBaseTrns[*it] = curgrp++;
			}
			
			mLevelGroupAdj.push_back(LevelGroupAdj(curgrp));
			
			for(auto it = mDomain.stateBegin(); it != mDomain.stateEnd(); ++it) {
				OperatorSet opset = mDomain.createOperatorSet(*it);
				
				GroupAdj grpadj;
				
				for(unsigned i=0; i<opset.size(); i++) {
					Edge edge = mDomain.createEdge(*it, opset[i]);
					grpadj.push_back(GroupEdge{.dst=mBaseTrns[edge.state()], .cost=edge.cost()});
					mDomain.destroyEdge(edge);
				}
				
				mLevelGroupAdj[0][mBaseTrns[*it]] = grpadj;
			}
		}
		
		bool prepAbtGroups() {
			unsigned nullGroup = Null_Group;
			mLevelGroupTrns.push_back(GroupTrns(mLevelGroupAdj.back().size(), nullGroup));
			
			std::vector<std::pair<unsigned, unsigned>> hubprio;
			
			for(unsigned i=0; i<mLevelGroupAdj.back().size(); i++)
				hubprio.push_back(std::pair<unsigned, unsigned>(mLevelGroupAdj.back()[i].size(), i));
			
			std::sort(hubprio.begin(), hubprio.end(), HubPrioCmp());
			
			unsigned curAbtGrp = 0;
			std::vector<unsigned> singletonGroups;
			
			for(unsigned i=0; i<hubprio.size(); i++) {
				if(tryAssignGroupRec(i, 0, curAbtGrp) == 1)
					singletonGroups.push_back(i);
				curAbtGrp++;
			}
			
			for(unsigned i=0; i<singletonGroups.size(); i++) {
				
				for(unsigned j=0; j<mLevelGroupAdj.back()[i].size(); i++)
					slow_assert(std::find(singletonGroups.begin(), singletonGroups.end(), mLevelGroupAdj.back()[singletonGroups[i]][j].dst) == singletonGroups.end());
			
				if(mLevelGroupAdj.back()[i].size() > 0) {
					slow_assert(mLevelGroupTrns.back()[mLevelGroupAdj.back()[i][0].dst] != Null_Group);
					mLevelGroupTrns.back()[singletonGroups[i]] = mLevelGroupTrns.back()[mLevelGroupAdj.back()[singletonGroups[i]][0].dst];
				}
			}
			
			std::map<unsigned, unsigned> groupRelabel;
			curAbtGrp = 0;
			
			for(unsigned i=0; i<mLevelGroupTrns.back().size(); i++) {
				if(groupRelabel.count(mLevelGroupTrns.back()[i] == 0))
					groupRelabel[mLevelGroupTrns.back()[i]] = curAbtGrp++;
				mLevelGroupTrns.back()[i] = groupRelabel[mLevelGroupTrns.back()[i]];
			}
			
			std::map<unsigned, std::map<unsigned, Cost>> abtgroupadj; //srcgrp -> dstgrp -> edgecost
			
			for(unsigned i=0; i<mLevelGroupAdj.back().size(); i++) {
				for(unsigned j=0; j<mLevelGroupAdj.back()[i].size(); j++) {
					unsigned srcgrp = mLevelGroupTrns.back()[i];
					unsigned dstgrp = mLevelGroupTrns.back()[mLevelGroupAdj.back()[i][j].dst];
					
					if(srcgrp == dstgrp)
						continue;
					
					Cost edgecost = mLevelGroupAdj.back()[i][j].cost;
					
					if(abtgroupadj[srcgrp][dstgrp] > edgecost)
						abtgroupadj[srcgrp][dstgrp] = edgecost;
				}
			}
			
			bool isTrivial = true;
			
			mLevelGroupAdj.push_back(LevelGroupAdj(curAbtGrp));
			
			for(unsigned i=0; i<curAbtGrp; i++) {
				for(auto it=abtgroupadj[i].begin(); it != abtgroupadj[i].end(); ++it) {
					mLevelGroupAdj.back()[i].push_back(GroupEdge{.dst=it->first, .cost=it->second});
					isTrivial = false;
				}
			}
			
			return isTrivial;
		}
		
		
		unsigned tryAssignGroupRec(unsigned i, unsigned depth, unsigned curAbtGrp) {
			if(mLevelGroupTrns.back()[i] != Null_Group || depth > mAbtRadius)
				return 0;
			
			mLevelGroupTrns.back()[i] = curAbtGrp;
			
			unsigned ret = 1;
			for(unsigned j=0; j<mLevelGroupAdj.back()[i].size(); j++)
				ret += tryAssignGroupRec(mLevelGroupAdj.back()[i][j].dst, depth+1, curAbtGrp);
			
			return ret;
		}
		
		
		private:
		
		struct HubPrioCmp {
			bool operator()(std::pair<unsigned, unsigned> const& a, std::pair<unsigned, unsigned> const& b) const {
				if(a.first != b.first)
					return a.first > b.first;
				return a.second < b.second;
			}
		};
		
		BaseGroupTrns mBaseTrns;
		std::vector<LevelGroupAdj> mLevelGroupAdj;
		std::vector<GroupTrns> mLevelGroupTrns;
		BaseDomain const& mDomain;
		const unsigned mAbtRadius;
	};
}}}
