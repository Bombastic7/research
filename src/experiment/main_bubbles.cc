

#include "search/debug_walker.hpp"
#include "domain/bubbles/domain_stack.hpp"


namespace mjon661 {
	static void run() {
		
		using D = bubbles::DomainStack<4,4,3>;
		using Alg_t = algorithm::DebugWalker<D>;
		
		D domStack;
		domStack.setInitState(0);
		
		Alg_t alg(domStack, Json());
		
		alg.execute(domStack.getInitState());
	
	}
	
}



int main() {
	mjon661::run();
}
