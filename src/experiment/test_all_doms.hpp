
#include <iostream>

#include <string>
#include <vector>

#include "util/json.hpp"
#include "util/exception.hpp"

#include "experiment/common.hpp"
#include "experiment/tiles_problems.hpp"
#include "experiment/gridnav_problems.hpp"

#include "search/astar.hpp"
#include "search/bugsy.hpp"
#include "search/bugsy_abt_lin.hpp"
#include "search/bugsy_hardbf.hpp"

#include "domain/tiles/fwd.hpp"
#include "domain/gridnav/dim2/fwd.hpp"


#define ENABLE_GRIDNAV
//#define ENABLE_TILES
//#define ENABLE_PANCAKE

//~ #define ENABLE_ASTAR
//~ #define ENABLE_GREEDY
//~ #define ENABLE_SPEEDY
//~ #define ENABLE_BUGSY_DOMHR_FIXED
//~ #define ENABLE_BUGSY_ABTLIN_FIXED
#define ENABLE_BUGSY_HARDBF_FIXED


namespace mjon661 { namespace experiment {


	template<typename = void>
	Json& getJsonLeaf(Json& j, unsigned pos, std::vector<std::string> const& pKey) {
		if(pos == pKey.size())
			return j;
		std::string k = pKey[pos];
		return getJsonLeaf(j[k], pos+1, pKey);
	}
	
	//~ template<typename = void>
	//~ bool jsonKeyExists(Json& j, unsigned pos, std::vector<std::string> const& pKey) {
		//~ if(pos == pKey.size())
			//~ return true;
		//~ if(j.count(pKey[pos]))
			//~ return jsonKeyExists(j.at(pKey[pos]), pos+1, pKey);
		//~ return false;
	//~ }


