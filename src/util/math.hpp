#pragma once

#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <vector>
#include <array>
#include <limits>
#include <type_traits>
#include <tuple>
#include <random>

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
	
	template<typename V>
	std::array<double, 3> sampleStats(V const& pVals) {
		if(pVals.size() == 0)
			return {0, 0, std::numeric_limits<double>::infinity()};
		if(pVals.size() == 1)
			return {pVals[0], pVals[0], std::numeric_limits<double>::infinity()};

		double mean = 0;
		for(auto v : pVals)
			mean += v;
		mean /= pVals.size();
		
		V vsrtd(pVals);
		
		std::sort(vsrtd.begin(), vsrtd.end());
		double median = vsrtd[(vsrtd.size()-1)/2];
		
		double stddev = 0;
		for(auto v : pVals)
			stddev += std::pow(v - mean, 2);
		stddev /= pVals.size() - 1;
		stddev = std::sqrt(stddev);
		
		return {mean, median, stddev};
	}
	
	template<typename U>
	struct BitIt {
		static_assert(std::is_unsigned<U>::value, "");
		
		BitIt(U u) :
			val(u),
			b(val & 1)
		{}
		
		bool operator()() {
			return b;
		}
		
		BitIt& operator++() {
			val >>= 1;
			b = val & 1;
			return *this;
		}
		
		bool bitsRemaining() {
			return val != 0;
		}
		
		private:
		U val;
		bool b;
	};
	
	
	
	//Quick and dirty implementation of Prim's algorithm for generating a MST.
	//There are pNnodes, labeled [0..pNnodes-1].
	//Edges have form {source, destination, cost}.
	//Edges forming the spanning tree are returned.
	template<typename Cost>
	std::vector<std::tuple<unsigned, unsigned, Cost>> minSpanningTreePrims(	unsigned pNnodes, 
																			std::vector<std::tuple<unsigned, unsigned, Cost>>& pEdges)
	{	
		using Edge_t = std::tuple<unsigned, unsigned, Cost>;
		
		std::mt19937 randgen;
		
		std::vector<Cost> bestCost(pNnodes, std::numeric_limits<Cost>::max());
		std::vector<unsigned> bestEdge(pNnodes, -1);
		
		std::vector<bool> nodeInForest(pNnodes, false);

		std::vector<Edge_t> edgesIn;
		

		
		while(true) {
			
			std::vector<unsigned> nodesOut;
			
			for(unsigned i=0; i<pNnodes; i++)
				if(!nodeInForest[i])
					nodesOut.push_back(i);
			
			if(nodesOut.size() == 0)
				break;
			
			std::shuffle(nodesOut.begin(), nodesOut.end(), randgen);
			
			unsigned bestOutNode = nodesOut[0];
			Cost costOutNode = bestCost[bestOutNode];

			for(unsigned outn : nodesOut) {
				if(costOutNode > bestCost[outn]) {
					bestOutNode = outn;
					costOutNode = bestCost[outn];
				}
			}
			
			
			nodeInForest[bestOutNode] = true;
			
			if(bestEdge[bestOutNode] != (unsigned)-1)
				edgesIn.push_back(pEdges[bestEdge[bestOutNode]]);
			
			for(unsigned i=0; i<pEdges.size(); i++) {
				unsigned src = std::get<0>(pEdges[i]);
				unsigned dst = std::get<1>(pEdges[i]);
				unsigned cst = std::get<2>(pEdges[i]);
				
				if(src != bestOutNode)
					continue;
				
				if(!nodeInForest[dst] && cst < bestCost[dst]) {
					bestCost[dst] = cst;
					bestEdge[dst] = i;
				}
			}
		}
		
		
		std::vector<bool> testseen(pNnodes, false);
		
		for(auto& e : edgesIn) {
			testseen.at(std::get<0>(e)) = true;
			testseen.at(std::get<1>(e)) = true;
		}
		
		for(bool b : testseen)
			fast_assert(b);
		
		return edgesIn;
	}
}}
