
#include <stdexcept>
#include <fstream>
#include <iostream>
#include "domain/gridnav/blocked/graph.hpp"


namespace mjon661 { namespace gridnav { namespace blocked {
	
	
	static void run() {
		
		std::ifstream ifs("gridnav_20_map");
		
		if(!ifs)
			throw std::runtime_error("");
		
		CellMap<EightWayFuncs<>, true> cellmap(20, 20, ifs);
		
		//cellmap.dump(std::cout, false, true, {4,5,6,7});
		
		AbstractCellMap<EightWayFuncs<>, true, 2, 2> abtmap(cellmap, 0);
		abtmap.dump(std::cout, true, true, {0,1,2,3});
		
		AbstractCellMap<EightWayFuncs<>, true, 2, 2> abtmap2(abtmap, 0);
		abtmap2.dump(std::cout, true, true, {0,1,2,3});
	}
	
}}}


int main(int argc, const char* argv[]) {
	mjon661::gridnav::blocked::run();
	
}
