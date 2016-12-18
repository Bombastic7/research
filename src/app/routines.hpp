#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/time.hpp"


#include "domain/tiles/tiles_fwd.hpp"
#include "domain/pancake/pancake.hpp"
#include "domain/gridnav/fwd.hpp"

#include "search/astar.hpp"
#include "search/idastar.hpp"
#include "search/bugsy.hpp"
#include "search/hastar/generic/hastar.hpp"
#include "search/ugsa/v1/ugsa_v1.hpp"
#include "search/ugsa/v2/ugsa_v2.hpp"


/*
#define N_Tiles8_NoH
#define N_Tiles15_NoH
#define N_Tiles8_MD
#define N_Tiles15_MD
#define N_WTiles8_NoH
#define N_WTiles15_NoH

#define N_Pancake10
#define N_WPancake10
#define N_Pancake10_Abt5

#define N_GridNav_4Wsimple_10_10_Uni
#define N_GridNav_4Wsimple_10_10_LC
#define N_GridNav_8Wsimple_10_10_Uni
#define N_GridNav_8Wsimple_10_10_LC

#define N_Astar
#define N_IDAstar
#define N_Bugsy
#define N_HAstar
#define N_UGSAv1
#define N_UGSAv2


*/

#define N_Tiles8_NoH
#define N_Tiles15_NoH
#define N_Tiles8_MD
#define N_Tiles15_MD
#define N_WTiles8_NoH
#define N_WTiles15_NoH

#define N_Pancake10
#define N_WPancake10
#define N_Pancake10_Abt5

#define N_GridNav_4Wsimple_10_10_Uni
#define N_GridNav_4Wsimple_10_10_LC
//#define N_GridNav_8Wsimple_10_10_Uni
//#define N_GridNav_8Wsimple_10_10_LC

//#define N_Astar
#define N_IDAstar
#define N_Bugsy
#define N_HAstar
#define N_UGSAv1
#define N_UGSAv2


namespace mjon661 { namespace execution_single {
	
	
	using Tiles8_NoH = tiles::Tiles8_NoH;
	using Tiles8_MD = tiles::Tiles8_MD;
	using Tiles15_NoH = tiles::Tiles15_NoH;
	using Tiles15_MD = tiles::Tiles15_MD;
	
	using WTiles8_NoH = tiles::WTiles8_NoH;
	using WTiles15_NoH = tiles::WTiles15_NoH;
	
	using Pancake10 = pancake::Pancake10;
	using WPancake10 = pancake::WPancake10;
	using Pancake10_Abt5 = pancake::Pancake10_Abt5;
	
	using GridNav_4Wsimple_10_10_Uni = gridnav::GridNav_4Wsimple_10_10_Uni;
	using GridNav_4Wsimple_10_10_LC = gridnav::GridNav_4Wsimple_10_10_LC;
	using GridNav_8Wsimple_10_10_Uni = gridnav::GridNav_8Wsimple_10_10_Uni;
	using GridNav_8Wsimple_10_10_LC = gridnav::GridNav_8Wsimple_10_10_LC;
	
	template<typename D> using Astar = algorithm::Astar<D>;
	template<typename D> using IDAstar = algorithm::IDAstar<D>;
	template<typename D> using Bugsy = algorithm::Bugsy<D>;
	template<typename D> using UGSAv1 = algorithm::ugsav1::UGSAv1<D>;
	
	template<typename D> using HAstar_StatsNone = algorithm::hastargeneric::HAstar_StatsNone<D>;
	template<typename D> using HAstar_StatsLevel = algorithm::hastargeneric::HAstar_StatsLevel<D>;
	template<typename D> using HAstar_StatsSearch = algorithm::hastargeneric::HAstar_StatsSearch<D>;
	