	template<typename D, template<typename> typename Alg_t>
	void run_routine(D& pDomStack, Json const& jAlgConfig, Json& jRes, std::vector<std::string> const& pKey) {
		
		for(auto const& s : pKey) {
			std::cout << s << " ";
		}
		
		//~ bool skip = false;
		//~ if(std::find(pSkipKeys.begin(), pSkipKeys.end(), pKey) != pSkipKeys.end()) {
			//~ std::cout << "skipped";
			//~ skip = true;
		//~ }
		
		std::cout << "\n";
		
		//~ if(skip)
			//~ return;
		
		Json& jLeaf = getJsonLeaf(jRes, 0, pKey);
		
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
		
		// tiles8, tiles8w
		{
			using D = tiles::TilesGeneric_DomainStack<3,3,true,false,5>;
			pKey.push_back("tiles8");
			
			Json jDomConfig;
			jDomConfig["goal"] = tiles::tiles_defgoal(9);
			jDomConfig["kept"] = tiles::tiles_abtfirst5(8);
				
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
			using D = tiles::TilesGeneric_DomainStack<3,3,true,true,5>;
			pKey.push_back("tiles8w");
			
			Json jDomConfig;
			jDomConfig["goal"] = tiles::tiles_defgoal(9);
			jDomConfig["kept"] = tiles::tiles_abtfirst5(8);
				
			for(unsigned i=0; i<5; i++) {
				jDomConfig["init"] = tiles::tiles8_instances(i);
				D domStack(jDomConfig);
				pKey.push_back(std::to_string(i));
				run_routine<D, Alg_t>(domStack, jAlgConfig, jRes, pKey);
				pKey.pop_back();
			}
			
			pKey.pop_back();
		}
		
		//tiles15, tiles15w
		{
			using D = tiles::TilesGeneric_DomainStack<4,4,true,false,7>;
			pKey.push_back("tiles15");
			
			Json jDomConfig;
			jDomConfig["goal"] = tiles::tiles_defgoal(16);
			jDomConfig["kept"] = tiles::tiles_abtfirst7(15);
				
			for(unsigned i=0; i<5; i++) {
				jDomConfig["init"] = tiles::tiles15_instances_korf(i);
				D domStack(jDomConfig);
				pKey.push_back(std::to_string(i));
				run_routine<D, Alg_t>(domStack, jAlgConfig, jRes, pKey);
				pKey.pop_back();
			}
			
			pKey.pop_back();
		}
		
		{
			using D = tiles::TilesGeneric_DomainStack<4,4,true,true,7>;
			pKey.push_back("tiles15w");
			
			Json jDomConfig;
			jDomConfig["goal"] = tiles::tiles_defgoal(16);
			jDomConfig["kept"] = tiles::tiles_abtfirst7(15);
				
			for(unsigned i=0; i<5; i++) {
				jDomConfig["init"] = tiles::tiles15_instances_korf(i);
				D domStack(jDomConfig);
				pKey.push_back(std::to_string(i));
				run_routine<D, Alg_t>(domStack, jAlgConfig, jRes, pKey);
				pKey.pop_back();
			}
			
			pKey.pop_back();
		}
		#endif
		
		#ifdef ENABLE_GRIDNAV
		{
			gridnav::dim2::CellMap2D<> cellmap_1k_0_35;
			cellmap_1k_0_35.setRandom(1000,1000,0,0.35);
		

			//gridnav_4way_unit_1k_0_35_abt2
			{
				using BaseDomain_t = gridnav::dim2::fourway::BaseDomain<false, true>; //4way, unit cost.
				using D = gridnav::dim2::DomainStack_StarAbt<BaseDomain_t, gridnav::dim2::topabtlevel_2_cellmap_1k_0_35>;
				
				gridnav::dim2::StarAbtInfo<BaseDomain_t::Cost> abtinfo(2);
				abtinfo.init<gridnav::dim2::fourway::BaseEdgeIterator<false>>(cellmap_1k_0_35);

				D domStack(cellmap_1k_0_35, abtinfo, Json());
				
				pKey.push_back("gridnav_4way_unit_1k_0_35_abt2");
				
				for(unsigned i=0; i<5; i++) {
					auto sp = gridnav::dim2::instances_cellmap_1k_0_35(i);
					domStack.setInitAndGoal(sp.first, sp.second);
					pKey.push_back(std::to_string(i));
					run_routine<D,Alg_t>(domStack, jAlgConfig, jRes, pKey);
					pKey.pop_back();
				}
				
				pKey.pop_back();
			}


			//gridnav_4way_life_1k_0_35_abt2
			{
				using BaseDomain_t = gridnav::dim2::fourway::BaseDomain<true, true>; //4way, life cost.
				using D = gridnav::dim2::DomainStack_StarAbt<BaseDomain_t, gridnav::dim2::topabtlevel_2_cellmap_1k_0_35>;
				
				gridnav::dim2::StarAbtInfo<BaseDomain_t::Cost> abtinfo(2);
				abtinfo.init<gridnav::dim2::fourway::BaseEdgeIterator<true>>(cellmap_1k_0_35);				
				
				D domStack(cellmap_1k_0_35, abtinfo, Json());
				
				pKey.push_back("gridnav_4way_life_1k_0_35_abt2");
				
				for(unsigned i=0; i<5; i++) {
					auto sp = gridnav::dim2::instances_cellmap_1k_0_35(i);
					domStack.setInitAndGoal(sp.first, sp.second);
					pKey.push_back(std::to_string(i));
					run_routine<D,Alg_t>(domStack, jAlgConfig, jRes, pKey);
					pKey.pop_back();
				}
				
				pKey.pop_back();
			}
		}
		{
			gridnav::dim2::CellMap2D<> cellmap_2k_0_35;
			cellmap_2k_0_35.setRandom(2000,2000,0,0.35);
			
			//gridnav_4way_unit_2k_0_35_abt2
			{
				using BaseDomain_t = gridnav::dim2::fourway::BaseDomain<false, true>; //4way, unit cost.
				using D = gridnav::dim2::DomainStack_StarAbt<BaseDomain_t, gridnav::dim2::topabtlevel_2_cellmap_2k_0_35>;
				
				gridnav::dim2::StarAbtInfo<BaseDomain_t::Cost> abtinfo(2);
				abtinfo.init<gridnav::dim2::fourway::BaseEdgeIterator<false>>(cellmap_2k_0_35);

				D domStack(cellmap_2k_0_35, abtinfo, Json());
				
				pKey.push_back("gridnav_4way_unit_2k_0_35_abt2");
				
				for(unsigned i=0; i<5; i++) {
					auto sp = gridnav::dim2::instances_cellmap_2k_0_35(i);
					domStack.setInitAndGoal(sp.first, sp.second);
					pKey.push_back(std::to_string(i));
					run_routine<D,Alg_t>(domStack, jAlgConfig, jRes, pKey);
					pKey.pop_back();
				}
				
				pKey.pop_back();
			}


			//gridnav_4way_life_2k_0_35_abt2
			{
				using BaseDomain_t = gridnav::dim2::fourway::BaseDomain<true, true>; //4way, life cost.
				using D = gridnav::dim2::DomainStack_StarAbt<BaseDomain_t, gridnav::dim2::topabtlevel_2_cellmap_2k_0_35>;
				
				gridnav::dim2::StarAbtInfo<BaseDomain_t::Cost> abtinfo(2);
				abtinfo.init<gridnav::dim2::fourway::BaseEdgeIterator<true>>(cellmap_2k_0_35);				
				
				D domStack(cellmap_2k_0_35, abtinfo, Json());
				
				pKey.push_back("gridnav_4way_life_2k_0_35_abt2");
				
				for(unsigned i=0; i<5; i++) {
					auto sp = gridnav::dim2::instances_cellmap_2k_0_35(i);
					domStack.setInitAndGoal(sp.first, sp.second);
					pKey.push_back(std::to_string(i));
					run_routine<D,Alg_t>(domStack, jAlgConfig, jRes, pKey);
					pKey.pop_back();
				}
				
				pKey.pop_back();
			}
		}
		
		
		
		
		
		#endif
	}
	
