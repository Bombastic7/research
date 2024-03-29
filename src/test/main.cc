
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <utility>
#include <tuple>

#include "util/debug.hpp"
#include "util/json.hpp"
//#include "domain/gridnav/blocked/graph.hpp"
#include "domain/pancake/fwd.hpp"
#include "domain/tiles/fwd.hpp"
//#include "domain/gridnav/hypernav/hypernav_real.hpp"
//#include "domain/test_graph.hpp"

#include "search/bugsy.hpp"
#include "search/bugsy_abt_lin.hpp"
#include "search/bugsy_abt_exp1.hpp"
#include "search/astar2.hpp"

#include "domain/gridnav/dim2/gridnav2d.hpp"


namespace mjon661 {

	
	std::vector<int> korf_15tiles_100instances(unsigned i) {
		switch(i) {
			case 1:
				return {14,13,15,7,11,12,9,5,6,0,2,1,4,8,10,3}; //57
			case 2:
				return {13,5,4,10,9,12,8,14,2,3,7,1,0,15,11,6}; //55
			case 3:
				return {14,7,8,2,13,11,10,4,9,12,5,0,3,6,1,15}; //59
			case 4:
				return {5,12,10,7,15,11,14,0,8,2,1,13,3,4,9,6}; //56
			case 5:
				return {4,7,14,13,10,3,9,12,11,5,6,15,1,2,8,0}; //56
			case 6:
				return {14,7,1,9,12,3,6,15,8,11,2,5,10,0,4,13}; //52
			case 7:
				return {2,11,15,5,13,4,6,7,12,8,10,1,9,3,14,0}; //52
			case 8:
				return {12,11,15,3,8,0,4,2,6,13,9,5,14,1,10,7}; //50
			case 9:
				return {3,14,9,11,5,4,8,2,13,12,6,7,10,1,15,0}; //46
			case 10:
				return {13,11,8,9,0,15,7,10,4,3,6,14,5,12,2,1}; //59
			default:
				gen_assert(false);
		}
		
		return {};
	}
	
	
	
	
	
	//Insert results of Alg_t run on pDomStack into jRes_nonutil[pAlgName], and inserts pseudo-entries for 
	//	each weight in pWeights into jRes_util[weight][pAlgName].
	template<typename D, typename Alg_t>
	void run_nonutil_search_fixedexptime(	D& pDomStack,
											Json const& jAlgConfig, 
											Json& jRes_nonutil,
											Json& jRes_util,
											std::string pAlgName,
											std::vector<std::tuple<double,double,std::string>> pWeights,
											double pFixedExpTime)
	{
		
		Timer searchTimer;
		Alg_t alg(pDomStack, jAlgConfig);
		
		try {
			searchTimer.start();
			alg.execute(pDomStack.getInitState());
			searchTimer.stop();
			
			jRes_nonutil[pAlgName] = alg.report();
			jRes_nonutil[pAlgName]["walltime"] = searchTimer.wallSeconds();
			jRes_nonutil[pAlgName]["cputime"] = searchTimer.cpuSeconds();
			
			for(auto& weight : pWeights) {
				std::string pseudoAlgName = pAlgName;
				jRes_util[std::get<2>(weight)][pseudoAlgName] = jRes_nonutil[pAlgName];
				
				Json& jEntryRef = jRes_util[std::get<2>(weight)][pseudoAlgName];
				
				jEntryRef["utility"] = 
					std::get<0>(weight) * jEntryRef["goal_g"].get<double>() + 
					std::get<1>(weight) * jEntryRef["expd"].get<double>() * pFixedExpTime;
			}
		} 
		catch(NoSolutionException const& e) {
			jRes_nonutil[pAlgName]["failed"] = e.what();
		}	
	}
	
	
	
