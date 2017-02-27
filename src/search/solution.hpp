#pragma once

#include <iostream>
#include <vector>

#include "util/debug.hpp"


namespace mjon661 {
	template<typename D>
	struct Solution {
		using BD = typename D::template Domain<0>;
		using Cost = typename BD::Cost;
		using State = typename BD::State;
		using Operator = typename BD::Operator;
		using OperatorSet = typename BD::OperatorSet;
		using Edge = typename BD::Edge;
		
		
		
		void printSolution(D const& pStack, std::ostream& out) {	
			BD dom(pStack);
			
			for(unsigned i=0; i<states.size(); i++) {
				out << "STATE " << i << ": \n";
				dom.prettyPrint(states[i], out);
				
				out << "\n";
				
				if(i < operators.size()) {
					out << "OP " << i << ": \n";
					dom.prettyPrint(operators[i], out);
					out << "\n";
				}
			}
		}
		
		
		Cost pathCost(D& pStack) {

			if(states.size() == 0)
				return 0;
			
			fast_assert(states.size()-1 == operators.size());
			
			State s0 = states[0];
			
			BD dom(pStack);

			Cost pthcst = 0;
			
			for(unsigned i=0; i<operators.size(); i++) {
				State s = states.at(i);
				Edge e = dom.createEdge(s, operators.at(i));
				
				pthcst += e.cost();
				
				dom.destroyEdge(e);
			}
			
			return pthcst;
		}

		
		std::vector<State> states;
		std::vector<Operator> operators;
	};
}
