
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


namespace mjon661 {
	

	static void run() {
		std::vector<unsigned> foo {0,1,2,3};
		
		Json j;
		std::cout << j.dump(2) << "\n\n";
		
		j["a"] = {};
		std::cout << j.dump(2) << "\n\n";
		j["b"].push_back(1);
		std::cout << j.dump(2) << "\n\n";
		
		j["c"].push_back(foo);
		std::cout << j.dump(2) << "\n\n";
	}
}



int main(int argc, const char* argv[]) {

	mjon661::run();
}
