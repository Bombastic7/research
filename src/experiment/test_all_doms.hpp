


#inlcude "util/json.hpp"
#include "util/exception.hpp"

#include "experiment/common.hpp"

#include "search/astar.hpp"


#define ENABLE_GRIDNAV
#define ENABLE_TILES
#define ENABLE_PANCAKE


namespace mjon661 {


	template<typename Alg_D_t>
	void run_routine(D& pDomStack, Json& const& jAlgConfig, Json& jRes, std::vector<std::string> const& pKey) {
		
		Json& jLeaf = jRes;
		
		for(auto const& s : pKey)
			jLeaf = jLeaf[s];
		
		Alg_D_t alg(pDomStack, jAlgConfig);
		
		try {
			alg.execute(pDomStack.getInitState());
			jLeaf = alg.report();
		} catch(NoSolutionException const& e) {
			jLeaf = {{"failed", e.what()}};
		}
	}

	template<template<typename> typename Alg_t>
	void select_domain(Json const& jAlgConfig, Json& jRes, std::vector<std::string>& pKey) {
		
		#ifdef ENABLE_TILES
		
		{
			using D = tiles::TilesGeneric_DomainStack<3,3,true,false,1>;
		run_routine(
		
		#endif
		
		
	}

	template<typename D>
	void run_astar_family(D& pDomStack, Json& jRes, Json const& pSkipAlgs) {
		
		using Astar_t = algorithm::Astar<D, AstarSearchMode::Standard, AstarHrMode::AbtHr
		
		
		alg(pDomStack
	}



}


