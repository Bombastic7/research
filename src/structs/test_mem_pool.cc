
#include <cstring>
#include <utility>
#include <vector>
#include <iostream>
#include <set>
#include <algorithm>
#include "structs/mem_pool.hpp"
#include "structs/object_pool.hpp"
#include "util/debug.hpp"


namespace mjon661 { namespace test {
	
	namespace mempoolimpl {
		
		const unsigned Element_Data_Size = 15;
		
		struct Element {
			unsigned data[Element_Data_Size];
		};
		
		bool corrupt(std::vector<std::pair<Element*, Element*>>& v) {
			
			for(unsigned i=0; i<v.size(); i++) {
				if(!v[i].first) {
					if(v[i].second) {
						std::cout << "Unpaired null\n";
						return true;
					}
					continue;
				}
				
				if(memcmp(v[i].first, v[i].second, sizeof(Element)) != 0) {
					std::cout << "Corrupt idx=" << i << "\n";
					for(unsigned j=0; j<Element_Data_Size; j++) {
						std::cout << v[i].first->data[j] << " " << v[i].second->data[j] << "\n";
					}
					return true;
				}
			}
			
			return false;
		}
		
		
	}
	
	void test_mem_pool(unsigned testN) {
		
		using mempoolimpl::corrupt;
		using mempoolimpl::Element;
		using mempoolimpl::Element_Data_Size;
		
		Element* elmRefPool = new Element[testN];
		
		std::vector<std::pair<Element*, Element*>> testRefElementPairs;
		
		MemPool<sizeof(Element), alignof(Element)> testPool;
		
		
		for(unsigned i=0, j=0; i<testN; i++) {
			Element* e_test = reinterpret_cast<Element*>(testPool.malloc());
			Element* e_ref = elmRefPool + i;
			
			for(unsigned k=0; k<Element_Data_Size; k++, j++)
				e_test->data[k] = e_ref->data[k] = j;
			
			testRefElementPairs.push_back(std::pair<Element*, Element*>(e_test, e_ref));
		}
		
		gen_assert(!corrupt(testRefElementPairs));
		
		
		std::set<unsigned> freedChunks;
		
		for(unsigned i=0; i<testN/2; i++) {
			
			unsigned idx = 0;
			do {
				idx = rand() % testRefElementPairs.size();
			} while(freedChunks.count(idx) == 1);
			
			freedChunks.insert(idx);
			
			std::pair<Element*, Element*> e = testRefElementPairs[idx];
			
			testRefElementPairs[idx].first = nullptr;
			testRefElementPairs[idx].second = nullptr;
			
			testPool.free(e.first);
		}
		
		
		gen_assert(!corrupt(testRefElementPairs));
		
		
		for(unsigned i=0; i<testN/4; i++) {
			testPool.malloc();
		}
		
		gen_assert(!corrupt(testRefElementPairs));
		
		
		testPool.shrink();
		
		gen_assert(!corrupt(testRefElementPairs));
		
		testPool.clear();
		
		std::vector<void*> allocd;
		
		for(unsigned i=0; i<100; i++) {
			
			for(unsigned j=0; j<10000; j++)
				allocd.push_back(testPool.malloc());
			
			std::random_shuffle(allocd.begin(), allocd.end());
			
			for(unsigned j=0; j<10000; j++)
				testPool.free(allocd[j]);
				
			allocd.clear();
		}
		
		for(unsigned i=0; i<100; i++) {
			
			for(unsigned j=0; j<10000; j++)
				testPool.malloc();
			
			testPool.purge();
		}
		
		
	}
	

}}
