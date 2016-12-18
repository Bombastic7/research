#include <fstream>
#include <iostream>
#include "util/json.hpp"




int main(int argc, const char* argv[]) {

	if(argc == 1) {
		std::cout << "Usage: " << argv[0] << "file\nAttempts to parse file and reports results.\n";
		return 1;
	}

	std::ifstream ifs(argv[1]);
	
	if(!ifs) {
		std::cout << "Could not open " << argv[1] << " for reading.\n";
		return 2;
	}

	mjon661::Json j;
	
	try {
		ifs >> j;
	} catch(std::exception& e) {
		std::cout << "Failure: " << e.what() << "\n";
		return 3;
	}
	
	std::cout << "Success\n";
	return 0;
}
