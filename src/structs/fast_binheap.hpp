#pragma once

#include <algorithm>
#include "util/debug.hpp"


namespace mjon661 {
	
	namespace impl {
		namespace fastbinheap {
			struct DefIndexNotifier {
				template<typename Elm_t>
				void operator()(Elm_t*, unsigned) {}
			};
		}
	}
	
	template<typename Elm_t, typename Comparer, typename IdxNotifier = impl::fastbinheap::DefIndexNotifier>
	class FastBinheap {

		public:	
		static const unsigned Grow_Fact = 2;

		FastBinheap(unsigned szHint, 
			    Comparer pComp, 
			    IdxNotifier pNotifier = IdxNotifier()) :
				mComp(pComp),
				mNotifier(pNotifier),
				mSize(0),
				mCap(szHint)
		{
			mHeap = new Elm_t*[szHint];
		}
		
		~FastBinheap() {
			delete[] mHeap;
		}

		void push(Elm_t* e) {
			if(mSize == mCap)
				resize();
			mHeap[mSize] = e;
			mSize++;
			mNotifier(e, mSize - 1);
			pullup(mSize - 1);
		}
		
		Elm_t* pop() {
			slow_assert(!empty());
			
			Elm_t* res = mHeap[0];
			if(mSize > 1) {
				mHeap[0] = mHeap[mSize-1];
				mNotifier(mHeap[0], 0);
				mSize--;
				pushdown(0);
			} else mSize--;
			
			return res;
		}

		void resize() {
			fast_assert(!overflow::mul(mCap, Grow_Fact));
			unsigned newCap = mCap * Grow_Fact;
			
			Elm_t** newHeap = new Elm_t*[newCap];
			
			std::copy(mHeap, mHeap + mSize, newHeap);
				
			delete[] mHeap;
			mHeap = newHeap;
			mCap = newCap;
		}
		
		void reinit() {
			if (mSize == 0)
				return;

			for (unsigned i = mSize / 2; i != (unsigned)-1; i--)
				pushdown(i);
		}
		
		void clear() {
			mSize = 0;
		}

		void update(unsigned i) {
			slow_assert(i < mSize);
			i = pullup(i);
			pushdown(i);
		}
		
		Elm_t* at(unsigned i) {
			fast_assert(i < mSize);
			return mHeap[i];
		}
		
		bool empty() {
			return mSize == 0;
		}
		
		unsigned size() {
			return mSize;
		}
		
		unsigned capacity() {
			return mCap;
		}

		private:

		unsigned pullup(unsigned i) {
			if (i == 0)
				return i;
			unsigned p = parent(i);
			if(mComp(mHeap[i], mHeap[p])) {
				swap(i, p);
				return pullup(p);
			}
			return i;
		}
		
		unsigned pushdown(unsigned i) {
			unsigned l = left(i), r = right(i);

			unsigned sml = i;
			if (l < mSize && mComp(mHeap[l], mHeap[i]))
				sml = l;
			if (r < mSize && mComp(mHeap[r], mHeap[sml]))
				sml = r;

			if (sml != i) {
				swap(sml, i);
				return pushdown(sml);
			}
			return i;
		}
		
		void swap(unsigned i, unsigned j) {
			mNotifier(mHeap[i], j);
			mNotifier(mHeap[j], i);
			Elm_t* tmp = mHeap[i];
			mHeap[i] = mHeap[j];
			mHeap[j] = tmp;
		}
		
		unsigned parent(unsigned i) { slow_assert(!overflow::sub(i, 1u)); return (i - 1u) / 2; }

		unsigned left(unsigned i) { slow_assert(!overflow::mul(2u, i), "i=%u", i); return 2u * i + 1; }

		unsigned right(unsigned i) { slow_assert(!overflow::mul(2u, i)); return 2u * i + 2u; }

		
		Comparer mComp;
		IdxNotifier mNotifier;
		Elm_t** mHeap;
		unsigned mSize, mCap;
	};

}
