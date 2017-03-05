
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <vector>
#include "domain/gridnav/blocked/graph.hpp"
//#include "domain/pancake/fwd.hpp"
//#include "domain/tiles/fwd.hpp"
//#include "domain/star_abt.hpp"
#include "search/debug_walker.hpp"

#include "util/debug.hpp"
#include "util/json.hpp"

//#include "search/hastar/v2/hastar.hpp"

#include "search/solution.hpp"

#include "domain/test_graph.hpp"
#include "search/ugsa/v6/ugsa.hpp"

#include "search/ugsa/cost_pure/ugsa_cost_pure.hpp"

/*
namespace mjon661 { namespace gridnav { namespace blocked {
	
	
	void run() {
		Json jConfig;
		jConfig["map"] = "gridnav_20_map";
		jConfig["width"] = 20;
		jConfig["height"] = 20;
		jConfig["init"] = 1;
		jConfig["goal"] = 305;
		jConfig["radius"] = 2;
		

		
		GridNav_StarAbtStack<CellGraph<4, false, false>, 3> abtStack(jConfig);

		//algorithm::DebugWalker<GridNav_StarAbtStack<CellGraph<4, false, false>, 3>> dbgwalker(abtStack);
		//dbgwalker.execute();
		
		auto s0 = abtStack.getInitState();
		
		algorithm::hastarv2::HAstar_StatsSimple<GridNav_StarAbtStack<CellGraph<4, false, false>, 3>> hastar_alg(abtStack, Json());
		
		Solution<GridNav_StarAbtStack<CellGraph<4, false, false>, 3>> sol;
		
		hastar_alg.execute(s0, sol);
		
		sol.printSolution(abtStack, std::cout);
		
		Json jStats = hastar_alg.report();
		std::cout << jStats.dump(4) << "\n";
	}

}}}
*/
/*
namespace mjon661 { namespace pancake {
	
	
	void run() {
		Json jConfig;
		jConfig["init"] = std::vector<unsigned>{0, 4, 1, 2, 6, 3, 7, 5};
		jConfig["kept"] = std::vector<unsigned>{3, 3, 3, 2, 2, 1, 1, 1};
		
		
		
		Pancake_DomainStack_IgnoreAbt<8, 5, 2, true, false> domStack(jConfig);

		auto s0 = domStack.getInitState();
		
		algorithm::DebugWalker<Pancake_DomainStack_IgnoreAbt<8, 5, 2, true, false>> dbgwalker(domStack);
		dbgwalker.execute();


		algorithm::hastarv2::HAstar_StatsSimple<Pancake_DomainStack_IgnoreAbt<8, 5, 2, true, false>> hastar_alg(domStack, Json());
		
		Solution<Pancake_DomainStack_IgnoreAbt<8, 5, 2, true, false>> sol;
		
		hastar_alg.execute(s0, sol);
		
		sol.printSolution(domStack, std::cout);
		std::cout << "Path cost: " << sol.pathCost(domStack) << "\n";
		Json jStats = hastar_alg.report();
		
		std::cout << jStats.dump(4) << "\n";
	}

}}
*/
/*
namespace mjon661 { namespace tiles {
	
	
	void run() {
		Json jConfig;
		jConfig["init"] = std::vector<unsigned>{4, 3, 2, 1, 8, 0, 6, 5, 7};
		jConfig["goal"] = std::vector<unsigned>{0, 1, 2, 3, 4, 5, 6, 7, 8};
		jConfig["kept"] = std::vector<unsigned>{5, 4, 3, 2, 1, 1, 1, 1};
		
		
		
		TilesGeneric_DomainStack<3,3, false, true, 5> domStack(jConfig);

		auto s0 = domStack.getInitState();
		
		algorithm::DebugWalker<TilesGeneric_DomainStack<3,3, false, true, 5>> dbgwalker(domStack);
		dbgwalker.execute();


		algorithm::hastarv2::HAstar_StatsSimple<TilesGeneric_DomainStack<3,3, false, true, 5>> hastar_alg(domStack, Json());
		
		Solution<TilesGeneric_DomainStack<3,3, false, true, 5>> sol;
		
		hastar_alg.execute(s0, sol);
		
		sol.printSolution(domStack, std::cout);
		std::cout << "Path cost: " << sol.pathCost(domStack) << "\n";
		Json jStats = hastar_alg.report();
		
		std::cout << jStats.dump(4) << "\n";
	}

}}
*/


