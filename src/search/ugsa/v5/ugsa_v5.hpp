#pragma once

#include <string>

#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/math.hpp"
#include "search/solution.hpp"

#include "search/ugsa/v5/base_search.hpp"
#include "search/ugsa/v5/simple_stats.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav5 {
	

	
	template<typename DomStack, typename StatsManager, unsigned Top_Limit>
	struct UGSAv5 {
		
		using BaseDomain = typename DomStack::template Domain<0>;
		using State = typename BaseDomain::State;
		
		static const unsigned Top_Abt_Level_Used = mathutil::min(DomStack::Top_Abstract_Level, Top_Limit);
		
		UGSAv5(DomStack& pStack, Json const& jConfig) :
			mStatsManager(),
			mAlgo(pStack, jConfig, mStatsManager)
		{}
		
		
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
		UGSAv5_Base<DomStack, Top_Abt_Level_Used, StatsManager> mAlgo;
		
	};
	
	template<typename DomStack>
	using UGSAv5_StatsSimple = UGSAv5<DomStack, SimpleStatsManager<>, 1000>;
}}}
