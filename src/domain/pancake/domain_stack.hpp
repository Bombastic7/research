#pragma once

#include <array>
#include <vector>

#include "util/json.hpp"
#include "util/math.hpp"
#include "util/exception.hpp"

#include "domain/pancake/domain.hpp"
#include "domain/pancake/pancake_stack.hpp"


namespace mjon661 { namespace pancake {
	 
	 
	 template<unsigned N, unsigned Abt1Sz, unsigned AbtStep, bool Use_H, bool Use_Weight>
	 struct Pancake_DomainStack_IgnoreAbt {
		 
		using domStack_t = Pancake_DomainStack_IgnoreAbt<N, Abt1Sz, AbtStep, Use_H, Use_Weight>;
		
		
		static constexpr unsigned cakesPerLevel(int L) {
			return L == 0 ? N : (int)Abt1Sz - (int)AbtStep * (L == 0 ? 0 : L - 1);
		}
		
		static constexpr int topAbtLvl(int L=0) {
			return cakesPerLevel(L) <= AbtStep+1 ? L : topAbtLvl(L+1); 
		}
		 
		static const unsigned Top_Abstract_Level = topAbtLvl();
		
		static_assert(cakesPerLevel(Top_Abstract_Level) >= 2, "");
		 
		 
		template<unsigned L, typename = void>
		struct Domain : public Domain_NoH_Relaxed<N, cakesPerLevel(L), Use_Weight> {
			Domain(domStack_t const& pStack) :
				Domain_NoH_Relaxed<N, cakesPerLevel(L), Use_Weight>(
					pStack.mAbt1Kept.begin(), 
					pStack.mAbt1Kept.begin() + cakesPerLevel(L))
				{}
		};
		 
		 

		template<typename Ign>
		struct Domain<0, Ign> : Pancake_Domain<N, Use_H, Use_Weight> {

			Domain(domStack_t const& pStack) :
				Pancake_Domain<N, Use_H, Use_Weight>(pStack.mInitState)
			{}
		};


		std::array<cake_t, Abt1Sz> prepL1Kept(Json const& jConfig) {
			std::array<cake_t, Abt1Sz> retArray;

			if(jConfig.count("kept")) {
				std::vector<cake_t> v = jConfig.at("kept");

				if(v.size() != (unsigned)Abt1Sz || !mathutil::withinInclusive(v, 0u, N-1) || !mathutil::uniqueElements(v))
					throw ConfigException("Bad kept cakes");

				for(unsigned i=0; i<Abt1Sz; i++)
					retArray[i] = v[i];
			} else
				for(unsigned i=0; i<Abt1Sz; i++)
					retArray[i] = i;

			return retArray;
		}

		PancakeStack<N> getInitState() {
			return mInitState;
		}


		Pancake_DomainStack_IgnoreAbt(Json const& jConfig) :
			mAbt1Kept(prepL1Kept(jConfig)),
			mInitState(jConfig.at("init").get<std::vector<cake_t>>())
		{}


		const std::array<cake_t, Abt1Sz> mAbt1Kept;
		const PancakeStack<N> mInitState;

	};


}}
