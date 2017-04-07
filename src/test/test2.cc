
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include "util/json.hpp"

//#include "search/astar2.hpp"
//#include "search/enum_util_results.hpp"
//#include "search/bugsy_abt_lin.hpp"
//#include "search/bugsy_abt_exp1.hpp"
//#include "search/debug_walker.hpp"
//#include "domain/tiles/fwd.hpp"

#include "domain/gridnav/dim2/gridnav2d.hpp"


namespace mjon661 {
	
	struct Foo {
		unsigned short f0, f1;
	};

	static void run() {
		gridnav::dim2::StarAbtStackInfo<gridnav::dim2::CellGraph<4,false,false>> abtStackInfo(20,20,2,",fill");
		
		abtStackInfo.dumpToFile("gridnav_test_dump.json");
	}
}



int main(int argc, const char* argv[]) {

	mjon661::run();
}
