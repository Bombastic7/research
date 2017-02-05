
#include <iostream>
#include <string>
#include <sstream>

#include "domain/gridnav/blocked/graph.hpp"




namespace mjon661 { namespace gridnav { namespace blocked {
	
	
	template<typename BaseFuncs>
	void isSolvable(std::string const& pMapFile, unsigned pHeight, unsigned pWidth, unsigned pRadius) {
		
		CellMap<BaseFuncs, false> cellmap(pHeight, pWidth, pMapFile);
		StarAbtCellMap<BaseFuncs, false> abtmap(cellmap, pRadius);
		
		while(true) {
			unsigned startpos, endpos;
			std::string inp;
			
			std::getline(std::cin, inp);
			
			if(inp == "")
				break;
			
			std::istringstream iss(inp);
			
			iss >> startpos >> endpos;
			
			if(abtmap.checkBaseConnected(startpos, endpos))
				std::cout << "true\n";
			else
				std::cout << "false\n";
		}
	}
}}}



int main(int argc, const char* argv[]) {
	
	std::string mapfile = argv[1];
	unsigned height = strtol(argv[2], nullptr, 10);
	unsigned width = strtol(argv[3], nullptr, 10);
	std::string movesStr = argv[4];
	unsigned radius = strtol(argv[5], nullptr, 10);
	
	if(movesStr == "four")
		mjon661::gridnav::blocked::isSolvable<mjon661::gridnav::blocked::FourWayFuncs<>>(mapfile, height, width, radius);
	
	else if(movesStr == "eight")
		mjon661::gridnav::blocked::isSolvable<mjon661::gridnav::blocked::EightWayFuncs<>>(mapfile, height, width, radius);
	
	else
		return 1;
		
	return 0;
}
