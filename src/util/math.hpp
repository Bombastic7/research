#pragma once

#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <iterator>
#include <cmath>
//#include <boost/multiprecision/cpp_int.hpp>
#include "util/debug.hpp"
#include "util/impl/LambertW/LambertW.h"

/*
 * Math operations, added as needed.
 */

namespace mjon661 { namespace mathutil {

	constexpr uint64_t factorial64(uint64_t n) {
		return n == 0 ? 1 : n * factorial64(n-1);
	}
	
	//Returns n! / s!
	constexpr uint64_t factorial64(uint64_t n, uint64_t s) {
		return s == 0 ? 1 : n * factorial64(n-1, s-1);
	}
	
	template<typename T>
	constexpr T factorial(unsigned n) {
		return n == 0 ? 1 : n * factorial<T>(n-1);
	}
	
	template<typename T>
	constexpr T factorial(unsigned n, unsigned s) {
		return s == 0 ? 1 : n * factorial<T>(n-1, s-1);
	}
	
	template<typename T>
	constexpr T min(T a, T b) {
		return a < b ? a : b;
	}
	
	template<typename T>
	constexpr T max(T a, T b) {
		return a > b ? a : b;
	}
	
	template<typename T>
	constexpr T pow_impl(T a, T b, unsigned n) {
		return n == b+1 ? 1 : a * pow_impl(a, b, n+1);
	}
	
	template<typename T>
	constexpr T pow(T a, T b) {
		return pow_impl(a, b, 1);
	}
	
	template<typename T>
	constexpr T abs(T a, T b) {
		return a > b ? a - b : b - a;
	}
	
	template<typename T>
	bool subset(T const& sb_, T const& s_, bool proper) {
		
		T sb_s(sb_), s_s(s_);
		
		std::sort(sb_s.begin(), sb_s.end());
		std::sort(s_s.begin(), s_s.end());
		
		T sb, s;
		
		std::unique_copy(sb_s.begin(), sb_s.end(), std::back_inserter(sb));
		std::unique_copy(s_s.begin(), s_s.end(), std::back_inserter(s));
		
		T i;
		
		std::set_intersection(sb.begin(), sb.end(), s.begin(), s.end(), std::back_inserter(i));
		
		return i == sb && (!proper || i != s);
	}

	
	template<typename T, typename V>
	bool contains(T const& t, V const& v, unsigned sz) {
		for(unsigned i=0; i<sz; i++)
			if(t[i] == v)
				return true;
		return false;
	}
	
	template<typename T, typename V>
	bool contains(T const& t, V const& v) {
		return contains(t, v, t.size());
	}
	
	template<typename T>
	bool uniqueElements(T const& v) {
		T sorted(v);
		std::sort(sorted.begin(), sorted.end());
		
		for(unsigned i=0; i<sorted.size()-1; i++) {
			if(sorted[i] == sorted[i+1])
				return false;
		}
		
		return true;
	}
	
	template<typename T, typename E>
	bool withinInclusive(T const& v, E const& a, E const& b) {
		T sorted(v);
		std::sort(sorted.begin(), sorted.end());
		
		if(static_cast<E>(sorted.front()) < a || static_cast<E>(sorted.back()) > b)
			return false;
		
		return true;
	}
	
	template<int Branch = 0>
	double lambertW(double x) {
		return utl::LambertW<Branch>(x);
	}
	
	
	template<typename T>
	double sumOfPowers(T b, unsigned n, unsigned start = 0) {
		T s = 0;
		
		for(unsigned i=start; i<=start+n; i++)
			s += std::pow(b, i);
		
		return s;
	}
}}
