
#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <vector>
#include <limits>

#include "util/debug.hpp"
#include "util/permutation.hpp"
#include "util/hash.hpp"
#include "util/math.hpp"

#include "domain/pancake/defs.hpp"


namespace mjon661 { namespace pancake {


	//A permutation of 0..N-1 with some useful members.
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
		
		static size_t doHash(packed_t const& pkd) {
			return (size_t)pkd;
		}
	};
	
	
	
	//A 0..N-1 permutation with N-Sz cakes having their value (but not position) ignored.
	template<int N, int Sz>
	struct PartialPancakeStack {
		
		static const cake_t Null_Cake = N;
		
		using Perm_t = Permutation<N, Sz>;
		
		
		static const size_t Hash_Range = Sz > 12 ? std::numeric_limits<size_t>::max() : 
											(Perm_t::maxRankTrunc()+1) * (Perm_t::maxRankTrunc()+1);
		
		
		struct packed_t {
			typename Perm_t::Rank_t cakes, positions;
			
			bool operator==(packed_t const& o) const {
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
		
		template<int BaseSz>
		void fromPartialStack(PartialPancakeStack<N, BaseSz> const& pBaseState, std::array<bool, N> const& pKeptTable) {
			
			static_assert(BaseSz >= Sz, "");
			
			for(int i=0; i<N; i++)
				if(pBaseState[i] != Null_Cake && pKeptTable[pBaseState[i]])
					mAllCakes[i] = pBaseState[i];
				
				else
					mAllCakes[i] = Null_Cake;
		}
		
		
		packed_t getPacked() const {
			
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
			
			const cake_t NC = Null_Cake;
			mAllCakes.fill(NC);
			
			for(int i=0; i<Sz; i++) {
				
				cake_t c = cakePerm[i];
				unsigned p = posPerm[i];
				
				mAllCakes[p] = c;
			}
		}
		
		bool isSorted() const {
			
			Permutation<N, Sz> cakePerm;
			
			for(int i=0, j=0; i<N; i++) {
				
				cake_t c = mAllCakes[i];
				
				if(c == Null_Cake)
					continue;
				
				cakePerm[j++] = c;
				
				if(j == Sz)
					break;
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
			
			out << "[";
			
			for(int i=0; i<N; i++) {
				
				if(mAllCakes[i] == Null_Cake)
					out << ".";
				else
					out << mAllCakes[i];;

				if(i != N-1)
					out << " ";
			}
			
			out << "]";
		}
		
		bool operator==(PartialPancakeStack<N,Sz> const& o) const {
			return mAllCakes == o.mAllCakes;
		}
		
		cake_t operator[](unsigned i) const {
			return mAllCakes[i];
		}
		
		static size_t doHash(packed_t const& pkd) {
			size_t v = pkd.cakes * (Perm_t::maxRank()+1) + pkd.positions;
			return v;
		}
		
		std::array<cake_t, N> mAllCakes;
		
	};

}}
