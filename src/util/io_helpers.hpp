#pragma once

#include <array>
#include <stdexcept>
#include <vector>


namespace mjon661 {

	template<unsigned N, typename Elm_t, typename In_t>
	std::array<Elm_t, N> createArrayFromVector(std::vector<In_t> const& pVec) {
		
		std::array<Elm_t, N> retArray;
		
		if(pVec.size() != N)
			throw std::runtime_error("CreateArrayFromVector: wrong size");
		
		for(unsigned i=0; i<pVec.size(); i++)
			retArray[i] = pVec[i];
		
		return retArray;
	}
	
	
	template<unsigned N, typename Elm_t, typename In_t>
	std::array<Elm_t, N> createArrayFromVector(std::vector<In_t> const& pVec, Elm_t const& a, Elm_t const& b) {
		
		std::array<Elm_t, N> retArray;
		
		if(pVec.size() != N)
			throw std::runtime_error("CreateArrayFromVector: wrong size");
		
		for(unsigned i=0; i<pVec.size(); i++) {
			if(pVec[i] < a || pVec[i] > b)
				throw std::runtime_error("CreateArrayFromVector: out of bounds");
			
			retArray[i] = pVec[i];
		}
		
		return retArray;
	}
}
