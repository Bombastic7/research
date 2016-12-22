#pragma once

#include <string>
#include <iostream>
#include <cstdlib>

#include "search/basic_astar.hpp"
#include "util/json.hpp"

namespace mjon661 { namespace algorithm {
	
	
	template<typename DomStack, unsigned L>
	struct PrintStatesAndOps {
		
		using Domain = typename DomStack::template Domain<L>;
		using State = typename Domain::State;
		using Operator = typename Domain::Operator;
		
		
		static void doPrint(Domain& dom, State& s) {
			
			dom.prettyPrint(s, std::cout);
			
		}
		
		static void doPrint(Domain& dom, Operator op) {
			dom.prettyPrint(op, std::cout);
			
		}
	};




	template<typename DomStack, unsigned L, bool = true>
	struct DebugWalkerImpl : public DebugWalkerImpl<DomStack, L+1, L+1 <= DomStack::Top_Abstract_Level> {
		
		using Domain = typename DomStack::template Domain<L>;
		using State = typename Domain::State;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using Edge = typename Domain::Edge;
		
		template<bool>
		struct Tag {};
		
		
		DebugWalkerImpl(DomStack& pStack) :
			DebugWalkerImpl<DomStack, L+1, L+1 <= DomStack::Top_Abstract_Level>(pStack),
			mDomain(pStack),
			mDomStack(pStack)
		{
			
			
			
		}
		
		
		void doWalk(State& s) {
			while(true) {
				std::cout << "\nLEVEL " << std::to_string(L) << "\n\n";
				
				PrintStatesAndOps<DomStack, L>::doPrint(mDomain, s);
				
				std::cout << "\n\n";
				
				if(mDomain.checkGoal(s))
					std::cout << "Is Goal\n\n";
				
				OperatorSet opSet = mDomain.createOperatorSet(s);
				
				std::cout << "Operators:\n\n";
				
				for(unsigned i=0; i<opSet.size(); i++) {
					
					Operator op = opSet[i];
					
					std::cout << i << ":\n";
					PrintStatesAndOps<DomStack, L>::doPrint(mDomain, op);
					
					{
						Edge edge = mDomain.createEdge(s, opSet[i]);
						std::cout << "\nCost: " << edge.cost() << "\n";
						mDomain.destroyEdge(edge);
					}
					
					std::cout << "\n\n";
				}
				
				
				
				std::cout << "\nCOMMAND (e opn, a, p): ";
				
				std::string comnd;
				
				std::getline(std::cin, comnd);
				

				if(comnd == "p")
					break;
					
				else if(comnd == "a") {
					doAbstraction(s, Tag<(L < DomStack::Top_Abstract_Level)>{});
					
				}
				else if(comnd[0] == 'e' && comnd.size() > 2) {
					
					
					
					unsigned opn = strtol(comnd.c_str() + 2, nullptr, 10);
					if(opn < opSet.size()) {
						
						Edge edge = mDomain.createEdge(s, opSet[opn]);
						doWalk(edge.state());
						mDomain.destroyEdge(edge);
						
					}
				}
				
			}
		}
		

		void doAbstraction(State& baseState, Tag<true>) {
			static_assert(L < DomStack::Top_Abstract_Level, "");
			
			typename DomStack::template Abstractor<L> abtor(mDomStack);
			
			auto abtState = abtor(baseState);
			
			DebugWalkerImpl<DomStack, L+1, L+1 <= DomStack::Top_Abstract_Level>::doWalk(abtState);
		}
		

		void doAbstraction(State& baseState, Tag<false>) {
			static_assert(L == DomStack::Top_Abstract_Level, "");
			
			std::cout << "Abstraction not available.\n\n";
		}
		
		
		Domain mDomain;
		DomStack& mDomStack;
	};


	template<typename DomStack, unsigned L>
	struct DebugWalkerImpl<DomStack, L, false> {
		DebugWalkerImpl(DomStack&) {}
	};



	template<typename DomStack>
	struct DebugWalker {
		
		DebugWalker(DomStack& pDomStack) :
			mImpl(pDomStack),
			mDomStack(pDomStack)
		{}
		
		void execute() {
			typename DomStack::template Domain<0> dom(mDomStack);
			
			auto s0 = dom.createState();
			
			mImpl.doWalk(s0);
		}
		
		
		DebugWalkerImpl<DomStack, 0> mImpl;
		DomStack& mDomStack;
	
	};
}}
