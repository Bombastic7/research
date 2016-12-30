#pragma once

#include <string>

#include "util/debug.hpp"
#include "util/json.hpp"
#include "search/solution.hpp"

#include "search/ugsa/v3/behaviour.hpp"
#include "search/ugsa/v3/base_search.hpp"
#include "search/ugsa/v3/level_stats.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav3 {	
	
	
	template<typename DomStack, typename StatsManager>
	struct UGSAv3 {
		
		using BaseDomain = typename DomStack::template Domain<0>;
		using State = typename BaseDomain::State;
		
		
		UGSAv3(DomStack& pStack, Json const& jConfig) :
			mBehaviour(jConfig.at("wf"), jConfig.at("wt")),
			mStatsManager(),
			mAlgo(pStack, mBehaviour, mStatsManager)
		{
			
		}
		
		
		void execute(Solution<BaseDomain>& pSol) {
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


		UGSABehaviour<DomStack::Top_Abstract_Level+1> mBehaviour;
		StatsManager mStatsManager;
		UGSAv3_Base<DomStack, DomStack::Top_Abstract_Level, StatsManager> mAlgo;
		
	};
	
	
	template<typename D>
	using UGSAv3_StatsLevel = UGSAv3<D, LevelStatsManager<>>;
	
}}}
