
#include <iostream>
#include <string>
#include <sstream>

#include "gridnav/domain/graph.hpp"


int main(int argc, const char* argv[]) {
	
	std::string mapfile = argv[1];
	unsigned height = strtol(argv[2], nullptr, 10);
	unsigned width = strtol(argv[3], nullptr, 10);
	
	mjon661::gridnav::blocked::CellMap cellmap(height, width, mapfile);
	mjon661::gridnav::blocked::StarAbtCellMap abtmap(cellmap);
	
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