	//Insert results of Alg_t run on pDomStack, for each weight in pWeights, into jRes[weight][pAlgName].
	//jAlgConfig is used as base of actual algorithm config; a copy is made and wf/wt added from pWeights.
	template<typename D, typename Alg_t>
	void run_util_search_fixedexptime(	D& pDomStack,
										Json const& jAlgConfig_tmpl, 
										Json& jRes,
										std::string const& pAlgName,
										std::vector<std::tuple<double,double,std::string>> pWeights,
										double pFixedExpTime)
	{		
		Timer searchTimer;
		
		Json jAlgConfig = jAlgConfig_tmpl;
		fast_assert(pFixedExpTime == jAlgConfig_tmpl.at("fixed_exptime").get<double>());
		
		for(auto& weight : pWeights) {			
			jRes[std::get<2>(weight)][pAlgName] = Json();
			Json& jEntryRef = jRes[std::get<2>(weight)][pAlgName];
			
			try {
				jAlgConfig["wf"] = std::get<0>(weight);
				jAlgConfig["wt"] = std::get<1>(weight);
				
				Alg_t alg(pDomStack, jAlgConfig);
				
				searchTimer.start();
				alg.execute(pDomStack.getInitState());
				searchTimer.stop();
				
				jEntryRef = alg.report();
				
				jEntryRef["walltime"] = searchTimer.wallSeconds();
				jEntryRef["cputime"] = searchTimer.cpuSeconds();

				jEntryRef["utility"] = 
					std::get<0>(weight) * jEntryRef["goal_g"].get<double>() + 
					std::get<1>(weight) * jEntryRef["expd"].get<double>() * pFixedExpTime;
			}
			catch(NoSolutionException const& e) {
				jEntryRef["failed"] = e.what();
			}	
		}
	}
	
	
	
	
	
	
	template<typename D>
	void run_bugsy_rollingbf_allOptions(D& pDomStack,
										Json const& jAlgConfig_tmpl, 
										Json& jRes,
										std::string const& pAlgName,
										std::vector<std::tuple<double,double,std::string>> pWeights,
										double pFixedExpTime)
	{
	
	using namespace algorithm::bugsy;
	
	#include "bugsy_rollingbf_options.inc"

	}
	
		//~ run_util_search_fixedexptime<D, algorithm::bugsy::BugsyAbtSearchBase<D>>(
			//~ pDomStack, jAlgConfig, jRes.at(probKey).at("util"), "bugsy_delayabt", weights, fixedExpTime);
	
	
	
	
	/*
	 * Generates the following data for the 15puzzle.
	 * 
	 * 	<problem>:
	 * 		"nonutil":
	 * 			<nonutil alg>:
	 * 				goal_g, goal_depth, expd, walltime, cputime... OR "failed"
	 * 
	 * 		"util":
	 * 			<weight>:
	 * 				<util alg>:
	 * 					goal_g, goal_depth, expd, walltime, cputime... OR "failed"
	 * 				
	 *	
	 * 	Non-utility-cognizant algorithms are given a pseudo-entry in "util"/weight branch with appropriate utility calculated.
	 */
 	
 	template<typename D>
	static Json run_searches_fixedexptime(D& pDomStack) {
		const double fixedExpTime = 3e-6;
		
		Json jAlgConfig, jRes;
		
		jAlgConfig["fixed_exptime"] = fixedExpTime;
		
		std::vector<std::tuple<double,double,std::string>> weights = {
			std::tuple<double,double,std::string>{1,1e6,"1e6"},
			std::tuple<double,double,std::string>{1,1e3, "1e3"},
			std::tuple<double,double,std::string>{1,1,"1"}
		};
		

		jRes["nonutil"] = Json();
		jRes["util"] = Json();
		
		run_nonutil_search_fixedexptime<D, algorithm::Astar2Impl<D, algorithm::Astar2SearchMode::Standard, algorithm::Astar2HrMode::DomainHr>>(
			pDomStack, jAlgConfig, jRes.at("nonutil"), jRes.at("util"), "astar", weights, fixedExpTime);
		
		run_nonutil_search_fixedexptime<D, algorithm::Astar2Impl<D, algorithm::Astar2SearchMode::Speedy, algorithm::Astar2HrMode::DomainHr>>(
			pDomStack, jAlgConfig, jRes.at("nonutil"), jRes.at("util"), "speedy", weights, fixedExpTime);
		
		run_nonutil_search_fixedexptime<D, algorithm::Astar2Impl<D, algorithm::Astar2SearchMode::Greedy, algorithm::Astar2HrMode::DomainHr>>(
			pDomStack, jAlgConfig, jRes.at("nonutil"), jRes.at("util"), "greedy", weights, fixedExpTime);
		
		const unsigned expdLimit = jRes.at("nonutil").at("astar").at("expd").get<unsigned>() + 100;
		jAlgConfig["expd_limit"] = expdLimit;
		
		run_bugsy_rollingbf_allOptions(pDomStack, jAlgConfig, jRes.at("util"), "bugsy_rollingbf", weights, fixedExpTime);

		return jRes;
	}
	




