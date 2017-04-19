
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include "util/json.hpp"

#include "domain/tiles/fwd.hpp"

#include "experiment/tiles_problems.hpp"

#include "search/debug_walker.hpp"


namespace mjon661 {
	

	
	static void run() {
		Json jDomConfig;
		jDomConfig["init"] = tiles::tiles8_instances(0);
		jDomConfig["kept"] = tiles::tiles_abtfirst5(8);
		jDomConfig["goal"] = tiles::tiles_defgoal(9);
		
		using D = tiles::TilesGeneric_DomainStack<3,3,true,true,5>;
		
		D domStack(jDomConfig);
		
		algorithm::DebugWalker<D> alg(domStack, Json());
		
		alg.execute(domStack.getInitState());
	}
}



int main(int argc, const char* argv[]) {

	mjon661::run();
}
