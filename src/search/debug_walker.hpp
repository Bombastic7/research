#pragma once

#include <string>
#include <iostream>
#include <cstdlib>
#include <string>
#include <utility>

#include "util/json.hpp"

namespace mjon661 { namespace algorithm {

	template<typename DomStack, unsigned L, unsigned Bound>
	struct DebugWalkerImpl {
		
		using Domain = typename DomStack::template Domain<L>;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Cost = typename Domain::Cost;
		
		
		DebugWalkerImpl(DomStack const& pStack) :
			mDomain(pStack),
			mNxtLevel(pStack)
		{}
		
		template<typename BS>
		void executeFromParentState(BS const& bs, unsigned expdTot) {
			State s0 = mDomain.abstractParentState(bs);
			doWalk(s0, expdTot);
		}
		
		void doWalk(State const& s, unsigned expdTot) {
			PackedState pkd;
			mDomain.packState(s, pkd);
			doWalk_packed(pkd, expdTot);
		}
		
		void doWalk_packed(PackedState const& pkd0, unsigned expdTot) {
			
			PackedState pkd = pkd0;
			unsigned expdThisLevel = 0;
			
			while(true) {
				State s;
				mDomain.unpackState(s, pkd);
				
				std::cout << "----------------\nLevel=" << L << "\nTotal expd=" << expdTot+expdThisLevel << "\nexpd=" << expdThisLevel << "\n";
				if(mDomain.checkGoal(s))
					std::cout << "goal=true\n";

				std::cout << "\n";
				
				mDomain.prettyPrintState(s, std::cout);
				std::cout << "\n\n";
				
				
				std::vector<std::pair<State, Cost>> adjEdges;
				
				for(auto edgeIt=mDomain.getAdjEdges(s); !edgeIt.finished(); edgeIt.next())
					adjEdges.push_back({edgeIt.state(), edgeIt.cost()});
					
				
				for(unsigned i=0; i<adjEdges.size(); i++) {
					std::cout << i << ": cost=" << adjEdges[i].second << "\n";
					mDomain.prettyPrintState(adjEdges[i].first, std::cout);
					std::cout << "\n\n";
				}

				std::string comnd;
				
				while(true) {
					std::cout << ":";
				
					std::getline(std::cin, comnd);

					if(comnd == "p") {
						std::cout << "\n\n";
						return;
					}
					else if(comnd == "a") {
						std::cout << "\n\n";
						mNxtLevel.executeFromParentState(s, expdTot+expdThisLevel);
					}
					else {
						//unsigned adjn = std::stol(comnd);
						char* p;
						unsigned adjn = std::strtoul(comnd.c_str(), &p, 10);
						if(p != comnd.c_str() + comnd.size())
							continue;
						
						mDomain.packState(adjEdges.at(adjn).first, pkd);
						expdThisLevel++;
						break;
					}
				}
			}
		}
		
		Domain mDomain;
		DebugWalkerImpl<DomStack, L+1, Bound> mNxtLevel;
	};


	template<typename DomStack, unsigned L>
	struct DebugWalkerImpl<DomStack, L, L> {
		DebugWalkerImpl(DomStack const&) {}
		
		template<typename BS>
		void executeFromParentState(BS const& bs, unsigned) {
			std::cout << "Reached hard abstraction limit.\n\n";
		}
	};



	template<typename DomStack>
	struct DebugWalker {
		
		DebugWalker(DomStack const& pDomStack, Json const& jConfig) :
			mImpl(pDomStack)
		{}
		
		void execute(typename DomStack::template Domain<0>::State const& pInitState) {
			
			mImpl.doWalk(pInitState, 0);
		}
		
		Json report() {
			return Json();
		}
		
		
		DebugWalkerImpl<DomStack, 0, DomStack::Top_Abstract_Level+1> mImpl;
	
	};
}}