	template<typename D> using UGSAv2_StatsNone = algorithm::ugsav2::UGSAv2_StatsNone<D>;
	template<typename D> using UGSAv2_StatsLevel = algorithm::ugsav2::UGSAv2_StatsLevel<D>;
	template<typename D> using UGSAv2_StatsSearch = algorithm::ugsav2::UGSAv2_StatsSearch<D>;
	
	
	
	template<typename D>
	void domSpecial(D&, Solution<typename D::template Domain<0>> const& pSol) {}
	
	template<>
	void domSpecial<GridNav_8Wsimple_10_10_Uni>(GridNav_8Wsimple_10_10_Uni& pDom, 
												Solution<typename GridNav_8Wsimple_10_10_Uni::template Domain<0>> const& pSol) {
		
		typename GridNav_8Wsimple_10_10_Uni::template Domain<0>::Methods m(pDom);
		
		m.drawPath(pSol.states, std::cout);
	}
	
	template<>
	void domSpecial<GridNav_8Wsimple_10_10_LC>(GridNav_8Wsimple_10_10_LC& pDom, 
												Solution<typename GridNav_8Wsimple_10_10_LC::template Domain<0>> const& pSol) {
		

		typename GridNav_8Wsimple_10_10_LC::template Domain<0>::Methods m(pDom);
		m.drawPath(pSol.states, std::cout);
	}
	
	
	template<typename D, template<typename> typename Alg>
	void singleExecRoutine(Json const& jExecDesc) {

		using BaseDomain = typename D::template Domain<0>;
		
		D 		dom(jExecDesc.at("domain conf"));
		Alg<D> 	algo(dom, jExecDesc.at("algorithm conf"));
		
		Timer timer;
		Solution<BaseDomain> sol;
		
		timer.start();
		algo.execute(sol);
		timer.stop();
		
		Json jOut;
		jOut["algo report"] = algo.report();
		jOut["time"] = timer.seconds();
		
		Json jSolInfo;
		jSolInfo["length"] = sol.operators.size();
		jSolInfo["cost"] = sol.pathCost(dom);
		
		jOut["solution"] = jSolInfo;
		
		domSpecial<D>(dom, sol);
		
		if(jExecDesc.count("dump solution")) {
			
			if(jExecDesc["dump solution"] == "stdout")
				sol.printSolution(dom, std::cout);
			
			else {
				std::ofstream ofs(jExecDesc["dump solution"].get<std::string>());
				if(!ofs)
					logDebug(std::string("Could not open ") + jExecDesc["dump solution"].get<std::string>());
				else
					sol.printSolution(dom, ofs);
			}
		}
		
		std::cout << jOut.dump(4);
	}
	
	
	template<template<typename> typename Alg>
	void selectDomain_abt(Json const& jExecDesc) {
		std::string pDom = jExecDesc["domain"];
		
		if(pDom == "")
			throw std::runtime_error("Bad domain");

		#ifndef N_Tiles8_NoH
		else if(pDom == "Tiles8_NoH")
			singleExecRoutine<Tiles8_NoH, Alg>(jExecDesc);
		#endif
		
		#ifndef N_Tiles15_NoH
		else if(pDom == "Tiles15_NoH")
			singleExecRoutine<Tiles15_NoH, Alg>(jExecDesc);
		#endif
		
		#ifndef N_WTiles8_NoH
		else if(pDom == "WTiles8_NoH")
			singleExecRoutine<WTiles8_NoH, Alg>(jExecDesc);
		#endif
		
		#ifndef N_WTiles15_NoH
		else if(pDom == "WTiles15_NoH")
			singleExecRoutine<WTiles15_NoH, Alg>(jExecDesc);
		#endif
		
		#ifndef N_Pancake10_Abt5
		else if(pDom == "Pancake10_Abt5")
			singleExecRoutine<Pancake10_Abt5, Alg>(jExecDesc);
		#endif
		
		else
			throw std::runtime_error("Bad domain");
	}
	
	
	
