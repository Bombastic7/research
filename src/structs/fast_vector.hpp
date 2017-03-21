#pragma once

#include <algorithm>
#include <vector>
#include "util/debug.hpp"

namespace mjon661 {
	
	//Wrapper around a std::vector. Keeps track of used entries, so clear() returns all entries to be used, without 
	//	calling clear() on/deallocating the backing vector (which can be terribly slow.)
	template<typename Elm_t>
	class FastVector {
	
		public:
		static const unsigned Grow_Fact = 2;
		static const unsigned Def_Cap_Hint = 1024;

		FastVector(unsigned capHint = Def_Cap_Hint) : mSoftSize(0) {
			mData.resize(capHint);
		}
		
		void push_back(Elm_t const& e) {
			if(mSoftSize == mData.size())
				mData.resize(mData.size()*Grow_Fact);
			
			mData[mSoftSize++] = e;
		}
		
		Elm_t operator[](unsigned i) const {
			slow_assert(i < mSoftSize);
			return mData[i];
		}
		
		void clear() {
			mSoftSize = 0;
		}
		
		unsigned size() const {
			return mSoftSize;
		}
		
		unsigned capacity() const {
			return mData.size();
		}
		
		void swap(FastVector<Elm_t>& o) {
			mData.swap(o.mData);
			unsigned tempSize = o.mSoftSize;
			o.mSoftSize = mSoftSize;
			mSoftSize = tempSize;
		}
		
		private:
		
		std::vector<Elm_t> mData;
		unsigned mSoftSize;
	};
	
	/*
	template<typename Elm_t>
	class FastVector {
	
		public:
		static const unsigned Grow_Fact = 2;
		static const unsigned Def_Cap_Hint = 1024;

		FastVector(unsigned capHint = Def_Cap_Hint) : mVec(NULL), mSize(0), mCap(0) {
			fast_assert(capHint > 0);
			expandCapacity(capHint);
		}
		
		FastVector(const FastVector<Elm_t>&) = delete;
		
		~FastVector() {
			fast_assert(mVec);
			delete[] mVec;
		}
		
		void push_back(const Elm_t& e) {
			if(mSize == mCap) {
				fast_assert(!overflow::mul(mCap, Grow_Fact));
				expandCapacity(mCap * Grow_Fact);
			}
			mVec[mSize++] = e;
		}	
		
		Elm_t operator[](unsigned i) const {
			slow_assert(i < mSize);
			return mVec[i];
		}
		
		void clear() {
			mSize = 0;
		}
		
		void swap(FastVector<Elm_t>& o) {
			Elm_t* tempVec = o.mVec;
			unsigned tempSize = o.mSize;
			unsigned tempCap = o.mCap;
			o.mVec = mVec;
			o.mSize = mSize;
			o.mCap = mCap;
			mVec = tempVec;
			mSize = tempSize;
			mCap = tempCap;
		}
		
		unsigned size() const {
			return mSize;
		}
		
		unsigned capacity() const {
			return mCap;
		}
		
		void expandCapacity(unsigned newCap) {
			fast_assert(newCap > mCap);
			
			Elm_t* newVec = new Elm_t[newCap];
			
			if(mVec) {
				std::copy(mVec, mVec + mSize, newVec);
				delete[] mVec;
			}
			
			mVec = newVec;
			mCap = newCap;
		}
		
		private:

		Elm_t* mVec;
		unsigned mSize, mCap;
	};
	* */
}
