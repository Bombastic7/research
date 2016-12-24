#pragma once


#include "util/debug.hpp"
#include "util/json.hpp"
#include "search/solution.hpp"

#include "search/ugsa/v3/common.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav3 {
	
	
	template<typename = void>
	struct TestStatsManager {
		
		void expd() {}
		void gend() {}
		void dups() {}
		void reopnd() {}
		
	};
	
	
	
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
			
			mAlgo.doSearch(mInitState, pSol);
		}


		UGSAConfig mBehaviour;
		StatsManager mStatsManager;

		UGSAv3_Base<DomStack, DomStack::Top_Abstract_Level, StatsManager> mAlgo;
		
	};
	
	
	template<typename D>
	using UGSAv3_Test = UGSAv3<D, TestStatsManager<>>;
	
}}}
