
#include <fstream>
#include <string>
#include <stdexcept>

#include "util/json.hpp"

#include "domain/gridnav/defs.hpp"
#include "domain/gridnav/gen.hpp"


namespace mjon661 {
	
	namespace gridnav {
		
		void run(Json const& jExecDesc) {
			
			if(jExecDesc["function"] == "generate") {
				if(jExecDesc["height"] == 10 && jExecDesc["width"] == 10) {
					
					CellArray<10, 10>* cellArray = new CellArray<10,10>;
					
					double blockedProb = jExecDesc.at("blocked prob");
					std::string pOutFile = jExecDesc.at("out");
					
					gen_simple_uniform<10, 10>(*cellArray, blockedProb);
					
					for(unsigned i=0; i<cellArray->size(); i++) {
						cell_t c = (*cellArray)[i];
						
						if(c != 0 && c != 1)
							throw std::runtime_error("");
					}
					
					std::ofstream ofs(pOutFile);
					
					if(ofs)
						cellArray->write(ofs);
					
					delete cellArray;
					
				}
				
			}
		}
		
	}
	
	void run(Json const& jExecDesc) {
		
		if(jExecDesc["domain"] == "gridnav")
			gridnav::run(jExecDesc);
		
	}
	
	
	
}



int main(int argc, const char* argv[]) {
	
	if(argc == 1) {
		std::cout << "USAGE: " << argv[0] << " exec_desc\n";
		return 1;
	}
	
	std::ifstream execDescIn(argv[1]);
	
	if(!execDescIn) {
		std::cout << " Could not open " << argv[1] <<  "\n";
		return 1;
	}
	
	mjon661::Json jExecDesc;
	execDescIn >> jExecDesc;
	
	mjon661::run(jExecDesc);
	
	return 0;
}
