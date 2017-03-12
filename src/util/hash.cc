#include "util/hash.hpp"

#include <cstdint>
#include <cstddef>
#include <vector>


namespace mjon661 { namespace hashfunctions {

	size_t djb2(unsigned n, const unsigned char * pStr) {
		size_t hash = 5381;
		int c;
	
		while((n-- > 0) && (c = *pStr++))
			hash = ((hash << 5) + hash) + c;
		
		return hash;
	}
	
	//from http://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector
	size_t vec_32(std::vector<uint32_t> const& v) {
		size_t seed = v.size();
		
		for(auto& i : v)
			seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		
		return seed;
	}
}}
