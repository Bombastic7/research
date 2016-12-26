#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <limits>
#include <iostream>
#include <string>
#include "util/debug.hpp"
#include "util/math.hpp"
#include "util/static_data.hpp"


namespace mjon661 {


	//Vals
	//idx		0	1	2		3		Sz-2	Sz-1
	//Radix 	N	N-1	N-2		N-3	    N-Sz+2	N-Sz+1
	//PV		N-1 N-2	N-3		N-4		N-Sz+1	N-Sz
	
	//Encapsulates an array of ints representing a k-permutation of N (where k == Sz).
	//Values are [0, N-1].
	//Underlying array can be directly accessed. No checks are performed; call valid() manually.
	template<int N, int Sz>
	struct Permutation : public std::array<int, Sz> {
		static_assert(N > 0 && Sz > 0 && N >= Sz, "");
		
		using base_t = std::array<int, Sz> ;
		using Rank_t = typename static_data::Factorial<N, Sz>::type;
		
		
		static Rank_t maxRank() {
			return static_data::Factorial<N, Sz>::getValue() - 1;
		}
		
		static constexpr size_t maxRankTrunc() {
			//Put the -10 to avoid overflow; e.g. HashTableSz = maxRank+1.
			return sizeof(Rank_t) > sizeof(size_t) ? std::numeric_limits<size_t>::max() - 10 : static_data::Factorial<N, Sz>::getValue()-1;
		}
		

		Permutation() = default;
		
		template<typename InputIt>
		Permutation(InputIt first, InputIt last) {
			setValues(first, last);
		}
		
		Permutation(base_t const& o)
			: base_t(o)
		{
			testCond(valid(), "Input not a permutation");
		}
		
		template<typename InputIt>
		void setValues(InputIt first, InputIt last) {
			for(int i=0; i<Sz; i++) {
				testCond(first != last, std::string("Input too short : ") + std::to_string(i) + " : " + std::to_string(N) + " : " + std::to_string(Sz));
				
				(*this)[i] = *first;
				
				++first;
			}
			
			testCond(first == last, "Input too long");
			testCond(valid(), "Input not a permutation");
		}


		Rank_t getRank() const {
			base_t vals(*this);
			
			for(int i=0; i<Sz; i++)
				for(int j=i+1; j<Sz; j++)
					if(vals[i] < vals[j])
						vals[j]--;
			
			Rank_t pkd = 0;
			
			for(int i=0; i<Sz; i++)
				pkd += vals[i] * mathutil::factorial<Rank_t>(N-i-1, Sz-i-1);
			
			return pkd;
		}

		void setRank(Rank_t r) {
			for(int i=Sz-1; i>=0; i--) {
				(*this)[i] = (int)(r % (N-i));
				r /= N-i;
			}
			
			for(int i=Sz-2; i>=0; i--)
				for(int j=i+1; j<Sz; j++)
					if((*this)[j] >= (*this)[i])
						(*this)[j]++;
		}
		
		bool valid() const {
			return valid(this->begin(), this->end());
		}
		
		
		template<typename InputIt>
		static bool valid(InputIt first, InputIt last) {
			
			std::vector<int> v(first, last);
			
			if(v.size() != Sz)
				return false;
			
			if(!mathutil::withinInclusive(v, 0, N-1) || !mathutil::uniqueElements(v))
				return false;
			
			return true;
		}
		
		void prefixReversal(unsigned op) {
			slow_assert(op < Sz);
			
			for (unsigned i = 0; i <= op/2; i++) {
				int tmp = (*this)[i];
				(*this)[i] = (*this)[op - i];
				(*this)[op - i] = tmp;
				//typename T::value_type tmp = p.at(i);
				//p.at(i) = p.at(op - i);
				//p.at(op - i) = tmp;
			}
		}	
		
		private:
		
		void testCond(bool b, std::string const& pStr) const {
			if(!b)
				throw std::invalid_argument(pStr);
		}
		
	};
	
	template<int N, int Sz>
	std::ostream& operator<<(std::ostream& out, Permutation<N, Sz> const& p) {
		
		out << "[";
		for(int i=0; i<Sz; i++) {
			out << p[i];
			if(i != Sz-1)
				out << " ";
		}
		out << "]";
		return out;
	}


	
	template<typename T>
	unsigned permInversions(const T& p) {
		unsigned n = 0;
		
		for(unsigned i=0; i<p.size(); i++) {
			for(unsigned j=i+1; j<p.size(); j++)
				if(p[i] > p[j]) n++;
		}
		return n;
	}
	
	template<typename T>
	unsigned permInversions_ignoreZero(const T& p) {
		unsigned n = 0;
		
		for(unsigned i=0; i<p.size(); i++) {
			for(unsigned j=i+1; j<p.size(); j++)
				if(p[i] > p[j] && p[j] != 0) n++;
		}
		return n;
	}
	

	
	
}
