#pragma once

#include <vector>

#include "util/debug.hpp"



namespace mjon661 { namespace tiles {
	std::vector<int> tiles15_instances_korf(unsigned i) __attribute__ ((unused));
	std::vector<int> tiles8_instances(unsigned i) __attribute__ ((unused));
	std::vector<int> tiles11_instances(unsigned i) __attribute__ ((unused));
	std::vector<int> tiles_defgoal(unsigned i) __attribute__ ((unused));
	std::vector<int> tiles_abtnullkept(unsigned i)  __attribute__ ((unused));
	
	
	
	std::vector<int> tiles15_instances_korf(unsigned i) {
		switch(i) {
			case 0:
				return {14,13,15,7,11,12,9,5,6,0,2,1,4,8,10,3}; //57
			case 1:
				return {13,5,4,10,9,12,8,14,2,3,7,1,0,15,11,6}; //55
			case 2:
				return {14,7,8,2,13,11,10,4,9,12,5,0,3,6,1,15}; //59
			case 3:
				return {5,12,10,7,15,11,14,0,8,2,1,13,3,4,9,6}; //56
			case 4:
				return {4,7,14,13,10,3,9,12,11,5,6,15,1,2,8,0}; //56
			case 5:
				return {14,7,1,9,12,3,6,15,8,11,2,5,10,0,4,13}; //52
			case 6:
				return {2,11,15,5,13,4,6,7,12,8,10,1,9,3,14,0}; //52
			case 7:
				return {12,11,15,3,8,0,4,2,6,13,9,5,14,1,10,7}; //50
			case 8:
				return {3,14,9,11,5,4,8,2,13,12,6,7,10,1,15,0}; //46
			case 9:
				return {13,11,8,9,0,15,7,10,4,3,6,14,5,12,2,1}; //59
			default:
				gen_assert(false);
		}
		
		return {};
	}


	std::vector<int> tiles8_instances(unsigned i) {
		switch(i) {
			case 0:
				return {7,6,4,1,5,3,8,0,2,};
			case 1:
				return {7,6,5,1,2,8,0,4,3,};
			case 2:
				return {3,0,1,8,6,7,4,2,5,};
			case 3:
				return {5,4,3,7,1,8,0,2,6,};
			case 4:
				return {2,0,5,1,8,7,6,4,3,};
			case 5:
				return {5,0,7,6,8,4,1,2,3,};
			case 6:
				return {7,1,8,5,6,0,3,2,4,};
			case 7:
				return {5,0,2,8,3,7,4,6,1,};
			case 8:
				return {1,6,8,3,0,7,5,4,2,};
			case 9:
				return {0,8,3,6,5,4,7,1,2,};
			default:
				gen_assert(false);
		}
		
		return {};
	}

	std::vector<int> tiles11_instances(unsigned i) {
		return std::vector<std::vector<int>> {
		{9,3,0,11,1,5,2,8,6,4,7,10,},
		{4,9,11,10,1,7,6,0,2,3,8,5,},
		{3,1,8,7,2,11,0,6,5,9,10,4,},
		{11,6,5,8,2,9,1,10,0,7,3,4,},
		{11,7,4,3,1,9,10,2,5,8,6,0,},
		{8,2,0,11,7,1,10,6,4,9,3,5,},
		{0,10,7,3,4,11,5,1,6,2,8,9,},
		{3,11,5,2,1,6,7,10,0,9,4,8,},
		{1,8,11,3,2,6,9,5,10,7,0,4,},
		{0,3,7,8,2,9,6,4,11,1,5,10,}}.at(i);
	}
	
	std::vector<int> tiles_defgoal(unsigned i) {
		std::vector<int> v(i);
		for(unsigned n=0; n<i; n++) {
			v[n] = n;
		}
		return v;
	}
	
	std::vector<int> tiles_abtnullkept(unsigned i) {
		return std::vector<int>(i-1, 1);
	}
}}
