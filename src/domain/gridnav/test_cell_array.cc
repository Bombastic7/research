#include "domain/gridnav/cell_array.hpp"

#include <fstream>

#include "util/debug.hpp"


namespace mjon661 { namespace gridnav {


	static void run(int argc, const char* argv[]) {
		fast_assert(argc > 1);
		std::ifstream ifs(argv[1]);
		
		GridNavMapStack_OpOrBl_MergeAbt gridStack(10,10,1000,2,2,2);
		
		gridStack.read(ifs);
		
		for(unsigned i=0; i<gridStack.getNLevels(); i++) {
			GridNavMap_OpOrBl const& gridMap = gridStack.getLevel(i);
			
			gridMap.write(std::cout);
		}
	}



}}


int main(int argc, const char* argv[]) {
	mjon661::gridnav::run(argc, argv);
}
