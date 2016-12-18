#include <iostream>
#include "util/debug.hpp"


namespace mjon661 { namespace test {
	void test_fast_vector(unsigned);
	void test_fast_binheap(unsigned);
	void test_fast_hashtable(unsigned);
	void test_mem_pool(unsigned);
}}

int main() {
	using namespace mjon661::test;
	const int testN = 10;
	
	try {
		test_fast_vector(testN);
		test_fast_binheap(testN);
		test_fast_hashtable(testN);
		test_mem_pool(testN);
		
	} catch(mjon661::AssertException& e) {
		std::cout << "FAILED\n" << e.what() << std::endl;
		return 1;
	}
	std::cout << "pass" << std::endl;
	return 0;
}
