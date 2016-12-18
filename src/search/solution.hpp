#pragma once

#include <iostream>
#include <vector>


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
			
			State s = states[0];
			
			Domain dom(pDomStack);
			
			return pathCostRec(dom, s, 0);
		}
		
		

		
		Cost pathCostRec(Domain& pDomain, State& pState, unsigned i) {
			if(i == operators.size())
				return 0;
			
			Edge edge = pDomain.createEdge(pState, operators[i]);
			Cost retCost = edge.cost() + pathCostRec(pDomain, edge.state(), i+1);
			pDomain.destroyEdge(edge);
			
			return retCost;
		}
		
		std::vector<State> states;
		std::vector<Operator> operators;
	};
}
