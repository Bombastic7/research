
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <vector>
#include "domain/gridnav/blocked/graph.hpp"


#include "util/debug.hpp"
#include "util/json.hpp"

namespace mjon661 { namespace gridnav { namespace blocked {
	
	
	void run() {
		Json jConfig;
		jConfig["map"] = "gridnav_20_map";
		jConfig["init"] = 1;
		jConfig["goal"] = 1;
		jConfig["radius"] = 2;
		
		GridNav_BaseDomain<4, false, false> dom(jConfig);
		
		StarAbtStack<GridNav_BaseDomain<4, false, false>> abtStack(dom, 2);
	}

}}}


int main(int argc, const char* argv[]) {
	mjon661::gridnav::blocked::run();
}
