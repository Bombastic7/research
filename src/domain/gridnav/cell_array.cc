
#include "domain/gridnav/cell_array.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "domain/gridnav/defs.hpp"

#include "util/debug.hpp"
#include "util/math.hpp"


namespace mjon661 { namespace gridnav {
	

	void CellArray::write(std::ostream& out, unsigned pWidth) const {
		
		for(unsigned i=0; i<mSize; i++) {

			out << std::to_string(mCells[i]) << " ";
			
			if((i+1) % pWidth == 0) {
				out << "\n";
				std::cout << "\n";
			}
		}
	}
	
	void CellArray::read(std::istream& ins, std::vector<cell_t> const& pAllowedTypes) {
		
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
	

	cell_t CellArray::operator[](idx_t pIdx) const {
		slow_assert(pIdx >= 0 && pIdx < mSize);
		slow_assert(mIsInit);
		return mCells[pIdx];
	}

	cell_t* CellArray::data() const {
		return mCells;
	}
	
	unsigned CellArray::getSize() const {
		return mSize;
	}
	
	CellArray::CellArray(unsigned pSize) :
		mCells(nullptr),
		mSize(pSize),
		mIsInit(false)
	{
		mCells = new cell_t[pSize];
	}
	
	CellArray::~CellArray() {
		delete[] mCells;
	}

}}
