
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <utility>
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


namespace mjon661 {
	
	
	void test_hypernav_real() {
		using D = gridnav::hypernav::DomainRealStack<2, 1>;
		using Alg_t = algorithm::Astar2Impl<D, algorithm::Astar2SearchMode::Standard>;
		
		Json jDomConfig;
		jDomConfig["map"] = ",random_hills,100,0,10,100,20";
		
		D domStack(jDomConfig);
		
		Alg_t alg(domStack, Json());
		
		alg.execute(domStack.getInitState());
		
		std::vector<unsigned> expStates;
		
		for(auto it=alg.mClosedList.begin(); it!=alg.mClosedList.end(); ++it)
			expStates.push_back((*it)->pkd);
		

		domStack.mCellMap.dumpHeatMap1(100,100);
		domStack.mCellMap.dumpHeatMap2(100,100, expStates);
		
	}

}

int main(int argc, const char* argv[]) {
	//mjon661::gridnav::blocked::run();
	//mjon661::pancake::run();
	//mjon661::tiles::run();
	
	//mjon661::testgraph::run();
	
	//mjon661::run_ugsapure();
	
	//mjon661::Json res;
	//res["gridnav"] = mjon661::run_session1_gridnav();
	
	//std::cout << res.dump(4);
	
	//std::cout << mjon661::test_hypernav2().dump(4);
	//std::cout << mjon661::test_pancake().dump(4);
	//std::cout << mjon661::run_tiles_44().dump(4);
	
	//std::cout << mjon661::gridnav::cube_blocked::test_cubenav_dirs() << "\n";
	
	//std::cout << stats[0] << " " << stats[1] << " " << stats[2] << "\n";
	
	//mjon661::gridnav::hypernav_blocked::test_statepack<3>();
	mjon661::test_hypernav_real();
	//mjon661::test_hypergridmoveset();
}
