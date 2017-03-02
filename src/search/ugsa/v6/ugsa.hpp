#pragma once

namespace mjon661 { namespace algorithm { namespace ugsav6 {
	
	
	
	template<typename DomStack>
	class UGSAv6 {
		
		
		using BaseDomain = typename DomStack::template Domain<0>;
		using State = typename BaseDomain::State;
		
		
		UGSAv6(DomStack& pStack, Json const& jConfig) :
			mStatsManager(),
			mAlgo(pStack, jConfig, mStatsManager)
		{}
		
		
		void execute(State const& s0, Solution<DomStack>& pSol) {
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
		UGSAv6_Base<DomStack> mAlgo;
	};
	
}}}
