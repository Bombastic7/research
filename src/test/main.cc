
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <utility>
#include <tuple>
#include "domain/gridnav/blocked/graph.hpp"
#include "domain/pancake/fwd.hpp"
#include "domain/tiles/fwd.hpp"
//#include "domain/star_abt.hpp"

#include "util/debug.hpp"
#include "util/json.hpp"

//#include "search/hastar/v2/hastar.hpp"

#include "search/solution.hpp"

#include "domain/test_graph.hpp"
#include "search/ugsa/v6/ugsa.hpp"

#include "search/admissible_abtsearch2.hpp"
#include "search/astar.hpp"
#include "search/bugsy.hpp"
#include "search/debug_walker.hpp"

#include "domain/gridnav/hypernav/hypernav_real.hpp"
#include "search/astar2.hpp"

#include "search/ugsa.hpp"

namespace mjon661 {
	
	//~ void test_moves() {
		//~ using D = gridnav::hypernav::DomainRealStack<3, 3>;
		
		//~ Json jDomConfig;
		//~ jDomConfig["map"] = ",random_hills,100,0,10,100,20";
		//~ jDomConfig["dimsz"] = std::vector<unsigned>{100,100,100};
		
		//~ D domStack(jDomConfig);
		//~ typename D::template Domain<0> dom(domStack);
		
		//~ unsigned n=0;
		
		//~ std::array<unsigned, 3> s = {1,1,1};
		
		//~ for(auto it=dom.getAdjEdges(s); !it.finished(); it.next()) {
			//~ std::cout << n++ << ": ";
			//~ dom.prettyPrintState(it.state(), std::cout);
			//~ std::cout << "   " << it.cost() << "\n";
		//~ }
	//~ }
	
	
	//~ struct AstarExpandedNodesSort {
		
		//~ template<typename Node>
		//~ bool operator()(Node* a, Node* b) const {
			//~ return a->expn < b->expn;
		//~ }
	//~ };
	
	//~ void test_hypernav_real() {
		//~ using D = gridnav::hypernav::DomainRealStack<2, 2>;
		//~ using Alg_t = algorithm::Astar2Impl<D, algorithm::Astar2SearchMode::Standard>;
		
		//~ Json jDomConfig;
		//~ jDomConfig["map"] = ",random_cones,100,0,10,20";
		//~ jDomConfig["dimsz"] = std::vector<unsigned>{1000,1000};
		
		//~ D domStack(jDomConfig);
		//~ domStack.assignInitGoalStates({{500,500}, {999,999}});
		
		//~ typename D::template Domain<0> dom(domStack);
		
		//~ Alg_t alg(domStack, Json());
		
		//~ alg.execute(domStack.getInitState());
		
		//~ std::vector<typename Alg_t::Node*> expNodes;
		
		//~ for(auto it=alg.mClosedList.begin(); it!=alg.mClosedList.end(); ++it) {
			//~ if((*it)->expn != (unsigned)-1)
				//~ expNodes.push_back(*it);
		//~ }
		
		//~ std::sort(expNodes.begin(), expNodes.end(), AstarExpandedNodesSort());


		//~ std::vector<unsigned> expStates;
		
		//~ for(auto* n : expNodes)
			//~ expStates.push_back(n->pkd);

		//~ domStack.mCellMap.dumpHeatMap1(1000,1000);
		//~ domStack.mCellMap.dumpHeatMap2(1000,1000, expStates);
		//~ domStack.mCellMap.dumpHeatMap3(1000,1000, expStates);
		
		
	//~ }
	
	
	

	
	std::vector<int> korf_15tiles_100instances(unsigned i) {
		fast_assert(i >= 1 && i <= 5);
		
		switch(i) {
			case 1:
				return {14,13,15,7,11,12,9,5,6,0,2,1,4,8,10,3};
			case 2:
				return {13,5,4,10,9,12,8,14,2,3,7,1,0,15,11,6};
			case 3:
				return {14,7,8,2,13,11,10,4,9,12,5,0,3,6,1,15};
			case 4:
				return {5,12,10,7,15,11,14,0,8,2,1,13,3,4,9,6};
			case 5:
				return {4,7,14,13,10,3,9,12,11,5,6,15,1,2,8,0};
		}
		
		return {};
	}
	
	
	
	

	template<typename D, typename Alg_t>
	void run_util_search(D& pDomStack, Json const& jAlgConfig, std::string const& pAlgName, Json& jRes) {
		using namespace algorithm;
		
		Timer searchTimer;
		
		Alg_t alg_bugsy(pDomStack, jAlgConfig);

		searchTimer.start();
		alg_bugsy.execute(pDomStack.getInitState());
		searchTimer.stop();
		

		
		jRes[pAlgName] = alg_bugsy.report();
		jRes[pAlgName]["walltime"] = searchTimer.seconds();
		jRes[pAlgName]["utility"] = 
			jRes[pAlgName]["goal_g"].get<double>() * jRes[pAlgName]["wf"].get<double>() +
			jRes[pAlgName]["expd"].get<double>() * jRes[pAlgName]["fixed_exp_time"].get<double>() * jRes[pAlgName]["wt"].get<double>();
	}
	

