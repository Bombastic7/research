#pragma once

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "domain/gridnav/defs.hpp"

#include "util/debug.hpp"
#include "util/math.hpp"


namespace mjon661 { namespace gridnav {
	

	struct CellArray {
		
		void write(std::ostream& out, unsigned pWidth = -1) const {
			
			for(unsigned i=0; i<mSize; i++) {

				out << std::to_string(mCells[i]) << " ";
				
				if((i+1) % pWidth == 0) {
					out << "\n";
					std::cout << "\n";
				}
			}
		}
		
		void read(std::istream& ins, std::vector<cell_t> const& pAllowedTypes = std::vector<cell_t>()) {
			
			for(unsigned i=0; i<mSize; i++) {
				if(!ins)
					throw std::runtime_error("CellArray::read: unexpected EOF");
				
				cell_t c;
				
				ins >> c;
				
				c -= '0';
				
				if(!pAllowedTypes.empty() && !contains(pAllowedTypes, c))
					throw std::runtime_error(std::string("CellArray::read: bad cell: ") + c);
				
				mCells[i] = c;
			}
			
			mIsInit = true;
		}
		
		
		template<typename InputIt>
		void read(InputIt first, InputIt last, std::vector<cell_t> const& pAllowedTypes = std::vector<cell_t>()) {
			
			for(unsigned i=0; i<mSize; i++, ++first) {
				if(first == last)
					throw std::runtime_error("CellArray::read: bad iterator");
				
				cell_t c = *first;
				
				if(!pAllowedTypes.empty() && !contains(pAllowedTypes, c))
					throw std::runtime_error(std::string("CellArray::read: iterator, bad cell: ") + c);
				
				mCells[i] = c;
			}
			
			mIsInit = true;
		}
		

		cell_t operator[](idx_t pIdx) const {
			slow_assert(pIdx >= 0 && pIdx < mSize);
			slow_assert(mIsInit);
			return mCells[pIdx];
		}

		cell_t* data() const {
			return mCells;
		}
		
		unsigned getSize() const {
			return mSize;
		}
		
		CellArray(unsigned pSize) :
			mCells(nullptr),
			mSize(pSize),
			mIsInit(false)
		{
			mCells = new cell_t[pSize];
		}
		
		~CellArray() {
			delete[] mCells;
		}
		
		CellArray(CellArray const&) = delete;

		
		protected:
		cell_t* mCells;
		const unsigned mSize;
		bool mIsInit;
	};
	
	
	struct GridNavMap_OpOrBl : public CellArray {
		
		static const cell_t Open_Cell = 0, Blocked_Cell = 1;
		
		
		GridNavMap_OpOrBl(unsigned pHeight, unsigned pWidth) :
			CellArray(pHeight * pWidth),
			mHeight(pHeight),
			mWidth(pWidth)
		{}
		
		void read(std::istream& ins) {
			CellArray::read(ins, {0, 1});
		}
		
		template<typename InputIt>
		void read(InputIt first, InputIt last) {
			CellArray::read(first, last, {0,1});
		}
		
		void write(std::ostream& out) const {
			CellArray::write(out, mWidth);
		}
		
		unsigned getHeight() const {
			return mHeight;
		}
		
		unsigned getWidth() const {
			return mWidth;
		}

		
		protected:
		const unsigned mHeight, mWidth;
	};
	
	
	
	
	
	struct GridNavMapStack_OpOrBl_MergeAbt {
		
		using Lvl_t = GridNavMap_OpOrBl;
		
		
		GridNavMapStack_OpOrBl_MergeAbt(
			unsigned pBaseHeight,
			unsigned pBaseWidth,
			unsigned pMaxLevels,
			unsigned pHeightFactor,
			unsigned pWidthFactor,
			unsigned pFillFactor) :
			
			mBaseHeight(pBaseHeight),
			mBaseWidth(pBaseWidth),
			mMaxLevels(pMaxLevels),
			mHeightFactor(pHeightFactor),
			mWidthFactor(pWidthFactor),
			mFillFactor(pFillFactor),
			mNLevels(0),
			mMaps({})
		{}
		
		
		~GridNavMapStack_OpOrBl_MergeAbt() {
			for(unsigned i=0; i<mMaps.size(); i++)
				delete mMaps[i];
		}

		
		void read(std::istream& ins) {
			
			fast_assert(mMaps.empty());
			
			mMaps.push_back(new Lvl_t(mBaseHeight, mBaseWidth));
			mMaps.back()->read(ins);
	
			
			
			while(true) {
				
				Lvl_t* prev = mMaps.back();
				unsigned prevW = prev->getWidth();
				unsigned prevH = prev->getHeight();
				
				unsigned selfHeight = prevH / mHeightFactor, selfWidth = prevW / mWidthFactor;
				
				if(mMaps.size() == mMaxLevels || selfHeight == 0 || selfWidth == 0)
					break;
						
				std::vector<unsigned> blockedCounts(selfHeight * selfWidth, 0);

				
				for(unsigned i=0; i<prev->getSize(); i++) {
					
					unsigned x = (i % prevW) / mWidthFactor, y = (i / prevW) / mHeightFactor;
					
					if(x >= selfWidth || y >= selfHeight)
						continue;
					
					if((*prev)[i] == 1)
						blockedCounts.at(y * selfWidth + x)++;
				}
				
				
				
				for(unsigned i=0; i<selfHeight * selfWidth; i++)
					blockedCounts[i] = blockedCounts[i] >= mFillFactor ? 1 : 0;
				
				Lvl_t* selfLvl = new Lvl_t(selfHeight, selfWidth);
				selfLvl->read(blockedCounts.begin(), blockedCounts.end());
				
				mMaps.push_back(selfLvl);
				
		
				
			}
			
			mNLevels = mMaps.size();
		}
		
		Lvl_t const& getLevel(unsigned pLvl) const {
			return *mMaps.at(pLvl);
		}
				
		unsigned getNLevels() const {
			return mNLevels;
		}
		
		unsigned getBaseHeight() const {
			return mBaseHeight;
		}
		
		unsigned getBaseWidth() const {
			return mBaseWidth;
		}
		
		
		
		protected:

		
		const unsigned mBaseHeight, mBaseWidth, mMaxLevels, mHeightFactor, mWidthFactor, mFillFactor;
		unsigned mNLevels;
		std::vector<GridNavMap_OpOrBl*> mMaps;
	};

}}
