
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cstdio>
#include "domain/gridnav/blocked/graph.hpp"

#include "util/debug.hpp"

namespace mjon661 { namespace gridnav { namespace blocked {
	
	
	static void run() {
		
		std::ifstream ifs("gridnav_20_map");
		
		if(!ifs)
			throw std::runtime_error("");
			
		
		std::vector<Cell_t> cellMap;
			
		for(unsigned i=0; i<400; i++) {
			int v;
			Cell_t c;
			ifs >> v;
			c = (Cell_t)v;
			
			gen_assert(c == Cell_t::Open || c == Cell_t::Blocked);
			cellMap.push_back(c);
		}
		
		
		//std::vector<unsigned> ops = {0,1,2,3};
		
		//CellMap<FourWayFuncs<>, true> cellmap(20, 20, ifs);
		//cellmap.dump(std::cout, false, true, {4,5,6,7});
		//cellmap.drawMap(std::cout);
		//std::cout << "\n";
		
		drawMap(cellMap, 20, 20, std::cout);
		
		
		std::vector<unsigned> groupedmap = compressMapD(cellMap, 20, 20);
		

		for(unsigned g=0; ; g++) {
			bool seenGroup = false;
			for(unsigned i=0; i<20; i++) {
				for(unsigned j=0; j<20; j++) {
					if(groupedmap[i*20+j] == (unsigned)-1)
						std::cout << "O ";
					else if(groupedmap[i*20+j] == g) {
						std::cout << (char)(groupedmap[i*20+j] % 26 + 'A')<< " ";
						seenGroup = true;
					}
					else {
						std::cout << ". ";
					}
						
				}
				std::cout << "\n";
			}
			getchar();
			std::cout << "\n";
			if(!seenGroup)
				break;
		//std::vector<Cell_t> xcompmap = compressMapB(cellMap, 20, 20);
		//std::cout << "\n";
		//drawMap(xcompmap, 10, 20, std::cout);
		}
	}
	
}}}


int main(int argc, const char* argv[]) {
	mjon661::gridnav::blocked::run();
	
}
