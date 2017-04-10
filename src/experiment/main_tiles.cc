
#include <algorithm>
#include <random>
#include <vector>

#include "util/json.hpp"
#include "util/debug.hpp"

#include "domain/tiles/fwd.hpp"

#include "experiment/noncog_search.hpp"



//~ static std::vector<int> korf_15tiles_100instances(unsigned i) {
	//~ switch(i) {
		//~ case 0:
			//~ return {14,13,15,7,11,12,9,5,6,0,2,1,4,8,10,3}; //57
		//~ case 1:
			//~ return {13,5,4,10,9,12,8,14,2,3,7,1,0,15,11,6}; //55
		//~ case 2:
			//~ return {14,7,8,2,13,11,10,4,9,12,5,0,3,6,1,15}; //59
		//~ case 3:
			//~ return {5,12,10,7,15,11,14,0,8,2,1,13,3,4,9,6}; //56
		//~ case 4:
			//~ return {4,7,14,13,10,3,9,12,11,5,6,15,1,2,8,0}; //56
		//~ case 5:
			//~ return {14,7,1,9,12,3,6,15,8,11,2,5,10,0,4,13}; //52
		//~ case 6:
			//~ return {2,11,15,5,13,4,6,7,12,8,10,1,9,3,14,0}; //52
		//~ case 7:
			//~ return {12,11,15,3,8,0,4,2,6,13,9,5,14,1,10,7}; //50
		//~ case 8:
			//~ return {3,14,9,11,5,4,8,2,13,12,6,7,10,1,15,0}; //46
		//~ case 9:
			//~ return {13,11,8,9,0,15,7,10,4,3,6,14,5,12,2,1}; //59
		//~ default:
			//~ gen_assert(false);
	//~ }
	
	//~ return {};
//~ }


static std::vector<int> tiles8_instances(unsigned i) {
	switch(i) {
		case 0:
			return {7,6,4,1,5,3,8,0,2,};
		case 1:
			return {7,6,5,1,2,8,0,4,3,};
		case 2:
			return {3,0,1,8,6,7,4,2,5,};
		case 3:
			return {5,4,3,7,1,8,0,2,6,};
		case 4:
			return {2,0,5,1,8,7,6,4,3,};
		case 5:
			return {5,0,7,6,8,4,1,2,3,};
		case 6:
			return {7,1,8,5,6,0,3,2,4,};
		case 7:
			return {5,0,2,8,3,7,4,6,1,};
		case 8:
			return {1,6,8,3,0,7,5,4,2,};
		case 9:
			return {0,8,3,6,5,4,7,1,2,};
		default:
			gen_assert(false);
	}
	
	return {};
}



namespace mjon661 { namespace tiles {
	
	
	//~ static void gen_8problems() {
		//~ BoardState<3,3> initstate(std::vector<int>{0,1,2,3,4,5,6,7,8});
		//~ BoardState<3,3> goalstate = initstate;
		//~ std::random_device rd;
		//~ std::mt19937 gen(rd());
		
		//~ for(unsigned i=0; i<10; i++) {
			//~ std::shuffle(initstate.begin(), initstate.end(), gen);
			//~ if(!isSolvable(initstate, goalstate)) {
				//~ i--;
				//~ continue;
			//~ }
			
			//~ std::cout << "return {";
			//~ for(auto t : initstate) {
				//~ std::cout << t << ",";
			//~ }
			//~ std::cout << "};\n";
		//~ }
	//~ }
	
	static void run() {
		//gen_8problems();
		Json jDomConfig;
		jDomConfig["kept"] = {1,1,1,1,1,1,1,1};
		jDomConfig["goal"] = {0,1,2,3,4,5,6,7,8};
		
		TilesGeneric_DomainStack<3,3,true,false,5> domStack(jDomConfig);
		domStack.setInitState(tiles8_instances(0));
		
		Json jReport;
		
		for(unsigned i=0; i<10; i++) {
			std::string ps = std::to_string(i);
			jReport[ps] = Json();
			run_astar(domStack, jReport.at(ps));
			run_greedy(domStack, jReport.at(ps));
			run_speedy(domStack, jReport.at(ps));
		}
		
		{
			std::ofstream ofs("tiles_res.json");
			fast_assert(ofs);
			ofs << jReport.dump(2);
		}
	}
	
	
	
	
	
	
	
}}



int main(int argc, const char* argv[]) {
	mjon661::tiles::run();
}
