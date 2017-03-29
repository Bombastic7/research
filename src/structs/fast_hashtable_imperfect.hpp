#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <stdexcept>
#include "structs/fast_vector.hpp"
#include "util/debug.hpp"
#include "util/math.hpp"


namespace mjon661 {

	template<typename Elm_t, 
			 typename WrappedBase_t, 
			 typename Key_t, 
			 typename Hasher, 
			 typename Comparer>
	class FastHashTableImperfect {
		public:
		
		using this_t = FastHashTableImperfect<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer>;
		
		static const unsigned Grow_Fact = 2;
		static const unsigned Fill_Fact = 3;
		static const unsigned Max_Table_Size = (1<<30) / sizeof(WrappedBase_t*); //Max table size is 1 GB.

		struct Wrapped_t : public WrappedBase_t {
			Wrapped_t* nxt;
			Wrapped_t() : nxt(nullptr) {}
		};
		
		FastHashTableImperfect(
			unsigned szHint,
			Hasher pHasher, 
			Comparer pComp)
			:
			mTableSize(szHint),
			mFill(0),
			mHasher(pHasher),
			mComp(pComp)
		{
			fast_assert(szHint > 0);
			mHashTable = new Wrapped_t*[szHint];
			std::fill(mHashTable, mHashTable + szHint, nullptr);
		}
		
		~FastHashTableImperfect() {
			delete[] mHashTable;
		}
		
		unsigned size() {
			return mTableSize;
		}
				
		unsigned getFill() {
			return mFill;
		}
				
		bool empty() {
			return mFill == 0;
		}

		void add(Elm_t* e_) {
			slow_assert(!overflow::mul(mFill, Grow_Fact));
			if(mFill * Fill_Fact > mTableSize && mTableSize < Max_Table_Size)
				resize(mathutil::min(Max_Table_Size, mTableSize * Grow_Fact));

			Wrapped_t* e = static_cast<Wrapped_t*>(e_);
			
			size_t h = mHasher(e) % mTableSize;

			unsigned idx = static_cast<unsigned>(h);
			
			if(!mHashTable[idx])
				mUsedIndices.push_back(idx);
			
			e->nxt = mHashTable[idx];
			mHashTable[idx] = e;

			mFill++;
		}
		
		Elm_t* find(const Key_t& k) {
			size_t h = mHasher(k) % mTableSize;
			unsigned idx = static_cast<unsigned>(h);
			
			for(Wrapped_t* e = mHashTable[idx]; e != nullptr; e = e->nxt) {
				if(mComp(e, k))
					return e;
			}
			return nullptr;
		}
		
		void clear() {
			for(unsigned i=0; i<mUsedIndices.size(); i++) {
				unsigned idx = mUsedIndices[i];
				slow_assert(mHashTable[idx]);
				
				mHashTable[idx] = nullptr;
			}
			mUsedIndices.clear();
			mFill = 0;
		}
		
		unsigned getMaxBucketLength() {
			unsigned maxBucketLength = 0;
			for(unsigned i=0; i<mUsedIndices.size(); i++) {
				Wrapped_t* nxt = nullptr;
				
				unsigned bucketLength = 0;
				for(Wrapped_t* e = mHashTable[mUsedIndices[i]]; e; e = nxt) {
					nxt = e->nxt;
					bucketLength++;
				}
				if(bucketLength > maxBucketLength)
					maxBucketLength = bucketLength;
			}
			return maxBucketLength;
		}
		
		std::string getChainLengthDist() {
			std::vector<unsigned> chainDist(getMaxBucketLength() + 1);
			
			for(unsigned i=0; i<mUsedIndices.size(); i++) {
				Wrapped_t* nxt = nullptr;
				
				unsigned bucketLength = 0;
				for(Wrapped_t* e = mHashTable[mUsedIndices[i]]; e; e = nxt) {
					nxt = e->nxt;
					bucketLength++;
				}
				chainDist[bucketLength]++;
			}
			
			std::string str;
			for(unsigned i=1; i<chainDist.size(); i++)
				str += std::to_string(i) + ": " + std::to_string(chainDist[i]) + "  ";
			
			return str;
		}
		
		Wrapped_t * const data() {
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
				if(mNode->nxt)
					mNode = mNode->nxt;
					
				else {
					mIndicesPos++;
					
					if(mIndicesPos == mInst.mUsedIndices.size())
						mNode = nullptr;
					else
						mNode = mInst.mHashTable[mInst.mUsedIndices[mIndicesPos]];
				}
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
		
		
		void resize(unsigned newSize) {
			Wrapped_t** oldHashTable = mHashTable;
			
			FastVector<unsigned> oldUsedIndices;
			mUsedIndices.swap(oldUsedIndices);
					
			fast_assert(mUsedIndices.size() == 0);
			
			try {
				mHashTable = new Wrapped_t*[newSize];
			} catch(std::bad_alloc const& e) {
				throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__) + " cursz=" + std::to_string(mTableSize) +
					" nextsz=" + std::to_string(newSize) + "\n\n" + e.what());
			}
			mTableSize = newSize;
			mFill = 0;
			
			std::fill(mHashTable, mHashTable + mTableSize, nullptr);
					
			for(unsigned j=0; j<oldUsedIndices.size(); j++) {
				Wrapped_t* nxt = nullptr;
				
				for(Wrapped_t* e = oldHashTable[oldUsedIndices[j]]; e; e = nxt) {
					nxt = e->nxt;
					add(e);
				}
			}
					
			delete[] oldHashTable;
		}
		
		Wrapped_t** mHashTable;
		FastVector<unsigned> mUsedIndices;
		unsigned mTableSize;
		unsigned mFill;
		Hasher mHasher;
		Comparer mComp;
	};
}