	template<typename D, unsigned v0, unsigned v1, unsigned v2, unsigned v3, unsigned v4>
	void run_bugsy_fixed_rollingbf(D& pDomStack, Json const& jAlgConfig, std::string const& pAlgName, Json& jRes) {
		using Alg_t = algorithm::bugsy::BugsyImpl<D, true, algorithm::bugsy::SearchMode::RollingBf, v0, v1, v2, v3, v4>;
		
		run_util_search<D,Alg_t>(	pDomStack, 
									jAlgConfig, 
									pAlgName + "_" + algorithm::bugsy::C_RollingBf::niceNameStr(v0,v1,v2,v3,v4), 
									jRes);
	}
	
	
	
	
	
	Json run_tiles_44() {
		
		using D = tiles::TilesGeneric_DomainStack<4,4,true,false,1>;
		
		using Alg_Astar_t = algorithm::Astar2Impl<D, algorithm::Astar2SearchMode::Standard>;
		//using Alg_Speedy_t = algorithm::Astar2Impl<D, algorithm::Astar2SearchMode::Speedy>;
		//using Alg_Bugsy_t = algorithm::BugsyImpl<D, true>;
		//using Alg_Ugsa_t = algorithm::UgsaImpl<D, true>;
		
		Json jDomConfig, jAlgConfig, jRes;
		
		
		jDomConfig["goal"] = std::vector<unsigned>{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
		jDomConfig["kept"] = std::vector<unsigned>{  1,1,1,1,1,1,1,1,1,1 ,1 ,1 ,1 ,1 ,1 };
		D domStack(jDomConfig);

		
		for(unsigned i=1; i<=1; i++) {
			tiles::BoardState<4,4> initState(korf_15tiles_100instances(i));
			domStack.setInitState(initState);
			std::string probKey = std::to_string(i);
			
			
			{
				Alg_Astar_t alg_astar(domStack, Json());
				//searchTimer.start();
				//alg_astar.execute(domStack.getInitState());
				//searchTimer.stop();
				//jRes[std::to_string(i)]["astar"] = alg_astar.report();
				//jRes[std::to_string(i)]["astar"]["walltime"] = searchTimer.seconds();
				
			}
			
			{
				//Alg_Speedy_t alg_speedy(domStack, Json());
				//searchTimer.start();
				//alg_speedy.execute(domStack.getInitState());
				//searchTimer.stop();
				//jRes[std::to_string(i)]["speedy"] = alg_speedy.report();
				//jRes[std::to_string(i)]["speedy"]["walltime"] = searchTimer.seconds();
			}
			
			

			
			{
				
				std::vector<std::tuple<double,double,std::string>> weights = {
						std::tuple<double,double,std::string>{1,1e6,"1e6"},
						std::tuple<double,double,std::string>{1,1e3, "1e3"},
						std::tuple<double,double,std::string>{1,1,"1"}
				};
				for(unsigned j=0; j<1; j++) {
					jAlgConfig["wf"] = std::get<0>(weights[j]);
					jAlgConfig["wt"] = std::get<1>(weights[j]);
					jAlgConfig["fixed_exptime"] = 3e-6;
					jAlgConfig["expd_limit"] = 500e3;
					
					std::string algName = std::string("bugsy_") + std::get<2>(weights[j]) + "_";


					run_util_search<D, algorithm::bugsy::BugsyImpl<D,true,algorithm::bugsy::SearchMode::Delay>>(domStack, jAlgConfig, algName + "_delay", jRes);
					
					run_bugsy_fixed_rollingbf<D, 0,0,0,0,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 0,0,0,0,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 0,0,0,1,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 0,0,0,1,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 0,0,1,0,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 0,0,1,0,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 0,0,1,1,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 0,0,1,1,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 0,1,0,0,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 0,1,0,0,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 0,1,0,1,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 0,1,0,1,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 0,1,1,0,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 0,1,1,0,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 0,1,1,1,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 0,1,1,1,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 1,0,0,0,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 1,0,0,0,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 1,0,0,1,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 1,0,0,1,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 1,0,1,0,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 1,0,1,0,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 1,0,1,1,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 1,0,1,1,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 1,1,0,0,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 1,1,0,0,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 1,1,0,1,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 1,1,0,1,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 1,1,1,0,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 1,1,1,0,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 1,1,1,1,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 1,1,1,1,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 2,0,0,0,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 2,0,0,0,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 2,0,0,1,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 2,0,0,1,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 2,0,1,0,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 2,0,1,0,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 2,0,1,1,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 2,0,1,1,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 2,1,0,0,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 2,1,0,0,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 2,1,0,1,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 2,1,0,1,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 2,1,1,0,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 2,1,1,0,1>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 2,1,1,1,0>(domStack, jAlgConfig, algName, jRes);
					run_bugsy_fixed_rollingbf<D, 2,1,1,1,1>(domStack, jAlgConfig, algName, jRes);


				}
			}
		}
		return jRes;
	}

}

int main(int argc, const char* argv[]) {
	std::cout << mjon661::run_tiles_44().dump(4) << "\n";

}
