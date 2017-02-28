
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <vector>
//#include "domain/gridnav/blocked/graph.hpp"
//#include "domain/pancake/fwd.hpp"
#include "domain/tiles/fwd.hpp"
#include "domain/star_abt.hpp"
#include "search/debug_walker.hpp"

#include "util/debug.hpp"
#include "util/json.hpp"

#include "search/hastar/v2/hastar.hpp"

#include "search/solution.hpp"
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

namespace mjon661 { namespace tiles {
	
	
	void run() {
		Json jConfig;
		jConfig["init"] = std::vector<unsigned>{4, 3, 2, 1, 8, 0, 6, 5, 7};
		jConfig["init"] = std::vector<unsigned>{0, 1, 2, 3, 4, 5, 6, 7, 8};
		jConfig["kept"] = std::vector<unsigned>{5, 4, 3, 2, 1, 1, 1, 1};
		
		
		
		TilesGeneric_DomainStack<3,3, true, true, 5> domStack(jConfig);

		auto s0 = domStack.getInitState();
		
		algorithm::DebugWalker<TilesGeneric_DomainStack<3,3, true, true, 5>> dbgwalker(domStack);
		dbgwalker.execute();


		algorithm::hastarv2::HAstar_StatsSimple<TilesGeneric_DomainStack<3,3, true, true, 5>> hastar_alg(domStack, Json());
		
		Solution<TilesGeneric_DomainStack<3,3, true, true, 5>> sol;
		
		hastar_alg.execute(s0, sol);
		
		sol.printSolution(domStack, std::cout);
		std::cout << "Path cost: " << sol.pathCost(domStack) << "\n";
		Json jStats = hastar_alg.report();
		
		std::cout << jStats.dump(4) << "\n";
	}

}}

int main(int argc, const char* argv[]) {
	//mjon661::gridnav::blocked::run();
	//mjon661::pancake::run();
	mjon661::tiles::run();
}
