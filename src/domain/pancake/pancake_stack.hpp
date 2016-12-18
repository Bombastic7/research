
#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <vector>

#include "util/debug.hpp"
#include "util/permutation.hpp"
#include "util/hash.hpp"

#include "domain/pancake/defs.hpp"


namespace mjon661 { namespace pancake {


	template<int N>
	struct PancakeStack : public Permutation<N, N> {
		static_assert(N > 0, "");
		
		using base_t = Permutation<N, N>;
		
		using packed_t = typename base_t::Rank_t;
		
		static const size_t Hash_Range = base_t::maxRankTrunc() + 1;
		
		
		PancakeStack() = default;
		
		PancakeStack(std::vector<cake_t> const& pVec) :
			base_t(pVec.begin(), pVec.end())
		{}
		
		
		void fromVec(std::vector<cake_t> const& pVec) {
			base_t::setValues(pVec.begin(), pVec.end());
		}
		
		std::vector<cake_t> getVec() const {
			std::vector<cake_t> v(N);
			
			for(unsigned i=0; i<N; i++)
				v[i] = (*this)[i];
			
			return v;
		}
		
		void flip(unsigned i) {
			this->prefixReversal(i);
		}
		
		void prettyPrint(std::ostream& out) const {
			//out << static_cast<Permutation<N,N> const&>(*this);
			out << *this;
		}
		
		packed_t getPacked() const {
			return base_t::getRank();
		}
		
		void fromPacked(packed_t const& pkd) {
			base_t::setRank(pkd);
		}
		
		bool isSorted() const {
			return std::is_sorted(this->begin(), this->end());
		}
	};


}}
