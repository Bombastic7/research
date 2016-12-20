
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
	
	
	template<int N, int Sz>
	struct PartialPancakeStack {
		
		static const cake_t Null_Cake = N;
		static const unsigned M = N-Sz;
		
		Perm_t = Permutation<N, Sz>;
		
		
		struct packed_t {
			typename Perm_t::Rank_t cakes, positions;
			
			bool operator==(packed_t const& o) {
				return cakes == o.cakes && positions == o.positions;
			}
		};
		
		PartialPancakeStack() = default;
		
		void fromFullStack(PancakeStack<N> const& pBaseState, std::array<bool, N> const& pKeptTable) {
			slow_assert(pBaseState.valid());
			
			
			for(int i=0; i<N; i++)
				if(pKeptTable[pBaseState[i]])
					mAllCakes[i] = pBaseState[i];
				
				else
					mAllCakes[i] = Null_Cake;
		}
		
		
		packed_t getPacked() {
			
			packed_t pkd;
			
			Perm_t cakePerm, posPerm;
			
			
			for(int i=0, j=0; i<N; i++) {
				
				cake_t c = mAllCakes[i];
				if(c == Null_Cake)
					continue;
				
				cakePerm[j] = c;
				posPerm[j] = i;
				j++;
			}
			
			slow_assert(cakePerm.valid() && posPerm.valid());
			
			pkd.cakes = cakePerm.getRank();
			pkd.positions = posPerm.getRank();
			
			return pkd;
		}
		
		void fromPacked(packed_t const& pkd) {

			Perm_t cakePerm, posPerm;
			cakePerm.setRank(pkd.cakes);
			posPerm.setRank(pkd.positions);

			slow_assert(cakePerm.valid() && posPerm.valid());
			
			mAllCakes.fill(Null_Cake);
			
			for(int i=0; i<Sz; i++) {
				
				cake_t c = cakePerm[i];
				unsigned p = posPerm[i];
				
				mAllCakes[p] = c;
			}
		}
		
		bool isSorted() {
			
			Permutation<N, Sz> cakePerm;
			
			for(int i=0, j=0; i<N; i++) {
				
				cake_t c = mAllCakes[i];
				
				if(c == Null_Cake)
					continue;
				
				cakePerm[j] = c;
			}
			
			slow_assert(cakePerm.valid());
			
			for(unsigned i=0; i<Sz-1; i++)
				if(cakePerm[i] > cakePerm[i+1])
					return false;
			
			return true;
		}
		
		void flip(unsigned op) {
			slow_assert(op < N);
			
			for (unsigned i = 0; i <= op/2; i++) {
				int tmp = mAllCakes[i];
				mAllCakes[i] = mAllCakes[op - i];
				mAllCakes[op - i] = tmp;
			}
		}
		
		void prettyPrint(std::ostream& out) const {
			out << "[ ";
			
			for(int i=0; i<N; i++)
				
				if(mAllCakes[i] == Null_Cake)
					out << ". ";
				else
					out << mAllCakes[i] << " ";

			out << "]";
		}
		
		std::array<cake_t, N> mAllCakes;
		
	};


}}
