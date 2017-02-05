
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <vector>
#include "domain/gridnav/fwd.hpp"
#include "search/debug_walker.hpp"


#include "util/debug.hpp"
#include "util/json.hpp"

namespace mjon661 { namespace gridnav { namespace blocked {
	
	
	void run() {
		Json jConfig;
		jConfig["map"] = "gridnav_20_map";
		jConfig["init"] = 1;
		jConfig["goal"] = 1;
		jConfig["radius"] = 2;
		
		GridNav_DomainStack_StarAbt<5,5,false,false,true> dom(jConfig);
		
		std::ofstream ofs("gridnav_test_out");
		gen_assert(ofs);
		
		dom.dump(ofs);
		
		algorithm::DebugWalker<GridNav_DomainStack_StarAbt<5,5,false,false,true>> alg(dom);
		alg.execute();
	}

}}}


int main(int argc, const char* argv[]) {
	mjon661::gridnav::blocked::run();
}
