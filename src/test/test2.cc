
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include "util/json.hpp"

//#include "search/astar2.hpp"
//#include "search/enum_util_results.hpp"
//#include "search/bugsy_abt_lin.hpp"
//#include "search/bugsy_abt_exp1.hpp"
#include "search/debug_walker.hpp"
//#include "domain/tiles/fwd.hpp"

#include "domain/gridnav/dim2/gridnav2d.hpp"


namespace mjon661 {
	
	
	//~ static void makeAbtStack() {
			//~ using CellGraph_t = gridnav::dim2::CellGraph<4,false,true>;
		
		
	//~ }
	

	static void run() {
		//gridnav::dim2::StarAbtStackInfo<gridnav::dim2::CellGraph<4,false,false>> abtStackInfo(20,20,2,",fill");
		
		//abtStackInfo.dumpToFile("gridnav_test_dump.json");
		
		
		using CellGraph_t = gridnav::dim2::CellGraph<4,false,true>;
		using D = gridnav::dim2::GridNav_StarAbtStack<CellGraph_t, 3>;
		
		using Alg_t = algorithm::DebugWalker<D>;
		
		Json jDomConfig;
		jDomConfig["height"] = 5;
		jDomConfig["width"] = 5;
		jDomConfig["map"] = ",random,0,0.35";
		
		D domStack(jDomConfig);
		
		{
			std::ofstream ofs("gridnav_dump.txt");
			fast_assert(ofs);
			typename D::template Domain<0> dom(domStack);
			
			dom.drawCells(ofs);
		}
		
		gridnav::dim2::StarAbtStackInfo<CellGraph_t> starAbtInfo(jDomConfig.at("height"), jDomConfig.at("width"), 2, jDomConfig.at("map"));
		
		starAbtInfo.dumpToFile("gridnav_dump.json");
		
		starAbtInfo.drawAll(std::cout);
		
		
		
		//gridnav::dim2::StarAbtStackInfo<CellGraph_t> starAbtInfo2("gridnav_dump.json");
		
		///fast_assert(starAbtInfo == starAbtInfo2);	
///starAbtInfo2.drawAll(std::cout);
		
		
		
		//std::cout << starAbtInfo.hash() << "\n" << starAbtInfo2.hash() << "\n";
		
		Alg_t alg(domStack, Json());
		
		alg.execute(domStack.getInitState());
	}
}



int main(int argc, const char* argv[]) {

	mjon661::run();
}