namespace mjon661 {
	
	
	void run_ugsapure() {
		Json jDomConfig;
		//jDomConfig["goal"] = std::vector<unsigned>{0, 1, 2, 3, 4, 5, 6, 7, 8};
		//jDomConfig["kept"] = std::vector<unsigned>{5, 4, 3, 2, 1, 1, 1, 1};
		jDomConfig["map"] = ".1";
		jDomConfig["width"] = 500;
		jDomConfig["height"] = 500;
		
		using DomStack_t = gridnav::blocked::GridNav_StarAbtStack<gridnav::blocked::CellGraph_4_hr<false,false>,5>;
		
		DomStack_t domStack(jDomConfig);

		//auto s0 = domStack.getInitState();

		Solution<DomStack_t> sol;

		using UGSAAlg_t = algorithm::ugsav6::UGSAv6<DomStack_t>;
		
		Json jConfig;
		jConfig["wf"] = 1;
		jConfig["wt"] = 1;
		UGSAAlg_t ugsaAlg(domStack, jConfig);
		

		ugsaAlg.execute(domStack.getInitState(), sol);

		Json jStats = ugsaAlg.report();
		std::cout << jStats.dump(4) << "\n";
	}

}

/*
namespace mjon661 { namespace testgraph {
	
	
	void run() {
		Json jDomConfig;
		//jDomConfig["init"] = std::vector<unsigned>{4, 3, 2, 1, 8, 0, 6, 5, 7};
		//jDomConfig["goal"] = std::vector<unsigned>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
		//jDomConfig["kept"] = std::vector<unsigned>{6, 5, 4, 3, 2, 1, 1, 1, 1, 1, 1};
		
		jDomConfig["goal"] = std::vector<unsigned>{0, 1, 2, 3, 4, 5, 6, 7, 8};
		jDomConfig["kept"] = std::vector<unsigned>{5, 4, 3, 2, 1, 1, 1, 1};
		
		//using DomStack_t = GridTestGraphStack<5>;
		//using DomStack_t = tiles::TilesGeneric_DomainStack<3, 4, false, false, 6>;
		using DomStack_t = tiles::TilesGeneric_DomainStack<3, 3, false, false, 5>;
		
		
		DomStack_t domStack(jDomConfig);
		
		using HAalg_t = algorithm::hastarv2::HAstar_StatsSimple<DomStack_t>;
		
		HAalg_t hastar_alg(domStack, Json());


		Solution<DomStack_t> sol;
		
		//hastar_alg.execute(domStack.getInitState(), sol);
		//
	//	sol.printSolution(domStack, std::cout);
	//	std::cout << "Path cost: " << sol.pathCost(domStack) << "\n";
	//	Json jStats = hastar_alg.report();
		
	//	std::cout << jStats.dump(4) << "\n";
		
		
		using UGSAv6AbtAlg_t = algorithm::ugsav6::UGSAv6<DomStack_t>;
		
		Json jConfig;
		jConfig["wf"] = 1;
		jConfig["wt"] = 1;
		UGSAv6AbtAlg_t ugsaAlg(domStack, jConfig);
		
		
		int remcost;
		algorithm::ugsav6::Util_t remdist;
		
		unsigned nruns = 0;
		for(unsigned i=0; i<2; i++) {
			nruns = 0;
			tiles::ProblemIterator<3,3> it(std::vector<int>{0,1,2,3,4,5,6,7,8});
			
			
			do {
				ugsaAlg.computeRemainingEffort(it(), remcost, remdist);
				ugsaAlg.reset();
				nruns++;
				if(nruns%10 == 0)
					std::cout << nruns << "\n";
			} while(it.next());
		}
		
		ugsaAlg.dumpExactCache(std::cout);
		
		std::cout << "\n\n" << remcost << " " << remdist << "\n";
	
	
		ugsaAlg.execute(domStack.randInitState(123), sol);
		
		//sol.printSolution(domStack, std::cout);
		//std::cout << "Path cost: " << sol.pathCost(domStack) << "\n";
		Json jStats = ugsaAlg.report();
		std::cout << jStats.dump(4) << "\n";
	}

}}
*/
int main(int argc, const char* argv[]) {
	//mjon661::gridnav::blocked::run();
	//mjon661::pancake::run();
	//mjon661::tiles::run();
	
	//mjon661::testgraph::run();
	
	mjon661::run_ugsapure();
	
	
	//std::cout << stats[0] << " " << stats[1] << " " << stats[2] << "\n";
}
