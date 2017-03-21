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
#include <fstream>

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
	
	
	template<typename = void>
	void writeImgPPM(	std::vector<uint8_t> const& pR,
						std::vector<uint8_t> const& pG,
						std::vector<uint8_t> const& pB,
						unsigned height, 
						unsigned width, 
						std::string const& pOutFile) 
	{
		std::ofstream out(pOutFile, std::ofstream::out | std::ofstream::binary);
		
		if(!out)
			throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__));
		
		fast_assert(height*width == pR.size() && pR.size() == pB.size() && pR.size() == pB.size());
		
		std::string magicStr("P6"), widthStr = std::to_string(width), heightStr = std::to_string(height), maxvalStr("255");
		
		
		out.write(magicStr.c_str(), magicStr.size());
		out.put(' ');
		out.write(widthStr.c_str(), widthStr.size());
		out.put(' ');
		out.write(heightStr.c_str(), heightStr.size());
		out.put(' ');
		out.write(maxvalStr.c_str(), maxvalStr.size());
		out.put('\n');

		for(unsigned i=0; i<height*width; i++) {
			out.put(pR[i]);
			out.put(pG[i]);
			out.put(pB[i]);
		}
	}
	
	
	template<typename T>
	std::tuple<T,T,T> HSLtoRGBcolor(double hue, double sat, double light) {
		slow_assert(hue >= 0 && hue <= 360);
		slow_assert(sat >= 0 && sat <= 1);
		slow_assert(light >= 0 && light <= 1);
		
		double chroma = (1.0 - std::abs(2*light - 1)) * sat;
		double Hp = hue / 60.0;
		
		double x = chroma * (1.0 - std::abs(std::remainder(Hp, 2) - 1.0));
		
		slow_assert(std::isfinite(chroma) && std::isfinite(Hp) && std::isfinite(x));
		
		double vR, vG, vB;
		
		slow_assert(Hp >= 0 && Hp < 6.0);
		
		if(Hp <= 1.0) {
			vR = chroma; vG = x; vB = 0;
		} else if(Hp <= 2.0) {
			vR = x; vG = chroma; vB = 0;
		} else if(Hp <= 3.0) {
			vR = 0; vG = chroma; vB = x;
		} else if(Hp <= 4.0) {
			vR = 0; vG = x; vB = chroma;
		} else if(Hp <= 5.0) {
			vR = x; vG = 0; vB = chroma;
		} else {
			vR = chroma; vG = 0; vB = x;
		} 
		
		double m = light - 0.5 * chroma;
		return std::tuple<T,T,T>{vR + m, vG + m, vB + m};
	}
	
	
	
	
	template<typename T>
	constexpr unsigned binomialCoeff(T n, T k) {
		return k > n ? 0 : factorial<unsigned>(n) / (factorial<unsigned>(k) * factorial<unsigned>(n-k));
	}
	
	template<typename T>
	std::vector<unsigned> unrankCombination(T n, T k) {
		std::vector<unsigned> v;
		
		for(; k>0; k--) { 
			for(unsigned i=k-1; ; i++) {
				if(binomialCoeff<unsigned>(i,k) > n) {
					v.push_back(i-1);
					n -= binomialCoeff<unsigned>(i-1,k);
					break;
				}
			}
		}
		
		return v;
	}
	
	
	//Computes and stores a table of moves for positions in an N-dimensional hypergrid.
	//A position is an N-length sequence of coordinate values, for dimensions 0..N-1, respectively.
	//A move is a k-length sequence of {dimension,delta} pairs, where 1 <= k <= MaxK <= N. Delta is either +1 or -1.

	//For 1 <= k <= MaxK, number of moves affecting k dimensions is binomialCoeff(N,k) * 2**k.
	
	//Format of move is std::array<std::pair<unsigned, bool>, MaxK>.
	
	template<unsigned N, unsigned MaxK>
	struct HypergridMoveSet {
		
		static constexpr unsigned compMvsK(unsigned k) {
			return binomialCoeff(N, k) * pow<unsigned>(2, k);
		}
		
		static constexpr unsigned compTotMvs(unsigned k=1) {
			return k == MaxK+1 ? 0 : compMvsK(k) + compTotMvs(k+1);
		}
		

		
		HypergridMoveSet()
		{
			init();
		}
		
		//Total moves
		constexpr unsigned size() const {
			return compTotMvs();
		}
		
		//Get move by ordinal and set out_k to number of affect dimensions for this move.
		std::array<std::pair<unsigned, bool>, MaxK> const& getMove(unsigned i, unsigned& out_k) const {
			slow_assert(i < mMvs.size());
			out_k = mKLookup[i];
			return mMvs[i];
		}
		
		//Get i-th move that affects k dimensions.
		//~ std::array<std::pair<unsigned, bool>, MaxK> const& getKMove(unsigned k, unsigned i) const {
			//~ slow_assert(k >= 1 && k <= MaxK);
			//~ auto const& p = mFindByK[k];
			//~ slow_assert(p.first + i < p.second);
			//~ slow_assert(mKLookup[p.first + i] == k);
			//~ return mMvs[p.first + i];
		//~ }
		
		//Return the first and last+1 ordinal of moves affecting k dimensions.
		std::pair<unsigned, unsigned> const& getKsize(unsigned k) const {
			slow_assert(k >= 1 && k <= MaxK);
			return mFindByK[k];
		}
		
		//Returns sqrt(k), where k is the number of affected dimensions for move i.
		double getMoveCost(unsigned i) const {
			slow_assert(i < mMvs.size());
			return mCostLookup[i];
		}
		
		void dump(std::ostream& out) const {
			for(unsigned i=0; i<size(); i++) {
				unsigned k;
				auto const& mv = getMove(i, k);
				
				out << i << ": ";
				
				for(unsigned j=0; j<k; j++) {
					out << mv[j].first;
					if(mv[j].second)
						out << "+";
					else
						out << "-";
					out << " ";
				}
				
				out << "\n";
			}
		}
		
		
		private:
		
		void init() {
			//Null tables.
			for(unsigned i=0; i<mMvs.size(); i++) {
				mKLookup[i] = -1;
				for(unsigned j=0; j<MaxK; j++)
					mMvs[i][j].first = -1;
			}
			
			for(unsigned i=0; i<mFindByK.size(); i++) {
				mFindByK[i].first = -1;
				mFindByK[i].second = -1;
			}
			
			unsigned nMvs = 0;
			
			//For k=[1..MaxK] ...
			for(unsigned k=1; k<=MaxK; k++) {
				if(k != 1)
					mFindByK[k-1].second = nMvs;
				mFindByK[k].first = nMvs;
				
				//For all k-combinations of N dimensions...
				for(unsigned dimsRank=0; dimsRank<binomialCoeff(N,k); dimsRank++) {
					
					//For all k-tuples of {true, false}...
					for(unsigned dimsDir=0; dimsDir<(1u<<k); dimsDir++) {
						
						std::vector<unsigned> tgtDims = unrankCombination(dimsRank, k);
						slow_assert(tgtDims.size() == k && uniqueElements(tgtDims));
						
						std::array<std::pair<unsigned,bool>, MaxK> mv;
						
						for(unsigned i=0; i<k; i++) {
							slow_assert(tgtDims[i] < k);
							mv[i].first = tgtDims[i];
							mv[i].second = (dimsDir >> i) & 1;
						}
						
						mMvs[nMvs] = mv;
						mKLookup[nMvs] = k;
						mCostLookup[nMvs] = std::sqrt(k);
						nMvs++;
					}
				}
			}
		}
		

		std::array<std::array<std::pair<unsigned, bool>, MaxK>, compTotMvs()> mMvs;
		std::array<unsigned, compTotMvs()> mKLookup;
		std::array<double, compTotMvs()> mCostLookup;
		std::array<std::pair<unsigned, unsigned>, MaxK+1> mFindByK;
		
	};
}}
