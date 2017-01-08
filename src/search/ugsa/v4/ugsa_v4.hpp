#pragma once

#include <string>

#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/math.hpp"
#include "search/solution.hpp"

#include "search/ugsa/v4/behaviour.hpp"
#include "search/ugsa/v4/base_search.hpp"
#include "search/ugsa/v4/level_stats.hpp"
#include "search/ugsa/v4/simple_stats.hpp"
//#include "search/ugsa/v4/null_stats.hpp"

namespace mjon661 { namespace algorithm { namespace ugsav4 {
	
	
	
	template<typename DomStack, typename StatsManager, unsigned Max_Abt_Level>
	struct UGSAv4 {
		
		using BaseDomain = typename DomStack::template Domain<0>;
		using State = typename BaseDomain::State;
		
		static const unsigned Used_Abt_Levels = mathutil::min(Max_Abt_Level, DomStack::Top_Abstract_Level);
		
		UGSAv4(DomStack& pStack, Json const& jConfig) :
			mBehaviour(mConf),
			mStatsManager(),
			mAlgo(pStack, jConfig, mBehaviour, mStatsManager)
		{
		}
		
		
		void execute(Solution<BaseDomain>& pSol) {
			mBehaviour.start();
			mAlgo.doSearch(pSol);
		}

		Json report() {
			mAlgo.submitStats();
			Json j = mStatsManager.report();
			j["behaviour"] = mBehaviour.report();
			return j;
		}
		
		void reset() {
			mAlgo.reset();
			mStatsManager.reset();
			mBehaviour.reset();
		}

		UGSABehaviour<BaseDomain> mBehaviour;
		StatsManager mStatsManager;
		UGSAv4_Base<DomStack, Used_Abt_Levels, StatsManager> mAlgo;
		
	};
	
	
	template<typename D>
	using UGSAv4_StatsLevel = UGSAv4<D, LevelStatsManager<>, 1000>;
	
	template<typename D>
	using UGSAv4_StatsSimple = UGSAv4<D, SimpleStatsManager<>, 1000>;
	
	template<typename D>
	using UGSAv4_StatsSLevel_1 = UGSAv4<D, LevelStatsManager<>, 1>;
}}}
