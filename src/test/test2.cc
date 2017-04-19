
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include "util/json.hpp"

#include "domain/tiles/fwd.hpp"

#include "experiment/tiles_problems.hpp"

#include "search/debug_walker.hpp"
#include "search/bugsy.hpp"
#include "search/bugsy_abt_lin.hpp"


namespace mjon661 {
	

	
	static void run() {
		//~ Json jDomConfig;
		//~ jDomConfig["init"] = tiles::tiles15_instances_korf(0);
		//~ jDomConfig["kept"] = tiles::tiles_abtfirst7(15);
		//~ jDomConfig["goal"] = tiles::tiles_defgoal(16);
		
		//~ using D = tiles::TilesGeneric_DomainStack<4,4,true,true,7>;
		
		//~ D domStack(jDomConfig);
		
		//~ Json jAlgConfig;
		//~ jAlgConfig["wf"] = 1;
		//~ jAlgConfig["wt"] = 1e6;
		//~ jAlgConfig["exptime"] = 3e-6;
		
		//~ algorithm::Bugsy<D,true> alg_domhr(domStack, jAlgConfig);
		//~ algorithm::BugsyAbtLin_baseSearch<D,true> alg_abt(domStack, jAlgConfig);
		
		//~ alg_domhr.execute(domStack.getInitState());
		//~ alg_abt.execute(domStack.getInitState());
		
		//~ algorithm::DebugWalker<D> alg(domStack, Json());
		//~ alg.execute(domStack.getInitState());
		
		
		Json j;
		
		std::cout << j.empty() << "\n";
		
		j["foo"] = {{"bar","baz"}};
		j["lorem"] = {};
		
		std::cout << j.empty() << "\n";
		
		std::cout << j.at("foo").empty() << "\n";
		std::cout << j.at("lorem").empty() << "\n";
	}
}



int main(int argc, const char* argv[]) {

	mjon661::run();
}
