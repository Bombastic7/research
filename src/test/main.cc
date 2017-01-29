
#include <stdexcept>
#include <fstream>
#include <iostream>
#include "domain/gridnav/blocked/graph.hpp"


namespace mjon661 { namespace gridnav { namespace blocked {
	
	
	static void run() {
		
		std::ifstream ifs("gridnav_20_map");
		
		if(!ifs)
			throw std::runtime_error("");
		
		CellMap<FourWayFuncs<>, false> cellmap(20, 20, ifs);
		
		cellmap.dump(std::cout, {0, 1});
		
		
		
	}
	
}}}


int main(int argc, const char* argv[]) {
	mjon661::gridnav::blocked::run();
	
}
