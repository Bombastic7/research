#pragma once

#include <array>
#include <stdexcept>

#include "util/debug.hpp"

#include "domain/pancake/pancake_stack.hpp"



namespace mjon661 { namespace pancake {


	template<unsigned L, unsigned N, unsigned Abt1Sz>
	struct DropAndTaper {
		
		static const unsigned Self_Sz = st_abtLvlToDomSz(L, N, Abt1Sz);
		
		static const unsigned Next_Sz = st_abtLvlToDomSz(L+1, N, Abt1Sz);
		
		
		
		DropAndTaper(std::array<cake_t, Abt1Sz> const& pL1Kept) :
			mKeptLookup{},
			mMapping{}
		{
			fast_assert(withinInclusive(pL1Kept, 0u, N-1));

			for(unsigned i=0; i<Next_Sz; i++) {
				mKeptLookup[pL1Kept.at(i)] = true;
				mMapping.at(pL1Kept[i]) = i;
			}
		}
		

		PancakeStack<Next_Sz> operator()(PancakeStack<Self_Sz> const& pState) {
			
			PancakeStack<Next_Sz> nxtStack;
			
			for(unsigned i=0, j=0; j<Next_Sz; i++) {
				if(!mKeptLookup[pState[i]])
					continue;
				
				nxtStack[j++] = mMapping[pState[i]];
			}
			return nxtStack;
		}
		
		std::array<bool, N> mKeptLookup;	//Is a cake kept from L to L+1?
		std::array<cake_t, N> mMapping;		//New cake values assigned to old ones, so we always have a permutation.
	};
	

}}
