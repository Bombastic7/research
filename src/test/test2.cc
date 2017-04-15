
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include "util/json.hpp"

#include "search/bugsy2.hpp"
#include "search/astar2.hpp"

#include "domain/tiles/fwd.hpp"


namespace mjon661 {
	
	
	//~ static void run8() {		
		//~ using D = tiles::TilesGeneric_DomainStack<3,3,true,true,1>;
		//~ //using Alg_t = algorithm::Bugsy2<D>;
		//~ using Alg_hr_t = algorithm::Astar2Impl<D, algorithm::Astar2SearchMode::Standard, algorithm::Astar2HrMode::DomainHr>;
		//~ using Alg_un_t = algorithm::Astar2Impl<D, algorithm::Astar2SearchMode::Uninformed, algorithm::Astar2HrMode::DomainHr>;
		
		//~ Json jDomConfig;
		//~ jDomConfig["kept"] = std::vector<int>{  1,1,1,1,1,1,1,1};
		//~ jDomConfig["goal"] = std::vector<int>{0,1,2,3,4,5,6,7,8};
		//~ jDomConfig["init"] = std::vector<int>{7,6,4,1,5,3,8,0,2,};
		//~ //jDomConfig["init"] = std::vector<int>{6,5,4,7,8,3,1,0,2};
		
		//~ D domStack(jDomConfig);
		//~ //domStack.setInitState(domStack.randInitState(0));

		//~ typename D::template Domain<0> testDom(domStack);
		//~ testDom.mManhattan.dump(std::cout);
		//~ testDom.prettyPrintState(domStack.getGoalState(), std::cout);
		
		//~ std::cout << "\n\n";
		
		//~ testDom.mManhattan.evalDump(domStack.getGoalState(), std::cout);
		//~ //return;
		
		//~ Alg_hr_t alg_hr(domStack, Json());
		
		//~ alg_hr.execute(domStack.getInitState());
		
		//~ {
			//~ std::ofstream ofs("astar_fd_test.json");
			//~ fast_assert(ofs);
			//~ ofs << alg_hr.report().dump(2);
		//~ }
		
		//~ Alg_un_t alg_un(domStack, Json());
		
		//~ alg_un.execute(domStack.getInitState());
		
		//~ {
			//~ std::ofstream ofs("astar_fd_test_un.json");
			//~ fast_assert(ofs);
			//~ ofs << alg_un.report().dump(2);
		//~ }
	//~ }
	
	//~ static void run11() {		
		//~ using D = tiles::TilesGeneric_DomainStack<3,4,true,false,1>;
		//~ using Alg_t = algorithm::Bugsy2<D>;
		
		//~ Json jDomConfig;
		//~ jDomConfig["kept"] = std::vector<int>{1,1,1,1,1,1,1,1,1,1,1};
		//~ jDomConfig["goal"] = std::vector<int>{0,1,2,3,4,5,6,7,8,9,10,11};
		
		//~ D domStack(jDomConfig);
		//~ domStack.setInitState(domStack.randInitState(0));

		//~ Alg_t alg(domStack, Json());
		
		//~ alg.execute(domStack.getInitState());
		
		//~ {
			//~ std::ofstream ofs("astar_fd_test.json");
			//~ fast_assert(ofs);
			//~ ofs << alg.report().dump(2);
		//~ }
	//~ }
	
	static void run15() {		
		using D = tiles::TilesGeneric_DomainStack<4,4,true,true,1>;
		using Alg_t = algorithm::Bugsy2<D>;
		//using Alg_t = algorithm::Astar2Impl<D, algorithm::Astar2SearchMode::Standard, algorithm::Astar2HrMode::DomainHr>;
		
		Json jDomConfig;
		jDomConfig["kept"] = std::vector<int>{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
		jDomConfig["goal"] = std::vector<int>{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
		jDomConfig["init"] = std::vector<int>{3,14,9,11,5,4,8,2,13,12,6,7,10,1,15,0};
		
		D domStack(jDomConfig);
		//domStack.setInitState(domStack.randInitState(0));

		typename D::template Domain<0> testDom(domStack);
		//testDom.mManhattan.dump(std::cout);
		//testDom.prettyPrintState(domStack.getInitState(), std::cout);
		//return;
		
		Alg_t alg(domStack, Json());
		
		alg.execute(domStack.getInitState());
		
		{
			std::ofstream ofs("bugsy_test.json");
			fast_assert(ofs);
			ofs << alg.report().dump(2);
		}
	}
}



int main(int argc, const char* argv[]) {

	mjon661::run15();
}
