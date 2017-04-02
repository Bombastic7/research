
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include "util/json.hpp"

#include "search/astar2.hpp"
#include "search/enum_util_results.hpp"
#include "search/bugsy_abt_lin.hpp"
#include "search/debug_walker.hpp"
#include "domain/tiles/fwd.hpp"


namespace mjon661 {
	

	
	
	static void run() {
		using D = tiles::TilesGeneric_DomainStack<3,3,true,false,5>;
		
		Json jDomConfig, jRes;
		
		jDomConfig["goal"] = std::vector<unsigned>{0,1,2,3,4,5,6,7,8};
		jDomConfig["kept"] = std::vector<unsigned>{  5,4,3,2,1,1,1,1};

		D domStack(jDomConfig);
		
		std::stringstream ss;
		domStack.getInitState().prettyPrint(ss);
		jRes["init_state"] = ss.str();
		
		ss.clear();
		domStack.mGoalState.prettyPrint(ss);
		jRes["goal_state"] = ss.str();
		

		{
			algorithm::EnumUtilSolutions<D, algorithm::EnumUtilSolutionsHrMode::DomainHr> alg(domStack, Json());
			
			alg.execute(domStack.getInitState());
			
			jRes["alg_report"] = alg.report();
			
			std::cout << jRes.dump(4) << "\n";
		}
	}
	
}



int main(int argc, const char* argv[]) {

	mjon661::run();
}
