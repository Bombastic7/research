#include <vector>
#include <cstdlib>
#include <set>
#include "structs/fast_hashtable.hpp"
#include "util/debug.hpp"


namespace mjon661 { namespace test {
		
	namespace fasthashtableimpl {
		
		struct Element {
			size_t hashValue;
			unsigned uniqueValue;
		};
	
		struct Key {
			size_t hashValue;
			unsigned uniqueValue;
		};
		
		struct ElementKeyCompare {
			bool operator()(Element* const e, Key const& k) const {
				return e->uniqueValue == k.uniqueValue;
			}
		};
		
		struct ElementKeyHasher {
			size_t operator()(Element* const e) const {
				return e->hashValue;
			}
			
			size_t operator()(Key const& k) const {
				return k.hashValue;
			}
		};
	
	
		template<typename HT>
		void test_hashtable(unsigned testN, unsigned hashMod, bool uniqueHash, unsigned initSize) {
			using namespace fasthashtableimpl;
			
			typename HT::Wrapped_t* memPool = new typename HT::Wrapped_t[testN];
			Element** elementPool = new Element*[testN];
			Key* keyPool = new Key[testN];
			
			std::set<size_t> usedHashes;
			
			for(unsigned i=0; i<testN; i++) {
				unsigned r = rand() % hashMod;
				
				while(uniqueHash && usedHashes.count(r) == 1) {
					r = rand() % hashMod;
				}
				
				usedHashes.insert(r);
				
				elementPool[i] = memPool + i;
						
				elementPool[i]->uniqueValue = i;
				elementPool[i]->hashValue = r;
				
				keyPool[i].uniqueValue = i;
				keyPool[i].hashValue = r;
			}
			
			if(uniqueHash) {
				for(unsigned i=0; i<testN; i++) {
					bool b = false;
					for(unsigned j=0; j<testN; j++) {
						if(elementPool[j]->hashValue == i) {
							gen_assert(!b);
							b = true;
						}
					}
					gen_assert(b);
				}
			}
			
			
			HT testHT(initSize, ElementKeyHasher(), ElementKeyCompare());
			
			for(unsigned i=0; i<testN; i++)
				testHT.add(elementPool[i]);
			
			
			testHT.clear();
			
			for(unsigned i=0; i<testN; i++) {
				gen_assert(elementPool[i]->uniqueValue == keyPool[i].uniqueValue);
				gen_assert(elementPool[i]->hashValue == keyPool[i].hashValue);
			}
			
			for(unsigned i=0; i<testN; i++)
				testHT.add(elementPool[i]);
			
			for(unsigned i=0; i<testN; i++) {
				Element* foundNode = testHT.find(keyPool[i]);
				foundNode = foundNode; //Supress warning
				gen_assert(foundNode);
				gen_assert(foundNode->uniqueValue == i);
			}
			
			bool* iteratedUniques = new bool[testN]();
			unsigned iteratedCount = 0;
			
			for(typename HT::iterator it = testHT.begin(); it != testHT.end(); ++it) {
				Element* e = *it;
				gen_assert(!iteratedUniques[e->uniqueValue]);
				
				iteratedUniques[e->uniqueValue] = true;
				iteratedCount++;
			}
			
			gen_assert(iteratedCount == testN);
			delete[] iteratedUniques;
			
			testHT.clear();
			
			for(unsigned i=0; i<testN; i++) {
				HT memLeakTest(initSize, ElementKeyHasher(), ElementKeyCompare());
				
				for(unsigned j=0; j<testN; j++)
					memLeakTest.add(elementPool[j]);
			}
			
			delete[] keyPool;
			delete[] elementPool;
			delete[] memPool;
		}
	}
	
	
	
	void test_fast_hashtable(unsigned testN) {
		using namespace fasthashtableimpl;
		using HTimperfect = FastHashTable<Element, Element, Key, ElementKeyHasher, ElementKeyCompare, false>;
		using HTperfect = FastHashTable<Element, Element, Key, ElementKeyHasher, ElementKeyCompare, true>;
		
		test_hashtable<HTimperfect>(testN, testN/3, false, 1);
		test_hashtable<HTperfect>(testN, testN, true, testN);
	}
}}
