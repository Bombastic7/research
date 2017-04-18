
#include <iostream>

#include "util/json.hpp"
#include "util/exception.hpp"

#include "experiment/common.hpp"
#include "experiment/tiles_problems.hpp"

#include "search/astar.hpp"

#include "domain/tiles/fwd.hpp"


#define ENABLE_GRIDNAV
#define ENABLE_TILES
#define ENABLE_PANCAKE


namespace mjon661 {


	template<typename D, template<typename> typename Alg_t>
	void run_routine(D& pDomStack, Json const& jAlgConfig, Json& jRes, std::vector<std::string> const& pKey) {
		
		Json& jLeaf = jRes;
		
		for(auto const& s : pKey) {
			std::cout << s << " ";
			jLeaf = jLeaf[s];
		}
		std::cout << "\n";
		
		Alg_t<D> alg(pDomStack, jAlgConfig);
		
		try {
			alg.execute(pDomStack.getInitState());
			jLeaf = alg.report();
		} catch(NoSolutionException const& e) {
			jLeaf = {{"failed", e.what()}};
		}
	}

	template<template<typename> typename Alg_t>
	void select_domain_prob(Json const& jAlgConfig, Json& jRes, std::vector<std::string>& pKey) {
		
		#ifdef ENABLE_TILES
		
		{
			using D = tiles::TilesGeneric_DomainStack<3,3,true,false,1>;
			pKey.push_back("tiles8");
			
			Json jDomConfig;
			jDomConfig["goal"] = tiles::tiles_defgoal(9);
			jDomConfig["kept"] = tiles::tiles_abtnullkept(9);
				
			for(unsigned i=0; i<5; i++) {
				jDomConfig["init"] = tiles::tiles8_instances(i);
				D domStack(jDomConfig);
				pKey.push_back(std::to_string(i));
				run_routine<D, Alg_t>(domStack, jAlgConfig, jRes, pKey);
				pKey.pop_back();
			}
			
			pKey.pop_back();
		}
		
		{
			using D = tiles::TilesGeneric_DomainStack<3,3,true,true,1>;
			pKey.push_back("tiles8w");
			
			Json jDomConfig;
			jDomConfig["goal"] = tiles::tiles_defgoal(9);
			jDomConfig["kept"] = tiles::tiles_abtnullkept(9);
				
			for(unsigned i=0; i<5; i++) {
				jDomConfig["init"] = tiles::tiles8_instances(i);
				D domStack(jDomConfig);
				pKey.push_back(std::to_string(i));
				run_routine<D, Alg_t>(domStack, jAlgConfig, jRes, pKey);
				pKey.pop_back();
			}
			
			pKey.pop_back();
		}
		
		#endif		
	}
	
	struct SearchAlgTemplateTypes {
		template<typename D> using Astar_t = algorithm::Astar<D, algorithm::AstarSearchMode::Standard, algorithm::AstarHrMode::DomainHr>;
		template<typename D> using Greedy_t = algorithm::Astar<D, algorithm::AstarSearchMode::Greedy, algorithm::AstarHrMode::DomainHr>;
		template<typename D> using Speedy_t = algorithm::Astar<D, algorithm::AstarSearchMode::Speedy, algorithm::AstarHrMode::DomainHr>;
	};
	
	
	template<typename = void>
	void select_alg_weight(Json& jRes) {
		
		std::vector<UtilityWeights> weights = {UtilityWeights(1,1,"1~1"), UtilityWeights(1,1e3,"1~1e3"), UtilityWeights(1,1e6,"1~1e6")};
		
		{
			std::vector<std::string> key {"astar", "nullweight"};
			select_domain_prob<SearchAlgTemplateTypes::Astar_t>(Json(), jRes, key);
		}
		
		{
			std::vector<std::string> key {"greedy", "nullweight"};
			select_domain_prob<SearchAlgTemplateTypes::Greedy_t>(Json(), jRes, key);
		}
		
		{
			std::vector<std::string> key {"speedy", "nullweight"};
			select_domain_prob<SearchAlgTemplateTypes::Speedy_t>(Json(), jRes, key);
		}
	}
}


