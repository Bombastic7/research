#pragma once

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "domain/gridnav/defs.hpp"
#include "domain/gridnav/cell_array.hpp"

#include "util/debug.hpp"
#include "util/math.hpp"


namespace mjon661 { namespace gridnav { namespace blocked {
	
	
	//Represents a board where cells are either blocked or unblocked.
	struct GridNav_Map : public CellArray {
		
		static const cell_t Open_Cell = 0, Blocked_Cell = 1;
		
		
		GridNav_Map(unsigned pHeight, unsigned pWidth);
		
		void read(std::istream& ins);
		
		template<typename InputIt>
		void read(InputIt first, InputIt last);
		
		void write(std::ostream& out) const;
		
		unsigned getHeight() const;
		
		unsigned getWidth() const;

		
		protected:
		const unsigned mHeight, mWidth;
	};
	
	
	
	/*
	Creates and manages a stack of GridNav_Map's. The value of each cell in levels greater than 0 (base level)
	 are determined by:
		
	cellValue(x, y, lvl):
		baseCells = Maps[lvl-1][[x_, y_] WHERE x*WidthFactor >= x_ < (x+1)*WidthFactor AND y*HeightFactor >= y_ < (y+1)*HeightFactor
			
		if baseCells.count(1) >= FillFactor:	//1 means blocked
			return 1
		else
			return 0
	
	
	i.e. for level L, we create level L+1 by dividing L in HeightFactor * WidthFactor rectangles of cells (a supercell). Each
	 	supercell in L represents one cell in L+1, and the cell value is 'blocked', iff their are FillFactor or more blocked cells
	 	in the supercell.
	
		Supercells start from (x=0, y=0), and any cells at the right and bottom edges that won't fit into a supercell are discarded.
	
	
	Retrieve references to levels [0, getNLevels()-1] with getLevel(). Each level is an initialised GridNavMap_OpOrBl.
	*/
	struct GridNav_MapStack_MergeAbt {
		
		using Lvl_t = GridNav_Map;
		
		
		GridNav_MapStack_MergeAbt(
			unsigned pBaseHeight,
			unsigned pBaseWidth,
			unsigned pMaxLevels,
			unsigned pHeightFactor,
			unsigned pWidthFactor,
			unsigned pFillFactor);
		
		
		~GridNav_MapStack_MergeAbt();
		
		GridNav_MapStack_MergeAbt(GridNav_MapStack_MergeAbt const&) = delete;
		

		
		void read(std::istream& ins);
		
		Lvl_t const& getLevel(unsigned pLvl) const;
				
		unsigned getNLevels() const;
		
		unsigned getBaseHeight() const;
		
		unsigned getBaseWidth() const;
		
		
		
		protected:

		
		const unsigned mBaseHeight, mBaseWidth, mMaxLevels, mHeightFactor, mWidthFactor, mFillFactor;
		unsigned mNLevels;
		std::vector<GridNav_Map*> mMaps;
	};

}}}
