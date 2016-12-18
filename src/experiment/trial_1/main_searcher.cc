#include <iostream>
#include <fstream>
#include <string>
#include "util/json.hpp"


void run_trial1(mjon661::Json const&, std::string const&);


int main(int argc, const char* argv[]) {
	
	if(argc <= 2) {
		std::cout << "USAGE: " << argv[0] << " exec_desc_set key\n";
		std::cout << "Print JSON search result object to stdout if no limits are reached.\n";
		std::cout << "Returns 0 on success (time/mem limit not reached)\n";
		std::cout << "Returns 10 on time limit reached, 11 on memory limit reached\n.";
		std::cout << "If exec_desc_set[key] contains key \"memory limit dump\", whatever stats are there will be output to file.\n.";
		return 1;
	}
	
	std::ifstream execDescSetIn(argv[1]);
	
	if(!execDescSetIn) {
		std::cout << "Could not open " << argv[1] <<  "\n";
		return 1;
	}
	
	mjon661::Json jExecDescSet;
	execDescSetIn >> jExecDescSet;
	
	
	std::string key(argv[2]);
	
	if(jExecDescSet.count(key) != 1) {
		std::cout << "Bad key\n";
		return 1;
	}
	
	
	run_trial1(jExecDescSet, key);
	
	return 0;
}
