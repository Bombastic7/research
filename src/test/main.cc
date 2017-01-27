#include <vector>
#include <iostream>
#include <string>

#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "domain/tiles/fwd.hpp"
#include "domain/pancake/fwd.hpp"
#include "domain/gridnav/fwd.hpp"

#include "search/debug_walker.hpp"


#include "util/json.hpp"
#include "app/platform/resource.hpp"

#include "domain/tiles/mdist.hpp"

#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>

#include "util/util.hpp"


namespace mjon661 { namespace tiles {
	
		
	void run() {
		
		Manhattan<4,4,true> mhtn(std::vector<tile_t>{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15});
		
		mhtn.dump(std::cout);
		std::cout << "\n";
		
		cost_t h, d;
		mhtn.eval(std::vector<tile_t>{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}, h, d);
		std::cout << h << " " << d << "\n";
		mhtn.eval(std::vector<tile_t>{4,1,2,3,0,5,6,7,8,9,10,11,12,13,14,15}, h, d);
		std::cout << h << " " << d << "\n";
		
		mhtn.increment(0, 4, 4, h, d);
		std::cout << h << " " << d << "\n";
	}
	
}}


namespace mjon661 { namespace tiles {
	
	
	void dbg_tiles() {
		
		using Dom_t = TilesGeneric_DomainStack<3,3,true,true,5>;
		
		Json j;
		
		j["init"] = std::vector<int>{0,1,2,3,4,5,6,7,8};
		j["goal"] = std::vector<int>{0,1,2,3,4,5,6,7,8};
		
		Dom_t dom(j);
		
		algorithm::DebugWalker<Dom_t> dbgwlkr(dom);
		
		dbgwlkr.execute();
	}
	
}}/* namespace pancake {
	
	void dbg_pancake() {
		
		using Dom_t = Pancake_DomainStack_IgnoreAbt<10, 5, 1>;
		
		Json j;
		
		j["init"] = std::vector<int>{0,1,2,3,4,5,6,7,8,9};
		
		Dom_t dom(j);
		
		algorithm::DebugWalker<Dom_t> dbgwlkr(dom);
		
		dbgwlkr.execute();
	}
	
} namespace gridnav {
	
	void dbg_gridnav() {
		
		using Dom_t = blocked::GridNav_DomainStack_single<1000, 1000, true, true, true>;
		Json j;
		
		j["init"] = std::vector<int>{0,999};
		j["goal"] = std::vector<int>{0,0};
		j["map"] = std::string("mapA");
		
		Dom_t dom(j);
		
		algorithm::DebugWalker<Dom_t> dbgwlkr(dom);
		
		dbgwlkr.execute();
	}
	
}}
*/



int main(int argc, const char* argv[]) {
	
	using mjon661::OptData;
	
	OptData<int, true> optT;
	OptData<int, false> optF;
	
	optT() = 1;
	optF() = 1;
	
	int i = optT();
	i = optF();
	
	bool b = optT() == optF();
	
	std::cout << b << " " << i << "\n";
}