	template<bool Use_Weight>
	Json run_simple_tiles_33() {
		using D = tiles::TilesGeneric_DomainStack<3,3,true,Use_Weight,1>;
		
		Json jDomConfig, jRes;
		
		jDomConfig["goal"] = std::vector<unsigned>{0,1,2,3,4,5,6,7,8};
		jDomConfig["kept"] = std::vector<unsigned>{  1,1,1,1,1,1,1,1};
		
		D domStack(jDomConfig, Json());
		
		for(unsigned i=1; i<=10; i++) {
			domStack.setInitState(domStack.randInitState(i));
			jRes[std::to_string(i)] = run_searches_fixedexptime(domStack);
		}
		return jRes;
	}
	

	template<bool Use_Weight>
	Json run_simple_tiles_34() {
		using D = tiles::TilesGeneric_DomainStack<3,3,true,Use_Weight,1>;
		
		Json jDomConfig, jRes;
		
		jDomConfig["goal"] = std::vector<unsigned>{0,1,2,3,4,5,6,7,8,9,10,11};
		jDomConfig["kept"] = std::vector<unsigned>{  1,1,1,1,1,1,1,1,1, 1, 1};
		
		D domStack(jDomConfig, Json());
		
		for(unsigned i=1; i<=10; i++) {
			domStack.setInitState(domStack.randInitState(i));
			jRes[std::to_string(i)] = run_searches_fixedexptime(domStack);
		}
		return jRes;
	}
	
	template<bool Use_Weight>
	Json run_simple_tiles_44() {
		using D = tiles::TilesGeneric_DomainStack<3,3,true,Use_Weight,1>;
		
		Json jDomConfig, jRes;
		
		jDomConfig["goal"] = std::vector<unsigned>{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
		jDomConfig["kept"] = std::vector<unsigned>{  1,1,1,1,1,1,1,1,1, 1, 1, 1, 1, 1, 1};
		
		D domStack(jDomConfig, Json());
		
		for(unsigned i=1; i<=10; i++) {
			tiles::BoardState<4,4> initState(korf_15tiles_100instances(i));
			domStack.setInitState(initState);
			jRes[std::to_string(i)] = run_searches_fixedexptime(domStack);
		}
		return jRes;
	}
	
	
	template<unsigned N, bool Use_Weight>
	Json run_simple_pancake() {
		using D = pancake::Pancake_DomainStack_IgnoreAbt<N, N/2, 1, true, Use_Weight>;
		
		Json jRes;
		
		D domStack(Json(), Json());
		
		for(unsigned i=1; i<=10; i++) {
			domStack.setInitState(domStack.randInitState());
			jRes[std::to_string(i)] = run_searches_fixedexptime(domStack);
		}
		return jRes;
	}
	
	
	template<unsigned Nways>
	static void gridnav_getAbtStackSize(unsigned pHeight, unsigned pWidth, std::string const& pMapStr, unsigned pAbtRadius) {
		using CG = gridnav::dim2::CellGraph<Nways, false, false>;
		
		CG cellgraph(pHeight, pWidth, pMapStr);
		gridnav::dim2::StarAbt_Stack<CG> stackInfo(cellgraph, pAbtRadius);
		
		std::cout << stackInfo.report().dump(2) << "\n";		
	}
	
	//~ template<unsigned H, unsigned W, bool Use_LifeCost>
	//~ Json run_simple_gridnav_4(unsigned pMapSeed) {
		//~ using CG_u = gridnav::dim2::CellGraph<4, false, true>;
		//~ using D_u = gridnav::dim2::GridNav_StarAbtStack<
		
	//~ }
	
	
	
	
}

int main(int argc, const char* argv[]) {
	//mjon661::gridnav_getAbtStackSize<4>(100,100,",random,0.35",2);
	//std::cout << mjon661::run_tiles_44().dump(4) << "\n";
	
	
}






