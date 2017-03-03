#pragma once

#include "search/solution.hpp"
#include "util/json.hpp"

#include "search/ugsa/v6/base_search.hpp"

namespace mjon661 { namespace algorithm { namespace ugsav6 {
	
	
	
	template<typename DomStack>
	struct UGSAv6 {
		
		
		using BaseDomain = typename DomStack::template Domain<0>;
		using State = typename BaseDomain::State;
		
		
		UGSAv6(DomStack& pStack, Json const& jConfig) :
			mAlgo(pStack, jConfig)
		{}
		
		
		void execute(State const& s0, Solution<DomStack>& pSol) {
			mAlgo.doSearch(s0, pSol);
		}

		Json report() {
			return mAlgo.report();
		}
		
		void reset() {
			mAlgo.reset();
		}

		private:
		UGSAv6_Base<DomStack> mAlgo;
	};
	
}}}
