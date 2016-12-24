#pragma once


#include "util/debug.hpp"
#include "util/json.hpp"
#include "search/solution.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav3 {
	
	
	template<typename DomStack, typename StatsManager>
	struct UGSAv3 {
		
		using BaseDomain = typename DomStack::template Domain<0>;
		using State = typename BaseDomain::State;
		
		
		UGSAv3(DomStack& pStack, Json const& jConfig) :
			mConfig(jConfig.at("wf"), jConfig.at("wt")),
			mBehaviour(),
			mStatsManager(),
			mAlgo(pStack, mConfig, mBehaviour, mStatsManager)
		{
			
		}
		
		
		void execute(Solution<BaseDomain>& pSol) {
			
			mAlgo.doSearch(mInitState, pSol);
		}
		
	};
	
	
	UGSAConfig mConfig;
	UGSAConfig mBehaviour;
	StatsManager mStatsManager;

	UGSAv3_Base<DomStack, DomStack::Top_Abstract_Level, StatsManager> mAlgo;
	
}}}
