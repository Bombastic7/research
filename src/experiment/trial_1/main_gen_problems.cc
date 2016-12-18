#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <stdexcept>

#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/time.hpp"

#include "domain/tiles/common.hpp"



/*
 * Generate problem instances.
 * 
 * jExecDesc:
 * 	domain : (string) tiles/pancake/gridnav
 * 	
 * 		tiles: (string) "4"/"8"
 * 		
 * 			
 * 
 * 
 * 
 */





namespace mjon661 {

	void pancake_gen(Json const& jExecDesc) {
		
		unsigned nCakes = jExecDesc.at("cakes");
		unsigned nProblems = jExecDesc.at("problems");
		
		std::vector<int> p(nCakes);
		
		for(unsigned i=0; i<nCakes; i++)
			p[i] = i;
		
		std::random_device rd;
		std::mt19937 gen(rd());
		
		Json j;
		
		for(unsigned i=0; i<nProblems; i++) {
			std::shuffle(p.begin(), p.end(), gen);
			j[std::to_string(i)]["init"] = p;
		}
		
		std::cout << j.dump(4);
	}
	
	

	
	
	void tiles_gen(Json const& jExecDesc) {
		std::string boardSz = jExecDesc.at("board");
		unsigned nProblems = jExecDesc.at("problems");
		
		Json j;
		
		if(boardSz == "8") {
			tiles::BoardStateV<3,3> goalBoard; //Identity state
		
			tiles::ProblemGenerator<3,3> pgen(goalBoard);
			
			for(unsigned i=0; i<nProblems; i++) {
				//tiles::BoardValues<9> initState = pgen();
				j[std::to_string(i)]["init"] = pgen();
				j[std::to_string(i)]["goal"] = goalBoard;
			}
		} 
		
		else if(boardSz == "15") {
			tiles::BoardStateV<4,4> goalBoard; //Identity state
		
			tiles::ProblemGenerator<4,4> pgen(goalBoard);
			
			for(unsigned i=0; i<nProblems; i++) {
				j[std::to_string(i)]["init"] = pgen();
				j[std::to_string(i)]["goal"] = goalBoard;
			}
		}
		
		else
			throw std::runtime_error("Bad board");
	
	
		std::cout << j.dump(4);
	}
	
	
	
	
	void gridnav_genMap(Json const& jExecDesc) {
		
		unsigned height = jExecDesc.at("height");
		unsigned width = jExecDesc.at("width");
		
		unsigned nProblems = jExecDesc.at("problems");
		double blockedProb = jExecDesc.at("blocked");
		
		std::string outfile = jExecDesc.at("out");
		
		gen_assert(blockedProb >= 0 && blockedProb <= 1);
		
		std::random_device rdev;
		
		std::mt19937 rgen(rdev());
		
		std::uniform_real_distribution<> randDist(0, 1.0);
		
		
		
		for(unsigned n = 0; n < nProblems; n++) {
			
			std::ofstream out(outfile + std::to_string(n));
			
			if(!out)
				throw std::runtime_error(std::string("Could not open ") + outfile);
			
			for(unsigned i=0; i<height*width; i++) {
				double d = randDist(rgen);
				
				if(d < blockedProb)
					out << "1 ";
				else
					out << "0 ";
				
				if(i % width == 0)
					out << "\n";
			}
		}
	}
}

void genProblems_trial1(mjon661::Json const& jExecDesc) {

	std::string domain = jExecDesc.at("domain");

	if(domain == "tiles")
		mjon661::tiles_gen(jExecDesc);
	else if(domain == "pancake")
		mjon661::pancake_gen(jExecDesc);
	else if(domain == "gridnav")
		mjon661::gridnav_genMap(jExecDesc);
	else
		throw std::runtime_error("Bad domain");
}




int main(int argc, const char* argv[]) {
	
	if(argc == 1) {
		std::cout << "USAGE: " << argv[0] << " <exec_desc>\n";
		return 1;
	}
	
	std::ifstream execDescIn(argv[1]);
	
	if(!execDescIn) {
		std::cout << " Could not open " << argv[1] <<  "\n";
		return 1;
	}
	
	mjon661::Json jExecDesc;
	execDescIn >> jExecDesc;
	
	genProblems_trial1(jExecDesc);
	
	return 0;
}
