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
	

	//Encapsulates an array of cell_t.
	struct CellArray {
		
		void write(std::ostream& out, unsigned pWidth = -1) const;
		
		void read(std::istream& ins, std::vector<cell_t> const& pAllowedTypes = std::vector<cell_t>());
		
		
		template<typename InputIt>
		void read(InputIt first, InputIt last, std::vector<cell_t> const& pAllowedTypes = std::vector<cell_t>()) {
			
			for(unsigned i=0; i<mSize; i++, ++first) {
				if(first == last)
					throw std::runtime_error("CellArray::read: bad iterator");
				
				cell_t c = *first;
				
				if(!pAllowedTypes.empty() && !mathutil::contains(pAllowedTypes, c))
					throw std::runtime_error(std::string("CellArray::read: iterator, bad cell: ") + c);
				
				mCells[i] = c;
			}
			
			mIsInit = true;
		}
		

		cell_t operator[](idx_t pIdx) const;

		cell_t* data() const;
		
		unsigned getSize() const;
		
		CellArray(unsigned pSize);
		
		~CellArray();
		
		CellArray(CellArray const&) = delete;

		
		protected:
		cell_t* mCells;
		const unsigned mSize;
		bool mIsInit;
	};

}}
