#pragma once

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>


namespace mjon661 { namespace gridnav {

	using coord_t = int;
	using idx_t = unsigned;
	using dist_t = double;
	
	
	const float SQRT2 = 1.41421356237;
	
	
	using cell_t = char;

	struct Pos {
		coord_t x, y;
	};	
	
	template<unsigned H, unsigned W>
	struct CellArray : public std::array<cell_t, H*W> {
		
		void write(std::ostream& out) const {
			for(unsigned i=0; i<H*W; i++) {
				if(i % W == 0) {
					out << "\n";
					std::cout << "\n";
				}
				
				std::cout << std::to_string((*this)[i]) << " ";
				out << std::to_string((*this)[i]) << " ";
			}
		}
		
		void read(std::istream& ins) {
			
			for(unsigned i=0; i<H*W; i++) {
				if(!ins)
					throw std::runtime_error("CellArray::read: unexpected EOF");
				
				cell_t c;
				
				ins >> c;
				
				if(c == '0')
					(*this)[i] = 0;
				else if(c == '1')
					(*this)[i] = 1;
				else
					throw std::runtime_error(std::string("CellArray::read: bad cell: ") + c);
			}
			
			for(unsigned i=0; i<H*W; i++) {
				slow_assert((*this)[i] == 0 || (*this)[i] == 1);
				
			}
			
		}
	};

}}
