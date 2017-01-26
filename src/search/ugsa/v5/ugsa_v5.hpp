#pragma once

#include <string>

#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/math.hpp"
#include "search/solution.hpp"

#include "search/ugsa/v5/common.hpp"
#include "search/ugsa/v5/base_search.hpp"
#include "search/ugsa/v5/simple_stats.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav5 {
	
	template<typename DomStack, HeuristicModes H_Mode, UCalcMode U_Mode, typename StatsManager, unsigned Top_Limit>
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
		UGSAv5_Base<DomStack, Top_Abt_Level_Used, H_Mode, U_Mode, StatsManager> mAlgo;
		
	};
	
	template<typename DomStack>
	using UGSAv5_Cost_Delay = UGSAv5<DomStack, HeuristicModes::Min_Cost, UCalcMode::Delay, SimpleStatsManager<>, 1000>;
	
	template<typename DomStack>
	using UGSAv5_Cost_HBF = UGSAv5<DomStack, HeuristicModes::Min_Cost, UCalcMode::HBF, SimpleStatsManager<>, 1000>;
	
	template<typename DomStack>
	using UGSAv5_Cost_CostOnly = UGSAv5<DomStack, HeuristicModes::Min_Cost, UCalcMode::CostOnly, SimpleStatsManager<>, 1000>;
	
	//~ template<typename DomStack>
	//~ using UGSAv5_Dist = UGSAv5<DomStack, HeuristicModes::Min_Dist, SimpleStatsManager<>, 1000>;
	
	//~ template<typename DomStack>
	//~ using UGSAv5_CostOrDist = UGSAv5<DomStack, HeuristicModes::Min_Cost_Or_Dist, SimpleStatsManager<>, 1000>;
	
	//~ template<typename DomStack>
	//~ using UGSAv5_CostAndDist = UGSAv5<DomStack, HeuristicModes::Min_Cost_And_Dist, SimpleStatsManager<>, 1000>;
}}}
