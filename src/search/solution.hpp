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
			
			State s0 = states[0];
			
			Domain dom(pDomStack);

			
			Edge prevEdge = dom.createEdge(s0, operators.at(0));
			Cost pthcst = prevEdge.cost();
			
			for(unsigned i=1; i<operators.size(); i++) {
				Edge e = dom.createEdge(prevEdge.state(), operators.at(i));
				pthcst += e.cost();
				prevEdge = e;
			}
			
			fast_assert(dom.checkGoal(prevEdge.state()));
			
			return pthcst;
		}

		
		std::vector<State> states;
		std::vector<Operator> operators;
	};
}
