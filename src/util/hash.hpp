#pragma once

#include <cstddef>
#include <vector>


namespace mjon661 { namespace hashfunctions {

	size_t djb2(unsigned n, const unsigned char * pStr);
	
	size_t vec_32(std::vector<uint32_t> const&);
}}
