#pragma once

#include <iostream>
#include <vector>

#include "util/debug.hpp"


namespace mjon661 {
	template<typename Domain>
	struct Solution {
		using Cost = typename Domain::Cost;
		using State = typename Domain::State;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using Edge = typename Domain::Edge;
		
		
		
		void printSolution(Domain pDomain, std::ostream& out) {	
			
			for(unsigned i=0; i<states.size(); i++) {
				out << "STATE " << i << ": \n";
				pDomain.prettyPrint(states[i], out);
				
				out << "\n";
				
				if(i < operators.size()) {
					out << "OP " << i << ": \n";
					pDomain.prettyPrint(operators[i], out);
					out << "\n";
				}
			}
		}
		
		template<typename D>
		Cost pathCost(D& pDomStack) {

			if(states.size() == 0)
				return 0;
			
			fast_assert(states.size()-1 == operators.size());
			
			State s0 = states[0];
			
			Domain dom(pDomStack);

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
