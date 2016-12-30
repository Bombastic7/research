#pragma once

#include <string>

#include "util/debug.hpp"
#include "util/json.hpp"
#include "search/solution.hpp"

#include "search/ugsa/v4/behaviour_b.hpp"
#include "search/ugsa/v4/base_search.hpp"
//#include "search/ugsa/v4/level_stats.hpp"

#include "search/ugsa/v4/null_stats.hpp"

namespace mjon661 { namespace algorithm { namespace ugsav4 {
	
	
	
	template<typename DomStack, typename StatsManager>
	struct UGSAv4 {
		
		using BaseDomain = typename DomStack::template Domain<0>;
		using State = typename BaseDomain::State;
		
		
		UGSAv4(DomStack& pStack, Json const& jConfig) :
			mBehaviour(jConfig),
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


		UGSABehaviour<> mBehaviour;
		StatsManager mStatsManager;
		UGSAv4_Base<DomStack, DomStack::Top_Abstract_Level, StatsManager> mAlgo;
		
	};
	
	
	template<typename D>
	using UGSAv4_StatsLevel = UGSAv4<D, NullStatsManager<>>;
	
}}}
