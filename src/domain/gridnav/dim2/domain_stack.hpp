#pragma once

#include "util/exception.hpp"
#include "util/json.hpp"
#include "util/debug.hpp"

#include "search/astar.hpp"

#include "domain/gridnav/dim2/common.hpp"


namespace mjon661 { namespace gridnav { namespace dim2 {

	template<typename BaseDomain_t>
	struct DomainStack_BaseOnly {
		static const unsigned Top_Abstract_Level = 0;
		
		template<unsigned L>
		struct Domain : BaseDomain_t {
			Domain(DomainStack_BaseOnly<BaseDomain_t> const& pStack) :
				BaseDomain_t(pStack.mCellMap)
			{
				this->setGoalState(pStack.mGoalState);
			}
		};
		
	
		DomainStack_BaseOnly(CellMap2D<> const& pCellMap, Json const& jConfig) :
			mCellMap(pCellMap)
		{
			if(jConfig.count("init"))
				setInitAndGoal(jConfig.at("init"), jConfig.at("goal"));				
		}
		
		void setInitAndGoal(unsigned i, unsigned g) {
			mInitState = i;
			mGoalState = g;
			fast_assert(mInitState < mCellMap.size() && mGoalState < mCellMap.size());
			fast_assert(mCellMap.cells()[mInitState] == CellMap2D<>::Cell_t::Open);
			fast_assert(mCellMap.cells()[mGoalState] == CellMap2D<>::Cell_t::Open);
		}
		
		unsigned getInitState() const {
			return mInitState;
		}

		
		CellMap2D<> const& mCellMap;
		unsigned mInitState, mGoalState;
	};

	template<typename D>
	std::pair<unsigned,unsigned> genRandomInitAndGoal(CellMap2D<> const& pCellMap, double pMinDist, unsigned pSeed) {
		std::mt19937 randgen(pSeed);
		std::uniform_int_distribution<unsigned> dist(0, pCellMap.size()-1);
		
		D domStack(pCellMap, Json());
		
		while(true) {
			unsigned i = dist(randgen), g = dist(randgen);
			
			double ix = i%pCellMap.getWidth(), iy = i/pCellMap.getWidth();
			double gx = g%pCellMap.getWidth(), gy = g/pCellMap.getWidth();
			
			if(std::hypot(ix-gx, iy-gy) < pMinDist)
				continue;
			
			if(pCellMap.cells()[i] != CellMap2D<>::Cell_t::Open || pCellMap.cells()[g] != CellMap2D<>::Cell_t::Open)
				continue;
			
			domStack.setInitAndGoal(i, g);
			
			using Alg_t = algorithm::Astar<D, algorithm::AstarSearchMode::Speedy, algorithm::AstarHrMode::DomainHr>;
			
			Alg_t alg(domStack, Json());
			
			try {
				alg.execute(domStack.getInitState());
			} catch(NoSolutionException const&) {
				continue;
			}
			
			return {i, g};
		}
	}	
}}}
