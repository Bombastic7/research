
#include <vector>

#include "util/json.hpp"
#include "util/debug.hpp"

#include "domain/tiles/fwd.hpp"

#include "experiment/noncog_search.hpp"



static std::vector<int> korf_15tiles_100instances(unsigned i) {
	switch(i) {
		case 1:
			return {14,13,15,7,11,12,9,5,6,0,2,1,4,8,10,3}; //57
		case 2:
			return {13,5,4,10,9,12,8,14,2,3,7,1,0,15,11,6}; //55
		case 3:
			return {14,7,8,2,13,11,10,4,9,12,5,0,3,6,1,15}; //59
		case 4:
			return {5,12,10,7,15,11,14,0,8,2,1,13,3,4,9,6}; //56
		case 5:
			return {4,7,14,13,10,3,9,12,11,5,6,15,1,2,8,0}; //56
		case 6:
			return {14,7,1,9,12,3,6,15,8,11,2,5,10,0,4,13}; //52
		case 7:
			return {2,11,15,5,13,4,6,7,12,8,10,1,9,3,14,0}; //52
		case 8:
			return {12,11,15,3,8,0,4,2,6,13,9,5,14,1,10,7}; //50
		case 9:
			return {3,14,9,11,5,4,8,2,13,12,6,7,10,1,15,0}; //46
		case 10:
			return {13,11,8,9,0,15,7,10,4,3,6,14,5,12,2,1}; //59
		default:
			gen_assert(false);
	}
	
	return {};
}



namespace mjon661 { namespace tiles {
	
	
	static void run() {
		TilesGeneric_DomainStack<3,3,true,false,5> domStack((Json()));
		domStack.setInitState(korf_15tiles_100instances(1));
		
		Json jReport;
		jReport["1"] = Json();
		run_astar(domStack, jReport.at("1"));
		
		
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
