#include <cstdlib>
#include <algorithm>
#include <vector>
#include "structs/fast_binheap.hpp"
#include "util/debug.hpp"

namespace mjon661 { namespace test {
	
	
	namespace binheapimpl {
		
		struct Element {
			int val;
			unsigned openind;
			
			Element() : val(rand()) {}
			
			void randomise() {
				val = rand();
			}
			
			bool operator== (const Element& o) const {
				return val == o.val && openind == openind;
			}
		};
		
		struct PredFunctor {
			bool operator()(Element * const a, Element * const b) const {
				return a->val < b->val;
			}
		};
		
		struct NotifierFunctor {
			void operator()(Element * const e, unsigned i) const {
				e->openind = i;
			}
		};
	}
	
	
	void test_fast_binheap(unsigned testN) {
		using namespace binheapimpl;
		
		//const int testN = 10000;
		
		FastBinheap<Element, PredFunctor, NotifierFunctor> testBinheap(1, PredFunctor(), NotifierFunctor());
		
		Element * elementPool = new Element[testN];
		
		std::vector<Element*> stdSortedElements(testN);
		for(unsigned i=0; i<testN; i++) {
			stdSortedElements[i] = elementPool + i;
		}
		
		std::sort(stdSortedElements.begin(), stdSortedElements.end(), PredFunctor());
		
		for(unsigned i=0; i<testN; i++)
			testBinheap.push(elementPool + i);
		
		std::vector<Element*> testSortedElements(testN);
		
		for(unsigned i=0; i<testN; i++)
			testSortedElements[i] = testBinheap.pop();
		
		for(unsigned i=0; i<testN; i++)
			gen_assert(testSortedElements[i]->val == stdSortedElements[i]->val);

		
		
		for(unsigned i=0; i<testN; i++)
			testBinheap.push(elementPool + i);
		
		for(unsigned i=0; i<testN; i++)
			elementPool[i].randomise();
			
		testBinheap.reinit();
		
		std::sort(stdSortedElements.begin(), stdSortedElements.end(), PredFunctor());
		
		for(unsigned i=0; i<testN; i++)
			testSortedElements[i] = testBinheap.pop();
			
		for(unsigned i=0; i<testN; i++)
			gen_assert(testSortedElements[i]->val == stdSortedElements[i]->val);
		
		for(unsigned i=0; i<testN; i++) {
			for(unsigned j=0; j<testN; j++) {
				FastBinheap<Element, PredFunctor, NotifierFunctor> memLeakTest(1, PredFunctor(), NotifierFunctor());
				memLeakTest.push(elementPool + i);
			}
		}
		
		delete[] elementPool;
	}
}}
		

