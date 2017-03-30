
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
//#include "domain/pancake/fwd.hpp"
#include "domain/tiles/fwd.hpp"
//#include "domain/gridnav/hypernav/hypernav_real.hpp"
//#include "domain/test_graph.hpp"

#include "search/bugsy.hpp"
#include "search/bugsy_abt_lin.hpp"
#include "search/astar2.hpp"



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
	
	
	
	
	
	//Insert results of Alg_t run on pDomStack into jRes_nonutil[pAlgName], 
	//	and also into jRes_util[weight][pAlgName_pseudo] with utility value added, for weight in pWeights.
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
		
		bool success = false;
			
		searchTimer.start();
		try {
			alg.execute(pDomStack.getInitState());
			searchTimer.stop();
			success = true;
		} catch(NoSolutionException const& e) {
			jRes_nonutil["failed"][pAlgName] = e.what();
		}
		
		if(success) {
			jRes_nonutil[pAlgName] = alg.report();
			jRes_nonutil[pAlgName]["walltime"] = searchTimer.seconds();
			
			std::string pseudoNameStr = pAlgName + "_pseudo";
			
			for(auto& w : pWeights) {				
				jRes_util[std::get<2>(w)][pseudoNameStr] = jRes_nonutil.at(pAlgName);
				jRes_util[std::get<2>(w)][pseudoNameStr]["utility"] = 
					jRes_util[std::get<2>(w)][pseudoNameStr].at("goal_g").get<double>() * std::get<0>(w)
					+
					jRes_util[std::get<2>(w)][pseudoNameStr].at("expd").get<double>() * std::get<1>(w) * pFixedExpTime;
			}
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
			jAlgConfig["wf"] = std::get<0>(weight);
			jAlgConfig["wt"] = std::get<1>(weight);
		
			Alg_t alg(pDomStack, jAlgConfig);
		
			bool success = false;
		
			try {
				searchTimer.start();
				alg.execute(pDomStack.getInitState());
				searchTimer.stop();
				success = true;
			} catch(NoSolutionException const& e) {
				jRes[std::get<2>(weight)]["failed"][pAlgName] = e.what();
			}
		
			if(success) {				
				jRes[std::get<2>(weight)][pAlgName] = alg.report();
				jRes[std::get<2>(weight)][pAlgName]["walltime"] = searchTimer.seconds();
				jRes[std::get<2>(weight)][pAlgName]["utility"] = 
					jRes[std::get<2>(weight)][pAlgName].at("goal_g").get<double>() * std::get<0>(weight) +
					jRes[std::get<2>(weight)][pAlgName].at("expd").get<double>() * pFixedExpTime * std::get<1>(weight);
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
	

	
	
	
	Json run_tiles_44() {
		using D = tiles::TilesGeneric_DomainStack<4,4,true,false,8>;

		Json jDomConfig, jAlgConfig, jRes;
		
		
		jDomConfig["goal"] = std::vector<unsigned>{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
		//jDomConfig["kept"] = std::vector<unsigned>{  1,2,1,1,2,1,2,2,1,2 ,1 ,1 ,2 ,1 ,2 };
		jDomConfig["kept"] = std::vector<unsigned>{  8,7,6,5,4,3,2,1,1, 1, 1, 1, 1, 1, 1};
		D domStack(jDomConfig);


		for(unsigned i=6; i<=6; i++) {
			tiles::BoardState<4,4> initState(korf_15tiles_100instances(i));
			domStack.setInitState(initState);
			
			std::string probKey = std::to_string(i);
			jRes[probKey] = Json();
			jRes.at(probKey)["nonutil"] = Json();
			jRes.at(probKey)["util"] = Json();
				
			std::vector<std::tuple<double,double,std::string>> weights = {
				std::tuple<double,double,std::string>{1,1e6,"1e6"},
				std::tuple<double,double,std::string>{1,1e3, "1e3"},
				std::tuple<double,double,std::string>{1,1,"1"}
			};
			
			double fixedExpTime = 3e-6;
			jAlgConfig["fixed_exptime"] = fixedExpTime;
			//jAlgConfig["expd_limit"] = 200e3;
			
			run_nonutil_search_fixedexptime<D, algorithm::Astar2Impl<D, algorithm::Astar2SearchMode::Standard, algorithm::Astar2HrMode::DomainHr>>(
				domStack, jAlgConfig, jRes.at(probKey).at("nonutil"), jRes.at(probKey).at("util"), "astar", weights, fixedExpTime);
			
			run_nonutil_search_fixedexptime<D, algorithm::Astar2Impl<D, algorithm::Astar2SearchMode::Speedy, algorithm::Astar2HrMode::DomainHr>>(
				domStack, jAlgConfig, jRes.at(probKey).at("nonutil"), jRes.at(probKey).at("util"), "speedy", weights, fixedExpTime);
			
			run_nonutil_search_fixedexptime<D, algorithm::Astar2Impl<D, algorithm::Astar2SearchMode::Greedy, algorithm::Astar2HrMode::DomainHr>>(
				domStack, jAlgConfig, jRes.at(probKey).at("nonutil"), jRes.at(probKey).at("util"), "greedy", weights, fixedExpTime);
			
			
			run_util_search_fixedexptime<D, algorithm::bugsy::BugsyImpl<D, true, algorithm::bugsy::CompRemExp_delay<D>>>(
				domStack, jAlgConfig, jRes.at(probKey).at("util"), "bugsy_delay", weights, fixedExpTime);
			
			
			run_bugsy_rollingbf_allOptions(domStack, jAlgConfig, jRes.at(probKey).at("util"), "bugsy_rollingbf", weights, fixedExpTime);
			
			run_util_search_fixedexptime<D, algorithm::bugsy::BugsyAbtSearchBase<D>>(
				domStack, jAlgConfig, jRes.at(probKey).at("util"), "bugsy_delayAbt", weights, fixedExpTime);

		}
		return jRes;
	}

}

int main(int argc, const char* argv[]) {
	std::cout << mjon661::run_tiles_44().dump(4) << "\n";
	//Bugsy abt remexp = k*bf^depth
}
