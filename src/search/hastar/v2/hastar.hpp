#pragma once

#include <string>

#include "util/debug.hpp"
#include "util/json.hpp"
#include "search/solution.hpp"

#include "search/hastar/v2/common.hpp"
#include "search/hastar/v2/base_search.hpp"
#include "search/hastar/v2/level_stats.hpp"


namespace mjon661 { namespace algorithm { namespace hastarv2 {
	

	
	template<typename DomStack, typename StatsManager, unsigned Top_Limit>
	struct HAstar {
		
		using BaseDomain = typename DomStack::template Domain<0>;
		using State = typename BaseDomain::State;
		
		static const unsigned Top_Abt_Level_Used = mathutil::min(DomStack::Top_Abstract_Level, Top_Limit);
		
		HAstar(DomStack& pStack, Json const& jConfig) :
			mStatsManager(),
			mConf(jConfig),
			mAlgo(pStack, mStatsManager, mConf)
		{
			
		}
		
		
		void execute(Solution<BaseDomain>& pSol) {
			mAlgo.doSearch(pSol);
		}

		Json report() {
			mAlgo.submitStats();
			Json j = mStatsManager.report();
			j["used conf"] = mConf.report();
			return j;
		}
		
		void reset() {
			mAlgo.reset();
			mStatsManager.reset();
		}


		StatsManager mStatsManager;
		AlgoConf<> mConf;
		HAstar_Base<DomStack, Top_Abt_Level_Used, StatsManager> mAlgo;
		
	};
	
	
	template<typename D>
	using HAstar_StatsLevel = HAstar<D, LevelStatsManager<>, 1000>;
	
	template<typename D>
	using HAstar_StatsLevel_1lvl = HAstar<D, LevelStatsManager<>, 1>;
	
	template<typename D>
	using HAstar_StatsSimple = HAstar<D, SimpleStatsManager<>, 1000>;
	
	template<typename D>
	using HAstar_StatsSimple_1lvl = HAstar<D, SimpleStatsManager<>, 1>;
}}}
