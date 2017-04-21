
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include "util/json.hpp"
#include "util/time.hpp"

#include "search/astar_testing.hpp"
#include "search/bugsy_testing.hpp"
#include "search/debug_walker.hpp"

#include "domain/gridnav/dim2/fwd.hpp"

namespace mjon661 {
	

	

	static void run() {

		using BaseDomain_t = gridnav::dim2::fourway::BaseDomain<false, true>;
		using D = gridnav::dim2::DomainStack_StarAbt<BaseDomain_t, 6>;
		
		
		gridnav::dim2::CellMap2D<> cellmap;
		cellmap.setRandom(1000,1000,0,0.35);
		
		gridnav::dim2::StarAbtInfo<BaseDomain_t::Cost> abtinfo(2);
		abtinfo.init<BaseDomain_t::AdjEdgeIterator>(cellmap);
		
		{
			std::ofstream out("out.txt");
			cellmap.drawCells(out);
			abtinfo.draw(cellmap, out);

			out << abtinfo.getNLevels() << "\n";
		}
		

		D domStack(cellmap, abtinfo, Json());
		
		for(unsigned i=0; i<10; i++) {
			auto sp = domStack.genRandomInitAndGoal(1000, 5000, i);
			//~ std::cout << "init: " << sp.first << " " << sp.first % 1000 << " " << sp.first / 1000 << "\n";
			//~ std::cout << "goal: " << sp.second << " " << sp.second % 1000 << " " << sp.second / 1000 << "\n";
			std::cout << "{" << sp.first << "," << sp.second << "},\n";
		}
		
		//~ domStack.setInitAndGoal(sp.first, sp.second);
		
		//~ using Alg_t = algorithm::DebugWalker<D>;
		
		//~ Alg_t alg(domStack, Json());
		
		//~ alg.execute(domStack.getInitState());
	}
}



int main(int argc, const char* argv[]) {

	mjon661::run();

}
