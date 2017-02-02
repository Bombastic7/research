
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <vector>
#include "domain/gridnav/blocked/graph.hpp"

#include "util/debug.hpp"

namespace mjon661 { namespace gridnav { namespace blocked {
	
	
	static void run() {
		
		std::ifstream ifs("gridnav_20_map");
		
		if(!ifs)
			throw std::runtime_error("");
			
		
		CellMap<FourWayFuncs<>, false> cellMap(20, 20, ifs);

		//drawMap(cellMap.getCells(), 20, 20, std::cout);
		std::cout << "\n";
		
		StarAbtCellMap<FourWayFuncs<>, false> abtmap(cellMap, 2);
		
		std::vector<unsigned> abtsizes = abtmap.getLevelSizes();
		
		for(unsigned i=0; i<abtsizes.size(); i++)
			std::cout << i << ": " << abtsizes[i] << "\n";
		
		getchar();
		
		for(unsigned i=0; i<abtsizes.size(); i++) {
			std::cout << i << "\n";
			abtmap.dump(std::cout, i);
			getchar();
			
		}
		//std::cout << abtmap.getLevels() << "\n";
		
/*	
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
		}*/
	}
	
}}}


int main(int argc, const char* argv[]) {
	mjon661::gridnav::blocked::run();
	
}
