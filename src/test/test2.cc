
#include <iostream>
#include <vector>
#include <string>

#include "util/json.hpp"

#include "search/astar2.hpp"
#include "search/bugsy_abt_lin.hpp"
#include "search/debug_walker.hpp"
#include "domain/tiles/fwd.hpp"


namespace mjon661 {
	
	
	
	std::vector<int> korf_15tiles_100instances(unsigned i) {
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
	
	
	
	
	
	static void run(unsigned pProbN) {
		using D = tiles::TilesGeneric_DomainStack<4,4,true,false,8>;
		
		Json jDomConfig;
		
		jDomConfig["goal"] = std::vector<unsigned>{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
		//jDomConfig["kept"] = std::vector<unsigned>{  1,2,1,1,2,1,2,2,1,2 ,1 ,1 ,2 ,1 ,2 };
		jDomConfig["kept"] = std::vector<unsigned>{  8,7,6,5,4,3,2,1,1, 1, 1, 1, 1, 1, 1};
		//jDomConfig["init"] = std::vector<unsigned>{14,13,15,7,11,12,9,5,6,0,2,1,4,8,10,3};

		jDomConfig["init"] = korf_15tiles_100instances(pProbN);
		D domStack(jDomConfig);
		
		
		

		{
			Json jRes;
			algorithm::Astar2Impl<D, algorithm::Astar2SearchMode::Standard, algorithm::Astar2HrMode::AbtHr> astar_abthr_alg(domStack, Json());

			try {
				astar_abthr_alg.execute(domStack.getInitState());
			} catch(std::exception const& e) {
				jRes["failed"] = e.what();
			}
			
			jRes["astar_abthr"] = astar_abthr_alg.report();
			
			std::cout << jRes.dump(4) << "\n";
		}
		
		
		{
			Json jRes, jAlgConfig;
			
			jAlgConfig["wf"] = 1;
			jAlgConfig["wt"] = 1;
			jAlgConfig["fixed_exptime"] = 3e-6;
			
			algorithm::bugsy::BugsyAbtSearchBase<D, algorithm::bugsy::BugsyAbtSearchBase_Imitate::HAstar_cost> bugsy_alg(domStack, jAlgConfig);
			bugsy_alg.execute(domStack.getInitState());
			jRes["bugsy_abt_cost"] = bugsy_alg.report();
			std::cout << jRes.dump(4) << "\n";
		}
		
		{
			Json jRes, jAlgConfig;
			
			jAlgConfig["wf"] = 1;
			jAlgConfig["wt"] = 1;
			jAlgConfig["fixed_exptime"] = 3e-6;
			
			algorithm::bugsy::BugsyAbtSearchBase<D, algorithm::bugsy::BugsyAbtSearchBase_Imitate::HAstar_dist> bugsy_alg(domStack, jAlgConfig);
			bugsy_alg.execute(domStack.getInitState());
			jRes["bugsy_abt_dist"] = bugsy_alg.report();
			std::cout << jRes.dump(4) << "\n";
		}
	}
	
	
	
	static void run_dbgwalker() {
		using D = tiles::TilesGeneric_DomainStack<4,4,true,false,8>;
		
		Json jDomConfig;
		
		jDomConfig["goal"] = std::vector<unsigned>{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
		//jDomConfig["kept"] = std::vector<unsigned>{  1,2,1,1,2,1,2,2,1,2 ,1 ,1 ,2 ,1 ,2 };
		jDomConfig["kept"] = std::vector<unsigned>{  8,7,6,5,4,3,2,1,1, 1, 1, 1, 1, 1, 1};
		jDomConfig["init"] = std::vector<unsigned>{1,0,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
		D domStack(jDomConfig);
		
		algorithm::DebugWalker<D> dbgwlk_alg(domStack, Json());
		
		dbgwlk_alg.execute(domStack.getInitState());
		
	}
	
	
}



int main(int argc, const char* argv[]) {
	if(argc > 1)
		try {
			mjon661::run(std::stoul(std::string(argv[1])));
		} catch(std::exception const& e) {
			std::cout << e.what() << "\n";
		}
	else
		mjon661::run_dbgwalker();
}
