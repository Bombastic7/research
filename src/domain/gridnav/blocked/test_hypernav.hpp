#pragma once

#include "domain/gridnav/blocked/hypernav.hpp"



namespace mjon661 { namespace gridnav { namespace hypernav_blocked {
	
	
	
	
	
	
	
	template<unsigned N>
	void test_statepack() {
		
		using D_3 = gridnav::hypernav_blocked::TestDomainStack<3,1>;
		
		std::array<unsigned, 3> dimSz{30, 60, 90};
		Json jConfig;
		jConfig["map"] = "-";
		jConfig["dimsz"] = dimSz;
		
		D_3 domStack(jConfig);
		
		typename D_3::template Domain<0> dom(domStack);
		
		
		
		for(unsigned i=0; i<dimSz[0]; i++) {
			for(unsigned j=0; j<dimSz[1]; j++) {
			for(unsigned m=0; m<dimSz[2]; m++) {
				StateN<3> s_in{i,j,m}, s_out;
				
				
				PackedStateN pkd = doPackState<3>(s_in, dimSz);
				s_out = doUnpackState<3>(pkd, dimSz);
				
				
				if(s_in != s_out) {
					typename D_3::template Domain<0> dom(domStack);
					
					dom.prettyPrintState(s_in, std::cout);
					std::cout << " " << pkd << " ";
					dom.prettyPrintState(s_out, std::cout);
					std::cout << "\n";
					gen_assert(false);
				}
			}
		}
		}
		
		
	}
	
	
	
	
}}}
