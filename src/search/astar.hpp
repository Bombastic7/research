#pragma once

#include "search/basic_astar.hpp"
#include "util/json.hpp"

namespace mjon661 { namespace algorithm {

	template<typename DomStack>
	class Astar {
		public:
		
		using Astar_t =			AstarImpl<DomStack, true>;
		using Domain =			typename DomStack::template Domain<0>;
		using State = 			typename Domain::State;
		using Node =			typename Astar_t::Node;
			
		Astar(DomStack& pDomStack, Json const&) :
			mDomain(pDomStack),
			mAlgo(pDomStack)
		{}
		
		void execute(Solution<typename DomStack::template Domain<0>>& pSolution) {
			State s0 = mDomain.createState();
			
			Node* goalNode = mAlgo.doSearch(s0);
			
			mAlgo.prepareSolution(pSolution, goalNode);
		}
		
		void reset() {
			mAlgo.reset();
		}
		
		Json report() {
			return mAlgo.report();
		}
		
		Domain mDomain;
		Astar_t mAlgo;
	};	
}}
