
#include "domain/gridnav/blocked/maps.hpp"

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "domain/gridnav/blocked/defs.hpp"
#include "domain/gridnav/cell_array.hpp"

#include "util/debug.hpp"
#include "util/math.hpp"


namespace mjon661 { namespace gridnav { namespace blocked {
	

	GridNav_Map::GridNav_Map(unsigned pHeight, unsigned pWidth) :
		CellArray(pHeight * pWidth),
		mHeight(pHeight),
		mWidth(pWidth)
	{}
	
	void GridNav_Map::read(std::istream& ins) {
		CellArray::read(ins, {0, 1});
	}
	
	template<typename InputIt>
	void GridNav_Map::read(InputIt first, InputIt last) {
		CellArray::read(first, last, {0,1});
	}
	
	void GridNav_Map::write(std::ostream& out) const {
		CellArray::write(out, mWidth);
	}
	
	unsigned GridNav_Map::getHeight() const {
		return mHeight;
	}
	
	unsigned GridNav_Map::getWidth() const {
		return mWidth;
	}



	

		
	GridNav_MapStack_MergeAbt::GridNav_MapStack_MergeAbt(
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
	
	
	GridNav_MapStack_MergeAbt::~GridNav_MapStack_MergeAbt() {
		for(unsigned i=0; i<mMaps.size(); i++)
			delete mMaps[i];
	}
	

	
	void GridNav_MapStack_MergeAbt::read(std::istream& ins) {
		
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
	
	GridNav_MapStack_MergeAbt::Lvl_t const& GridNav_MapStack_MergeAbt::getLevel(unsigned pLvl) const {
		return *mMaps.at(pLvl);
	}
			
	unsigned GridNav_MapStack_MergeAbt::getNLevels() const {
		return mNLevels;
	}
	
	unsigned GridNav_MapStack_MergeAbt::getBaseHeight() const {
		return mBaseHeight;
	}
	
	unsigned GridNav_MapStack_MergeAbt::getBaseWidth() const {
		return mBaseWidth;
	}
	


}}}