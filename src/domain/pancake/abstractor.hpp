#pragma once

#include <array>
#include <stdexcept>

#include "util/debug.hpp"

#include "domain/pancake/pancake_stack.hpp"



namespace mjon661 { namespace pancake {


	//Takes a pancake stack and produces a new one with AbtSz cakes not being ignored.
	//The kept cakes in argument stacks are expected to be a superset of those supplied in the ctor.
	template<int N, int AbtSz>
	struct IgnoreCakesAbt {

		template<typename InputIt>
		IgnoreCakesAbt(InputIt first, InputIt last) :
			mKeptCakesTable{}
		{
			
			for(unsigned i=0; i<AbtSz; ++first, i++) {
				
				fast_assert(first != last);
				
				cake_t c = *first;
				
				fast_assert(c >= 0 && (unsigned)c < N);
				
				mKeptCakesTable[c] = true;
			}
			
			fast_assert(first == last);
		}
		
		template<int BaseSz>
		PartialPancakeStack<N, AbtSz> operator()(PartialPancakeStack<N, BaseSz> const& pBaseState) {
			
			PartialPancakeStack<N, AbtSz> s;
			
			s.fromPartialStack(pBaseState, mKeptCakesTable);
			
			return s;
		}
		
		PartialPancakeStack<N, AbtSz> operator()(PancakeStack<N> const& pBaseState) {
			
			PartialPancakeStack<N, AbtSz> s;
			
			s.fromFullStack(pBaseState, mKeptCakesTable);
			
			return s;
		}
		
		
		private:
		std::array<bool, N> mKeptCakesTable;
		
	};

/*
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
*/

}}
