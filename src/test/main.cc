
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

namespace mjon661 { namespace gridnav { namespace blocked {
	
	
	void run() {
		Json jConfig;
		jConfig["map"] = "gridnav_20_map";
		jConfig["width"] = 20;
		jConfig["height"] = 20;
		jConfig["init"] = 1;
		jConfig["goal"] = 1;
		jConfig["radius"] = 2;
		

		
		GridNav_StarAbtStack<CellGraph<4, false, false>> abtStack(jConfig);
		
		algorithm::DebugWalker<StarAbtStack<GridNav_BaseDomain<4, false, false>>> dbgwalker(abtStack);
		dbgwalker.execute();
	}

}}}


int main(int argc, const char* argv[]) {
	mjon661::gridnav::blocked::run();
}
