#pragma once

#include <string>

#include "util/debug.hpp"
#include "util/json.hpp"
#include "search/solution.hpp"

#include "search/hastar/v2/common.hpp"
#include "search/hastar/v2/base_search.hpp"
#include "search/hastar/v2/level_stats.hpp"


namespace mjon661 { namespace algorithm { namespace hastarv2 {
	

	
	template<typename DomStack, typename StatsManager>
	struct HAstar {
		
		using BaseDomain = typename DomStack::template Domain<0>;
		using State = typename BaseDomain::State;
		
		
		HAstar(DomStack& pStack, Json const&) :
			mStatsManager(),
			mAlgo(pStack, mStatsManager)
		{
			
		}
		
		
		void execute(Solution<BaseDomain>& pSol) {
			mAlgo.doSearch(pSol);
		}

		Json report() {
			mAlgo.submitStats();
			Json j = mStatsManager.report();
			return j;
		}
		
		void reset() {
			mAlgo.reset();
			mStatsManager.reset();
		}


		StatsManager mStatsManager;
		HAstar_Base<DomStack, DomStack::Top_Abstract_Level, StatsManager> mAlgo;
		
	};
	
	
	template<typename D>
	using HAstar_StatsLevel = HAstar<D, LevelStatsManager<>>;
	
}}}
