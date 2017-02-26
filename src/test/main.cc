
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
		
		GridNav_BaseDomain<4, false, false> dom(jConfig);
		
		for(auto it = dom.stateBegin(); it != dom.stateEnd(); ++it) {
			std::cout << *it << "\n";;
		}
		
		std::cout << "\n";
		

		
		StarAbtStack<GridNav_BaseDomain<4, false, false>> abtStack(dom, 2);
		
		algorithm::DebugWalker<StarAbtStack<GridNav_BaseDomain<4, false, false>>> dbgwalker(dom);
		dbgwalker.execute();
	}

}}}


int main(int argc, const char* argv[]) {
	mjon661::gridnav::blocked::run();
}
