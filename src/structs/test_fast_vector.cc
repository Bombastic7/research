#include <vector>
#include <ctime>
#include <cstdlib>
#include "structs/fast_vector.hpp"
#include "util/debug.hpp"


namespace mjon661 { namespace test {
	
	
	void test_fast_vector(unsigned testN) {
		//const int testN = 10000;
		
		std::vector<int> stdVec1;
		FastVector<int> testVec1;
		
		for(unsigned i=0; i<testN; i++) {
			int r = rand();
			stdVec1.push_back(r);
			testVec1.push_back(r);
		}
		
		for(unsigned i=0; i<testN; i++)
			gen_assert(stdVec1[i] == testVec1[i]);

		
		std::vector<int> stdVec2;
		FastVector<int> testVec2;
		
		for(unsigned i=0; i<testN/2; i++) {
			int r = rand();
			stdVec2.push_back(r);
			testVec2.push_back(r);
		}
		
		testVec2.swap(testVec1);
		
		gen_assert(testVec2.size() == stdVec1.size() && testVec1.size() == stdVec2.size());
		
		for(unsigned i=0; i<testVec2.size(); i++)
			gen_assert(testVec2[i] == stdVec1[i]);

		for(unsigned i=0; i<testVec1.size(); i++)
			gen_assert(testVec1[i] == stdVec2[i]);
		
		testVec2.swap(testVec1);
		
		gen_assert(testVec1.size() == stdVec1.size() && testVec2.size() == stdVec2.size());
			
		for(unsigned i=0; i<testVec2.size(); i++)
			gen_assert(testVec2[i] == stdVec2[i]);

		for(unsigned i=0; i<testVec1.size(); i++)
			gen_assert(testVec1[i] == stdVec1[i]);

		for(unsigned i=0; i<testN; i++) {
			FastVector<int> v1(1), v2(1);
			for(unsigned j=0; j<testN; j++) {
				v1.push_back(rand());
				v2.push_back(rand());
			}
			v1.swap(v2);
		}
		
		testVec1.clear();
		testVec2.clear();
		
		gen_assert(testVec2.size() == 0 && testVec1.size() == 0);
	}
}}
