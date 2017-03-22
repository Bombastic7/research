
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <utility>
#include "domain/gridnav/blocked/graph.hpp"
#include "domain/pancake/fwd.hpp"
#include "domain/tiles/fwd.hpp"
//#include "domain/star_abt.hpp"

#include "util/debug.hpp"
#include "util/json.hpp"

//#include "search/hastar/v2/hastar.hpp"

#include "search/solution.hpp"

#include "domain/test_graph.hpp"
#include "search/ugsa/v6/ugsa.hpp"

#include "search/admissible_abtsearch2.hpp"
#include "search/astar.hpp"
#include "search/bugsy.hpp"
#include "search/debug_walker.hpp"

#include "domain/gridnav/hypernav/hypernav_real.hpp"
#include "search/astar2.hpp"


namespace mjon661 {
	
	void test_moves() {
		using D = gridnav::hypernav::DomainRealStack<3, 3>;
		
		Json jDomConfig;
		jDomConfig["map"] = ",random_hills,100,0,10,100,20";
		jDomConfig["dimsz"] = std::vector<unsigned>{100,100,100};
		
		D domStack(jDomConfig);
		typename D::template Domain<0> dom(domStack);
		
		unsigned n=0;
		
		std::array<unsigned, 3> s = {1,1,1};
		
		for(auto it=dom.getAdjEdges(s); !it.finished(); it.next()) {
			std::cout << n++ << ": ";
			dom.prettyPrintState(it.state(), std::cout);
			std::cout << "   " << it.cost() << "\n";
		}
	}
	
	
	struct AstarExpandedNodesSort {
		
		template<typename Node>
		bool operator()(Node* a, Node* b) const {
			return a->expn < b->expn;
		}
	};
	
	void test_hypernav_real() {
		using D = gridnav::hypernav::DomainRealStack<2, 2>;
		using Alg_t = algorithm::Astar2Impl<D, algorithm::Astar2SearchMode::Speedy>;
		
		Json jDomConfig;
		jDomConfig["map"] = ",random_cones,100,0,10,20";
		jDomConfig["dimsz"] = std::vector<unsigned>{100,100};
		
		D domStack(jDomConfig);
		domStack.assignInitGoalStates({{50,50}, {99,99}});
		
		typename D::template Domain<0> dom(domStack);
		
		Alg_t alg(domStack, Json());
		
		alg.execute(domStack.getInitState());
		
		std::vector<typename Alg_t::Node*> expNodes;
		
		for(auto it=alg.mClosedList.begin(); it!=alg.mClosedList.end(); ++it) {
			if((*it)->expn != (unsigned)-1)
				expNodes.push_back(*it);
		}
		
		std::sort(expNodes.begin(), expNodes.end(), AstarExpandedNodesSort());


		std::vector<unsigned> expStates;
		
		for(auto* n : expNodes)
			expStates.push_back(n->pkd);

		domStack.mCellMap.dumpHeatMap1(100,100);
		domStack.mCellMap.dumpHeatMap2(100,100, expStates);
		domStack.mCellMap.dumpHeatMap3(100,100, expStates);
		
		for(auto* n = alg.mGoalNode; n; n=n->parent) {
			unsigned x = n->pkd%100, y = n->pkd/100;
			std::cout << "(" << x << "," << y << ") " << dom.costHeuristic({x,y}) << " " << dom.distanceHeuristic({x,y}) << "\n";
		}
		
	}

}

int main(int argc, const char* argv[]) {
	mjon661::test_hypernav_real();
	//mjon661::test_moves();

}
