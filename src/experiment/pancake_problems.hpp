#pragma once

#include <algorithm>
#include <random>
#include <vector>


namespace mjon661 { namespace pancake {

	std::vector<int> ident_state(unsigned i) {
		std::vector<int> v(i);
		for(unsigned n=0; n<i; n++)
			v[n] = n;
		
		return v;
	}


	std::vector<int> pancakeN_instances(unsigned ncakes, unsigned i) {
		std::mt19937 randgen;
		std::vector s = ident_state(ncakes);
		
		for(unsigned j=0; j<=i; j++)
			std::shuffle(s.begin(), s.end(), randgen);
		
		return s;
	}

}}
