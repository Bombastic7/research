#pragma once

#include "util/debug.hpp"
#include "util/json.hpp"

#include "search/astar2.hpp"



namespace mjon661 {




	template<typename D, algorithm::Astar2SearchMode Search_Mode, algorithm::Astar2HrMode Hr_Mode>
	void run_noncog_search(D& pDomStack, std::string const& pAlgName, Json& j, Json const& jAlgConfig = Json()) {
		using Alg_t = algorithm::Astar2Impl<D,Search_Mode, Hr_Mode>;
		
		fast_assert(j.count(pAlgName) == 0);
		
		Alg_t alg(pDomStack, Json());
		
		try {
			alg.execute(pDomStack.getInitState());
			j[pAlgName] = alg.report();
		} catch(NoSolutionException const& e) {
			j[pAlgName] = {{"failed", e.what()}};
		}
	}
	
	
	
	template<typename D>
	void run_astar(D& pDomStack, Json& j) {
		run_noncog_search<D, algorithm::Astar2SearchMode::Standard, algorithm::Astar2HrMode::DomainHr>(pDomStack, "astar", j);
	}
	
	template<typename D>
	void run_speedy(D& pDomStack, Json& j) {
		run_noncog_search<D, algorithm::Astar2SearchMode::Speedy, algorithm::Astar2HrMode::DomainHr>(pDomStack, "speedy", j);
	}
	
	template<typename D>
	void run_greedy(D& pDomStack, Json& j) {
		run_noncog_search<D, algorithm::Astar2SearchMode::Greedy, algorithm::Astar2HrMode::DomainHr>(pDomStack, "greedy", j);
	}
	
	template<typename D>
	void run_wastar(D& pDomStack, double pWeight, Json& j) {
		run_noncog_search<D, algorithm::Astar2SearchMode::Weighted, algorithm::Astar2HrMode::DomainHr>(
			pDomStack, "wastar", j, {{"weight",pWeight}});
	}
}
