#pragma once

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>


namespace mjon661 { namespace gridnav {

	using coord_t = int;
	using idx_t = unsigned;
	using dist_t = double;
	
	
	const float SQRT2 = 1.41421356237;
	
	
	using cell_t = char;

	struct Pos {
		coord_t x, y;
	};	
	
	/*
	template<unsigned H, unsigned W>
	struct CellArray : public std::array<cell_t, H*W> {
		
		void write(std::ostream& out) const {
			for(unsigned i=0; i<H*W; i++) {

				out << ((*this)[i] + '0') << " ";
				
				if((i+1) % W == 0) {
					out << "\n";
					std::cout << "\n";
				}
			}
		}
		
		void read(std::istream& ins, std::vector<cell_t> const& pAllowedTypes = std::vector<cell_t>()) {
			
			for(unsigned i=0; i<H*W; i++) {
				if(!ins)
					throw std::runtime_error("CellArray::read: unexpected EOF");
				
				cell_t c;
				
				ins >> c;
				
				c -= '0';
				
				if(!pAllowedTypes.empty() && !contains(pAllowedTypes, c))
					throw std::runtime_error(std::string("CellArray::read: bad cell: ") + c);
				
				(*this)[i] = c;
			}
			
		}
	};
	
	template<unsigned H, unsigned W>
	struct CellArray_OpenOrBlocked : CellArray<H, W> {
		
		static const cell_t Open_Cell = 0, Blocked_Cell = 1;
		
		
		void read(std::istream& ins) {
			CellArray<H, W>::read(ins, {0, 1});
		}
	};
	*/
}}
