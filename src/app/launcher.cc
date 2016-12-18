
#include <fstream>
#include <string>
#include "util/json.hpp"


#include "app/routines.hpp"


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
	
	mjon661::execution_single::selectAlgorithm<>(jExecDesc);
	
	return 0;
}
