#pragma once

#include "structs/fast_hashtable_perfect.hpp"
#include "structs/fast_hashtable_imperfect.hpp"

#include "util/debug.hpp"


namespace mjon661 {
	
	namespace impl {
		template<typename Elm_t, 
			 typename WrappedBase_t, 
			 typename Key_t, 
			 typename Hasher, 
			 typename Comparer,
			 bool Perfect_Hash>
		class FastHashTableBounce;
		
		template<typename Elm_t, 
			 typename WrappedBase_t, 
			 typename Key_t, 
			 typename Hasher,
			 typename Comparer>
		class FastHashTableBounce<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer, true>
			: public FastHashTablePerfect<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer>
		{
			using FastHashTablePerfect<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer>::FastHashTablePerfect;
		};
		
		template<typename Elm_t, 
			 typename WrappedBase_t, 
			 typename Key_t, 
			 typename Hasher, 
			 typename Comparer>
		class FastHashTableBounce<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer, false>
			: public FastHashTableImperfect<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer>
		{
			using FastHashTableImperfect<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer>::FastHashTableImperfect;
		};
	}
	
	template<typename Elm_t, 
		typename WrappedBase_t, 
		typename Key_t, 
		typename Hasher, 
		typename Comparer,
		bool Perfect_Hash>
	class FastHashTable : 
		public impl::FastHashTableBounce<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer, Perfect_Hash>
	{
		using impl::FastHashTableBounce<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer, Perfect_Hash>::FastHashTableBounce;
	};
	
	/*
	
	template<typename Elm_t, typename Comparer>
	struct HTBase_Perfect {
		
		using Wrapped_t = Elm_t;
		
		static bool addToBin(Wrapped_t** pBin, Wrapped_t* pElm) {
			slow_assert(!pBin);
			*pBin = pElm;
			return true;
		}
		
		static Wrapped_t* findInBin(Wrapped_t** pBin, Elm_t const& pElm, Comparer const& pComp) {
			if(*pBin)
				slow_assert(pComp(*pBin, pElm));
			
			return *pBin;
		}
	};

	template<typename Elm_t, typename Comparer>
	struct HTBase_Imperfect {
		
		struct Wrapped_t : public Elm_t {
			Wrapped_t* nxt;
		};
		
		static bool addToBin(Wrapped_t** pBin, Wrapped_t* pElm) {
			pElm->nxt = *pBin;
			*pBin = pElm;

			return pElm->nxt;
		}
		
		static Wrapped_t* findInBin(Wrapped_t** pBin, Elm_t const& pElm, Comparer const& pComp) {
			
			for(Wrapped_t* n=*pBin; n; n=n->nxt)
				if(pComp(n, pElm))
					return n;

			return nullptr;
		}
	};
	
	
	template<typename Elm_t, typename WrappedBase_t,  typename Key_t, typename Hasher, typename Comparer, bool Use_Perfect>
	struct FastHashTable {
		
		
		void add(Elm_t* pElm) {
			
		}
		
		void find(Key_t const& pKey) {
			
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
		
		void clear() {
			for(unsigned i=0; i<mUsedIndices.size(); i++) {
				unsigned idx = mUsedIndices[i];
				glacial_assert(mHashTable[idx]);
				
				mHashTable[idx] = nullptr;
			}
			mUsedIndices.clear();
			mFill = 0;

			for(unsigned i=0; i<mTableSize; i++)
				glacial_assert(!mHashTable[i]);
		}
		
		
		
		std::vector<unsigned> mUsedIndices;
		unsigned mUsedIndicesPos;
		
	};
	*/
}
