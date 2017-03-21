#pragma once

#include <cstdint>
#include <cstdio>
#include "domain/gridnav/hypernav/hypernav.hpp"



namespace mjon661 { namespace gridnav { namespace hypernav {
	
	
	
	
	
	
	
	//~ template<unsigned N>
	//~ void test_statepack() {
		
		//~ using D_3 = gridnav::hypernav_blocked::TestDomainStack<3,3>;
		
		//~ std::array<unsigned, 3> dimSz{100,100,100};
		//~ Json jConfig;
		//~ jConfig["map"] = "-";
		//~ jConfig["dimsz"] = dimSz;
		
		//~ D_3 domStack(jConfig);
		
		//~ typename D_3::template Domain<0> dom(domStack);
		
		//~ StateN<3> s_in;
		//~ s_in.fill(0);
		//~ uint64_t h = 0, nstates = 0;
					
		//~ while(true) {
			//~ bool atEnd = true;
			
			//~ for(unsigned n=0; n<dimSz.size(); n++) {
				//~ s_in[n]++;
				//~ if(s_in[n] < dimSz[n]) {
					//~ atEnd = false;
					//~ break;
				//~ }
				
				//~ s_in[n] = 0;
			//~ }
			
			//~ if(atEnd)
				//~ break;
			
			

			
			//~ StateN<3> s_out;
				
				
			//~ PackedStateN pkd = doPackState<3>(s_in, dimSz);
			//~ s_out = doUnpackState<3>(pkd, dimSz);
				
			//~ gen_assert(s_in == s_out);


			//~ nstates++;


			//~ for(auto edgeIt = dom.getAdjEdges(s_in); !edgeIt.finished(); edgeIt.next()) {
				//~ pkd = doPackState(edgeIt.state(), dimSz);
				//~ h++;
				//~ //std::cout << h << "\n";
				//~ //getchar();
			//~ }


		//~ }
		//~ std::cout << h << " " << nstates << " " << (double)h / nstates << "\n";
	//~ }
	
	
	
	
}}}
