#include "util/hash.hpp"

namespace mjon661 { namespace hashfunctions {

	size_t djb2(unsigned n, const unsigned char * pStr) {
		size_t hash = 5381;
		int c;
	
		while((n-- > 0) && (c = *pStr++))
			hash = ((hash << 5) + hash) + c;
		
		return hash;
	}
}}
