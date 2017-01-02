#pragma once

#include "search/basic_astar.hpp"
#include "util/json.hpp"

namespace mjon661 { namespace algorithm {

	template<	typename DomStack,
				bool Do_Weighted,
				bool Do_Greedy,
				bool Do_Speedy>
	class AstarAlgorithm {
		public:
		
		using Astar_t =			AstarImpl<DomStack, Do_Weighted, Do_Greedy, Do_Speedy, true>;
		using Domain =			typename DomStack::template Domain<0>;
		using State = 			typename Domain::State;
		using Node =			typename Astar_t::Node;
			
		Astar(DomStack& pDomStack, Json const& jConfig) :
			mDomain(pDomStack),
			mAlgo(pDomStack, jConfig)
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
	
	template<typename DomStack>
	using Astar = AstarAlgorithm<DomStack, false, false, false>;

	template<typename DomStack>
	using WeightedAstar = AstarAlgorithm<DomStack, true, false, false>;

	template<typename DomStack>
	using GreedyAstar = AstarAlgorithm<DomStack, false, true, false>;

	template<typename DomStack>
	using SpeedyAstar = AstarAlgorithm<DomStack, false, false, true>;
}}
