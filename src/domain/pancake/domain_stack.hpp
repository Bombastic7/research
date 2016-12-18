#pragma once

#include <array>
#include <vector>

#include "util/json.hpp"
#include "util/math.hpp"
#include "util/exception.hpp"

#include "domain/pancake/domain.hpp"
#include "domain/pancake/abstractor.hpp"
#include "domain/pancake/pancake_stack.hpp"


namespace mjon661 { namespace pancake {

	/**
	 * \brief Pancake domain stack.
	 * 
	 * Implementation for a pancake domain stack.
	 * 
	 * \verbatim
	 * 
	 * N is size of base state.
	 * Abt1Sz is size of first abstraction level state.
	 * DomImpl is the underlying domain type.
	 * 
	 * Size of abstraction levels greater than 0 is  Abt1Sz + 1 - L. So Final level is L == Abt1Sz.
	 * 
	 * 
	 * DomImpl follows the normal domain pattern, with the following:
	 * - Single template parameter (call it 'M') is an unsigned int, valid for 1 to N.
	 * 
	 * - Implements default ctor (does not need to implement createState)
	 * - Implements ctor(PancakeStack<N>& pInitState) (does implement createState)
	 * 
	 * - DomImpl::State must be a subclass of PancakeStack<N>
	 * - DomImpl::State must have (in addition to default) ctor(PancakeStack<N>&)
	 * 
	 * 
	 * 
	 * jConfig:
	 * 	init :	Required. Array[N], permutation of {0, N-1}.
	 * 	kept :	Optional. Array[Abt1Sz], proper subset of init. Default is range(0, Abt1Sz).
	 * 
	 * \endverbatim
	 */
	template<unsigned N, unsigned Abt1Sz, template<unsigned> typename DomImpl>
	struct DomainStack {
		static_assert(N > 1 && Abt1Sz > 1 && Abt1Sz < N, "");
		
		using domstack_t = DomainStack<N, Abt1Sz, DomImpl>;
		
		static const unsigned Top_Abstract_Level = Abt1Sz-1;


		template<unsigned L, typename = void>
		struct Domain : public DomImpl<st_abtLvlToDomSz(L, N, Abt1Sz)> {
			
			Domain(domstack_t& pSt) {}
		};
		
		template<typename Ign>
		struct Domain<0, Ign> : DomImpl<st_abtLvlToDomSz(0, N, Abt1Sz)> {
			
			Domain(domstack_t& pSt) :
				DomImpl<st_abtLvlToDomSz(0, N, Abt1Sz)>(pSt.mInitState)
			{}
		};
		
		
		
		template<unsigned L>
		struct Abstractor : public DropAndTaper<L, N, Abt1Sz> {
			
			Abstractor(domstack_t& pSt) :
				DropAndTaper<L, N, Abt1Sz>(pSt.mL1Kept)
			{}
		};
		
		
		
		
		DomainStack(Json const& jConfig) :
			mInitState(jConfig.at("init").get<std::vector<cake_t>>()),
			mL1Kept(prepL1Kept(jConfig))
		{}
		
		
		std::array<cake_t, Abt1Sz> prepL1Kept(Json const& jConfig) {
			std::array<cake_t, Abt1Sz> retArray;
			
			if(jConfig.count("kept")) {
				std::vector<cake_t> v = jConfig.at("kept");
				
				if(v.size() != (unsigned)Abt1Sz || !withinInclusive(v, 0u, N-1) || !uniqueElements(v))
					throw ConfigException("Bad kept cakes");
	
				for(unsigned i=0; i<Abt1Sz; i++)
					retArray[i] = v[i];
			} else
				for(unsigned i=0; i<Abt1Sz; i++)
					retArray[i] = i;
			
			return retArray;
		}
		
		const PancakeStack<N> mInitState;
		const std::array<cake_t, Abt1Sz> mL1Kept;
	};



}}
