#pragma once

#include "domain/gridnav/blocked/hypernav.hpp"



namespace mjon661 { namespace gridnav { namespace hypernav_blocked {
	
	
	
	
	
	
	
	template<unsigned N>
	void test_statepack() {
		
		using D_2 = gridnav::hypernav_blocked::TestDomainStack<2>;
		
		Json jConfig;
		jConfig["map"] = "-";
		jConfig["dimsz"] = std::vector<unsigned>{5,5};
		
		D_2 domStack(jConfig);
		
		typename D_2::template Domain<0> dom(domStack);
		
		
		
		for(unsigned i=0; i<5; i++) {
			for(unsigned j=0; j<5; j++) {
				StateN<2> s_in{i,j};
				
				dom.prettyPrintState(s_in, std::cout);
				std::cout << " " << dom.checkGoal(s_in) << "\n";
				
				auto edgeIt = dom.getAdjEdges(s_in);

			

				unsigned n=0;
				while(!edgeIt.finished()) {
					std::cout << n << " ";
					edgeIt.prettyPrint(std::cout);
					std::cout << " " << edgeIt.cost() << " ";
					dom.prettyPrintState(edgeIt.state(), std::cout);
					std::cout << " " << dom.checkGoal(edgeIt.state());
					edgeIt.next();
					
					std::cout << "\n";
				}
				
				std::cout << "Finished. s=";
				dom.prettyPrintState(s_in, std::cout);
				std::cout << "\n\n\n";
					
			}
		}
		
		
	}
	
	
	
	
}}}
