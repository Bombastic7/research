
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <vector>
#include "domain/gridnav/blocked/graph.hpp"
#include "domain/star_abt.hpp"
#include "search/debug_walker.hpp"

#include "util/debug.hpp"
#include "util/json.hpp"

#include "search/hastar/v2/hastar.hpp"

#include "search/solution.hpp"

namespace mjon661 { namespace gridnav { namespace blocked {
	
	
	void run() {
		Json jConfig;
		jConfig["map"] = "gridnav_20_map";
		jConfig["width"] = 20;
		jConfig["height"] = 20;
		jConfig["init"] = 9;
		jConfig["goal"] = 380;
		jConfig["radius"] = 2;
		

		
		GridNav_StarAbtStack<CellGraph<4, false, false>, 3> abtStack(jConfig);

		//algorithm::DebugWalker<GridNav_StarAbtStack<CellGraph<4, false, false>, 3>> dbgwalker(abtStack);
		//dbgwalker.execute();
		
		auto s0 = abtStack.getInitState();
		
		algorithm::hastarv2::HAstar_StatsSimple<GridNav_StarAbtStack<CellGraph<4, false, false>, 3>> hastar_alg(abtStack, Json());
		
		Solution<GridNav_StarAbtStack<CellGraph<4, false, false>, 3>> sol;
		
		hastar_alg.execute(s0, sol);
		
		sol.printSolution(abtStack, std::cout);
	}

}}}


int main(int argc, const char* argv[]) {
	mjon661::gridnav::blocked::run();
}
