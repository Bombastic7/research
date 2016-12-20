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

	 
	 
	 template<unsigned N, bool Use_H>
	 struct Pancake_DomainStack_single {
		 
		 using domStack_t = Pancake_DomainStack_single<N, Use_H>;
		 
		 template<unsigned L>
		 struct Domain : Pancake_Domain<N, Use_H> {
			 static_assert(L == 0, "");
			 
			 Domain(domStack_t& pStack) :
				Pancake_Domain<N, Use_H>(pStack.mInitState)
			{}
		 
		};
		 
		 Pancake_DomainStack_single(Json const& jConfig) :
			mInitState(jConfig.at("init").get<std::vector<cake_t>>())
		{}
		
		const PancakeStack<N> mInitState;
	 };
	 
	 
	 
	 template<unsigned N, unsigned Abt1Sz, unsigned AbtStep>
	 struct Pancake_DomainStack_IgnoreAbt : Pancake_DomainStack_single<N, false> {
		 
		 using domStack_t = Pancake_DomainStack_IgnoreAbt<N, Abt1Sz, AbtStep>;
		 
		 
		 static constexpr unsigned cakesPerLevel(unsigned L) {
			 return L == 0 ? N : Abt1Sz - AbtStep * (L == 0 ? 0 : L - 1);
		 }
		 
		 static const unsigned Top_Abstract_Level = Abt1Sz/AbtStep - 1;
		 static_assert(cakesPerLevel(Top_Abstract_Level) >= 2, "");
		 static_assert(cakesPerLevel(Top_Abstract_Level+1) < 2 || cakesPerLevel(Top_Abstract_Level+1) >= N, "");
		 
		 
		 template<unsigned L, typename = void>
		 struct Domain : public Pancake_Domain<cakesPerLevel(L), false> {
			 
			Domain(domStack_t& pStack) :
				Pancake_Domain<cakesPerLevel(L), false>()
			{}			 
		 };
		 
		 template<typename Ign>
		 struct Domain<0, Ign> : Pancake_DomainStack_single<N, false>::template Domain<0> {};
		 
		 
		 
		 
		 
		 template<unsigned L>
		 struct Abstractor : public IgnoreCakesAbt<N, cakesPerLevel(L)> {
			 
			 Abstractor(domStack_t& pStack) :
				IgnoreCakesAbt<N, cakesPerLevel(L)>(pStack.mAbt1Kept.begin(), pStack.mAbt1Kept.begin() + cakesPerLevel(L))
			{}
		 };

		 
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
		
		
		
		 Pancake_DomainStack_IgnoreAbt(Json const& jConfig) :
			Pancake_DomainStack_single<N, false>(jConfig),
			mAbt1Kept(prepL1Kept(jConfig))
		{}
		 
		 const std::array<cake_t, Abt1Sz> mAbt1Kept;
		 
	 };
	 

}}
