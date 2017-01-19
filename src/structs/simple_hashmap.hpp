#pragma once

#include <algorithm>
#include <array>
#include <vector>
#include "util/debug.hpp"
#include "structs/object_pool.hpp"


namespace mjon661 {

	template<typename K, typename V, unsigned Table_Size>
	class SimpleHashMap {
		static_assert(Table_Size > 0, "");
		
		public:
		
		using this_t = SimpleHashMap<K, V, Table_Size>;
		

		struct HTentry {
			const K key;
			V val;
			HTentry* nxt;
			
			HTentry(K pKey) : key(pKey), val(), nxt(nullptr) {}
		};
		
		struct ModuloHash {
			static size_t doHash(K pKey) {
				return pKey % Table_Size;
			};
		};


		
		SimpleHashMap() :
			mDummyEntry(0),
			mHashTable(),
			mKeyList(),
			mPool(),
			mFill(0)
		{
			mHashTable.fill(nullptr);
		}
		
		unsigned size() {
			return Table_Size;
		}
				
		unsigned getFill() {
			return mFill;
		}
				
		bool empty() {
			return mFill == 0;
		}
		
		bool contains(K pKey) {
			unsigned h = ModuloHash::doHash(pKey);
			slow_assert(h < Table_Size);
			
			if(!mHashTable[h])
				return false;
			
			HTentry* current = mHashTable[h];
			
			while(current->nxt) {
				if(current->key == pKey)
					return true;
				
				current = current->nxt;
			}

			return false;
		}


		HTentry& operator[](K pKey) {
			
			unsigned h = ModuloHash::doHash(pKey);
			
			slow_assert(h < Table_Size);
			
			if(!mHashTable[h]) {
				mHashTable[h] = mPool.construct(pKey);
				mKeyList.push_back(pKey);
				mFill++;
				return *(mHashTable[h]);
			}
			
			HTentry* current = mHashTable[h];
			
			while(true) {
				
				if(current->key == pKey)
					return *current;
				
				if(!current->nxt) {
					mFill++;
					current->nxt = mPool.construct(pKey);
					mKeyList.push_back(pKey);
					return *(current->nxt);
				}
				
				current = current->nxt;
			}
			
			gen_assert(false);
			return mDummyEntry;
		}


		void clear() {
			for(unsigned i=0; i<mKeyList.size(); i++)
				mHashTable[ModuloHash::doHash(mKeyList[i])] = nullptr;

			mPool.clear();
			mKeyList.clear();
			mFill = 0;
		}
		
		std::vector<K> const& orderedKeys() {
			std::sort(mKeyList.begin(), mKeyList.end());
			return mKeyList;
		}
		
		std::vector<K> const& unorderedKeys() {
			return mKeyList;
		}

		
		HTentry mDummyEntry;
		std::array<HTentry*, Table_Size> mHashTable;
		std::vector<K> mKeyList;
		ObjectPool<HTentry> mPool;
		unsigned mFill;
	};
}
