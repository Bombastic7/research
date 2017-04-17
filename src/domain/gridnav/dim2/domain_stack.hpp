#pragma once

#include "util/json.hpp"
#include "util/debug.hpp"

#include "domain/gridnav/dim2/common.hpp"


namespace mjon661 { namespace gridnav { namespace dim2 {

	template<typename BaseDomain_t>
	struct DomainStack_BaseOnly {
		unsigned Top_Abstract_Level = 0;
		
		template<unsigned L>
		struct Domain : BaseDomain_t {
			Domain(DomainStack_BaseOnly<BaseDomain_t> const& pStack) :
				BaseDomain(pStack.mCellMap)
			{
				this->setGoalState(s);
			}
		};
		
	
		DomainStack_BaseOnly(Json const& jConfig, CellMap2D<> const& pCellMap) :
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

}}}
