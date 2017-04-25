

#include "search/debug_walker.hpp"
#include "search/astar.hpp"
#include "domain/bubbles/domain_stack.hpp"


namespace mjon661 {
	static void run() {
		
		using D = bubbles::DomainStack<6,4,2>;

		D domStack;
		
		
		//~ algorithm::DebugWalker<D> alg(domStack, Json());
		//~ alg.execute(domStack.getInitState());
		
		algorithm::Astar<D, algorithm::AstarSearchMode::Standard, algorithm::AstarHrMode::DomainHr> alg(domStack, Json());
		
		unsigned r=0;
		for(; r<100; r++) {
			domStack.setInitState(r);
			try {
				alg.execute(domStack.getInitState());
				break;
			} catch(NoSolutionException const&) {}
		}
		
		std::cout << alg.report().dump(2) << "\n" << "initState=" << r << "\n";
	
	}
	
}



int main() {
	mjon661::run();
}