	template<template<typename> typename Alg>
	void selectDomain_simple(Json const& jExecDesc) {
		std::string pDom = jExecDesc["domain"];
		
		if(pDom == "")
			throw std::runtime_error("Bad domain");
		
		#ifndef N_Tiles8_MD
		else if(pDom == "Tiles8_MD")
			singleExecRoutine<Tiles8_MD, Alg>(jExecDesc);
		#endif
		
		#ifndef N_Tiles15_MD
		else if(pDom == "Tiles15_MD")
			singleExecRoutine<Tiles15_MD, Alg>(jExecDesc);
		#endif
		
		#ifndef N_Pancake10
		else if(pDom == "Pancake10")
			singleExecRoutine<Pancake10, Alg>(jExecDesc);
		#endif
		
		#ifndef N_WPancake10
		else if(pDom == "WPancake10")
			singleExecRoutine<WPancake10, Alg>(jExecDesc);
		#endif
		
		#ifndef N_GridNav_4Wsimple_10_10_Uni
		else if(pDom == "GridNav_4Wsimple_10_10_Uni")
			singleExecRoutine<GridNav_4Wsimple_10_10_Uni, Alg>(jExecDesc);
		#endif
		
		#ifndef N_GridNav_4Wsimple_10_10_LC
		else if(pDom == "GridNav_4Wsimple_10_10_LC")
			singleExecRoutine<GridNav_4Wsimple_10_10_LC, Alg>(jExecDesc);
		#endif
		
		#ifndef N_GridNav_8Wsimple_10_10_Uni
		else if(pDom == "GridNav_8Wsimple_10_10_Uni")
			singleExecRoutine<GridNav_8Wsimple_10_10_Uni, Alg>(jExecDesc);
		#endif
		
		#ifndef N_GridNav_8Wsimple_10_10_LC
		else if(pDom == "GridNav_8Wsimple_10_10_LC")
			singleExecRoutine<GridNav_8Wsimple_10_10_LC, Alg>(jExecDesc);
		#endif
		
		else
			selectDomain_abt<Alg>(jExecDesc);
			
	}
	
	
	
	template<typename = void>
	void selectAlgorithm(Json const& jExecDesc) {
		std::string pAlg = jExecDesc["algorithm"];
		
		if(pAlg == "")
			throw std::runtime_error("Bad algorithm");
			
		#ifndef N_Astar
		if(pAlg == "Astar")
			selectDomain_simple<Astar>(jExecDesc);
		#endif
		
 		#ifndef N_IDAstar
 		else if(pAlg == "IDAstar")
			selectDomain_simple<IDAstar>(jExecDesc);
		#endif
		
		#ifndef N_Bugsy
 		else if(pAlg == "Bugsy")
			selectDomain_simple<Bugsy>(jExecDesc);
		#endif
		
 		#ifndef N_HAstar
 		else if(pAlg == "HAstar_StatsNone")
			selectDomain_abt<HAstar_StatsNone>(jExecDesc);
		else if(pAlg == "HAstar_StatsLevel")
			selectDomain_abt<HAstar_StatsLevel>(jExecDesc);
		else if(pAlg == "HAstar_StatsSearch")
			selectDomain_abt<HAstar_StatsSearch>(jExecDesc);
		#endif
		
		#ifndef N_UGSAv1
 		else if(pAlg == "UGSAv1")
			selectDomain_abt<UGSAv1>(jExecDesc);
		#endif
		
		#ifndef N_UGSAv2
		else if(pAlg == "UGSAv2_StatsNone")
			selectDomain_abt<UGSAv2_StatsNone>(jExecDesc);
		else if(pAlg == "UGSAv2_StatsLevel")
			selectDomain_abt<UGSAv2_StatsLevel>(jExecDesc);
		else if(pAlg == "UGSAv2_StatsSearch")
			selectDomain_abt<UGSAv2_StatsSearch>(jExecDesc);
		#endif
		
		else
			throw std::runtime_error("Bad algorithm");
		
	}

}}