	struct SearchAlgTemplateTypes {
		template<typename D> using Astar_t = algorithm::Astar<D, algorithm::AstarSearchMode::Standard, algorithm::AstarHrMode::DomainHr>;
		template<typename D> using Greedy_t = algorithm::Astar<D, algorithm::AstarSearchMode::Greedy, algorithm::AstarHrMode::DomainHr>;
		template<typename D> using Speedy_t = algorithm::Astar<D, algorithm::AstarSearchMode::Speedy, algorithm::AstarHrMode::DomainHr>;
		template<typename D> using Bugsy_domhr_Fixed_t = algorithm::Bugsy<D, true>;
		template<typename D> using Bugsy_abtlin_Fixed_t = algorithm::BugsyAbtLin_baseSearch<D, true>;
		template<typename D> using Bugsy_hardbf_Fixed_t = algorithm::BugsyHardBF<D>;
	};
	
	
	template<typename = void>
	void select_alg_weight(Json& jRes) {
		
		std::vector<UtilityWeights> weights = {UtilityWeights(1,1,"1~1"), UtilityWeights(1,1e3,"1~1e3"), UtilityWeights(1,1e6,"1~1e6")};
		
		#ifdef ENABLE_ASTAR
		{
			std::vector<std::string> key {"astar", "nullweight"};
			select_domain_prob<SearchAlgTemplateTypes::Astar_t>(Json(), jRes, key);
		}
		#endif
		#ifdef ENABLE_GREEDY
		{
			std::vector<std::string> key {"greedy", "nullweight"};
			select_domain_prob<SearchAlgTemplateTypes::Greedy_t>(Json(), jRes, key);
		}
		#endif
		#ifdef ENABLE_SPEEDY
		{
			std::vector<std::string> key {"speedy", "nullweight"};
			select_domain_prob<SearchAlgTemplateTypes::Speedy_t>(Json(), jRes, key);
		}
		#endif
		
		for(auto const& weight : weights) {
			Json jAlgConfig;
			jAlgConfig["wf"] = weight.wf;
			jAlgConfig["wt"] = weight.wt;
			jAlgConfig["exptime"] = 3e-6;
			
			#ifdef ENABLE_BUGSY_DOMHR_FIXED
			{
				std::vector<std::string> key {"bugsy_domhr_fixed", weight.str};
				select_domain_prob<SearchAlgTemplateTypes::Bugsy_domhr_Fixed_t>(jAlgConfig, jRes, key);
			}
			#endif
			#ifdef ENABLE_BUGSY_ABTLIN_FIXED
			{
				std::vector<std::string> key {"bugsy_abtlin_fixed", weight.str};
				select_domain_prob<SearchAlgTemplateTypes::Bugsy_abtlin_Fixed_t>(jAlgConfig, jRes, key);
			}
			#endif
			#ifdef ENABLE_BUGSY_HARDBF_FIXED
			{
				jAlgConfig["bf"] = 1.01;
				std::vector<std::string> key {"bugsy_hardbf_fixed", weight.str};
				select_domain_prob<SearchAlgTemplateTypes::Bugsy_hardbf_Fixed_t>(jAlgConfig, jRes, key);
				jAlgConfig.at("bf") = {};
			}
			#endif
		}
	}

}}


