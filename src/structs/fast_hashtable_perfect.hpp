#pragma once

#include <algorithm>
#include <string>
#include "structs/fast_vector.hpp"
#include "util/debug.hpp"


namespace mjon661 {

	template<typename Elm_t, 
			 typename WrappedBase_t, 
			 typename Key_t, 
			 typename Hasher,
			 typename Comparer>
	class FastHashTablePerfect {
		public:
		
		using this_t = FastHashTablePerfect<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer>;

		using Wrapped_t = WrappedBase_t;
		
		FastHashTablePerfect(
			unsigned pTableSize,
			Hasher pHasher,
			Comparer)
			:
			mTableSize(pTableSize),
			mFill(0),
			mHasher(pHasher)
		{
			fast_assert(pTableSize > 0);
			mHashTable = new Wrapped_t*[pTableSize];
			std::fill(mHashTable, mHashTable + pTableSize, nullptr);
		}
		
		~FastHashTablePerfect() {
			delete[] mHashTable;
		}
		
		unsigned size() const {
			return mTableSize;
		}
				
		unsigned getFill() const {
			return mFill;
		}
				
		bool empty() const {
			return mFill == 0;
		}
		
		void add(Elm_t* e_) {
			Wrapped_t* e = static_cast<Wrapped_t*>(e_);
			
			size_t h = mHasher(e);
			unsigned idx = static_cast<unsigned>(h);
			
			if(mTableSize <= idx)
				resize(idx+1);
			
			slow_assert(idx < mTableSize, "%u %u", idx, mTableSize);
			slow_assert(!mHashTable[idx]);
			
			mUsedIndices.push_back(idx);
			mFill++;
			
			mHashTable[idx] = e;
		}
		
		Elm_t* find(const Key_t& k) const {
			size_t h = mHasher(k);
			
			unsigned idx = static_cast<unsigned>(h);
			if(idx >= mTableSize)
				return nullptr;
			//slow_assert(idx < mTableSize, "%u %u", idx, mTableSize);

			return mHashTable[idx];
		}
		
		void clear() {
			for(unsigned i=0; i<mUsedIndices.size(); i++) {
				unsigned idx = mUsedIndices[i];
				glacial_assert(mHashTable[idx]);
				
				mHashTable[idx] = nullptr;
			}
			mUsedIndices.clear();
			mFill = 0;

			glacial_assert(check_clear());
		}
		
		bool check_clear() {
			for(unsigned i=0; i<mTableSize; i++)
				if(mHashTable[i])
					return false;
			return true;
		}
		
		unsigned getMaxBucketLength() const {
			return 1;
		}
		
		std::string getChainLengthDist() const {
			return std::string("1: -");
		}
		
		Wrapped_t * const data() const {
			return mHashTable;
		}
		
		
		// Iterator //

		class iterator {
			friend this_t;
			
			iterator(this_t const& pInst, bool makeEnd) : mInst(pInst), mNode(nullptr), mIndicesPos(0)
			{
				if(makeEnd || pInst.mUsedIndices.size() == 0)
					return;
				
				mNode = pInst.mHashTable[pInst.mUsedIndices[0]];
			}
			
			public:
			
			bool operator==(iterator const& o) {
				return mNode == o.mNode;
			}
			
			bool operator!=(iterator const& o) {
				return mNode != o.mNode;
			}
			
			iterator& operator++() {
				mIndicesPos++;
					
				if(mIndicesPos == mInst.mUsedIndices.size())
					mNode = nullptr;
				else
					mNode = mInst.mHashTable[mInst.mUsedIndices[mIndicesPos]];
				
				return *this;
			}

			Elm_t * const operator*() {
				return mNode;
			}
			
			private:
			
			this_t const& mInst;
			Wrapped_t* mNode;
			unsigned mIndicesPos;
		};
		
		iterator begin() {
			return iterator(*this, false);
		}
		
		iterator end() {
			return iterator(*this, true);
		}
	
	
	
		private:
		friend class iterator;
		
		void resize(unsigned pMinSize) {
			unsigned newSz = mTableSize;
			while(newSz < pMinSize) newSz *= 2;
			
			logDebugStream() << "trying to alloc " << newSz << "\n";
			g_logDebugOfs.flush();
			Wrapped_t** newTable = new Wrapped_t*[newSz];
			std::fill(newTable, newTable+newSz, nullptr);
			
			for(unsigned i=0; i<mUsedIndices.size(); i++) {
				unsigned idx = mUsedIndices[i];
				newTable[idx] = mHashTable[idx];
			}
			
			delete[] mHashTable;
			mHashTable = newTable;
			mTableSize = newSz;
		}
		
		Wrapped_t** mHashTable;
		FastVector<unsigned> mUsedIndices;
		unsigned mTableSize;
		unsigned mFill;
		Hasher mHasher;
	};
}
