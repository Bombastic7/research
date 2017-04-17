#pragma once

#include <utility>
#include <string>
#include <vector>

#include "domain/gridnav/dim2/gridnav2d.hpp"



namespace mjon661 { namespace gridnav { namespace dim2 {


	//~ std::pair<std::string, unsigned> gridnav_maps(unsigned i) {
		
		
	//~ }


	//~ const unsigned Gridnav_Top_Abt_1k1k_random_0_35 = 

	//~ std::pair<unsigned,unsigned> gridnav_1k1k_35_0_instances(unsigned i) {
		//~ std::vector<std::pair<unsigned,unsigned>
		
		
	//~ }



	template<typename CG>
	void getMapInfo(unsigned pHeight, unsigned pWidth, std::string const& pMapStr) {
		CG cellgraph(pHeight, pWidth, pMapStr);
		StarAbt_Stack<CG> abtStack(cellgraph, 2);
		
		std::cout << pHeight << " " << pWidth << " " << pMapStr << " : topabt=" << abtStack.getLevelsInfo().size()-1 << "\n";
	}
	
	template<typename CG, unsigned Top_Abt>
	void genProblems(unsigned pHeight, unsigned pWidth, std::string const& pMapStr, unsigned pNprobs) {
		
		Json jDomConfig;
		jDomConfig["height"] = pHeight;
		jDomConfig["width"] = pWidth;
		jDomConfig["map"] = pMapStr;
		jDomConfig["abt_radius"] = 2;
		
		GridNav_StarAbtStack<CG, Top_Abt> dom(jDomConfig);
		
		for(unsigned i=0; i<pNprobs; i++) {
			std::pair<unsigned, unsigned> sp = dom.genRandInitAndGoal(i);
			std::cout << sp.first << " " << sp.second << "\n";
		}
	}


}}}
