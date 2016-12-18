#pragma once

#include <array>
#include <iostream>
#include <vector>
#include "domain/tiles/defs.hpp"
#include "domain/tiles/index_map.hpp"
#include "util/debug.hpp"
#include "util/permutation.hpp"


namespace mjon661 { namespace tiles {


	
	//Stores tiles as array[position] = tile at that position.
	template<int H, int W>
	struct BoardStateV : public Permutation<H*W, H*W> {
		static_assert(H > 0 && W > 0, "");
		
		using base_t = Permutation<H*W, H*W>;
		
		using packed_t = typename base_t::Rank_t;
		
		static const size_t Max_Packed = base_t::maxRankTrunc();
		
		
		
		BoardStateV() = default;
		
		BoardStateV(std::array<tile_t, H*W> const& o) :
			base_t(o)
		{
			initBlankPos();
		}
		
		BoardStateV(std::vector<tile_t> const& pVec) :
			base_t(pVec.begin(), pVec.end())
		{
			initBlankPos();
		}
		
		packed_t getPacked() const {
			return base_t::getRank();
		}
		
		void fromPacked(packed_t const& pkd) {
			base_t::setRank(pkd);
			initBlankPos();
		}
		
		void initBlankPos() {
			mBlankPos = find(0);
		}
		
		idx_t find(tile_t t) const {
			for(idx_t i=0; i<H*W; i++)
				if((*this)[i] == t)
					return i;
					
			gen_assert(false);
			return 0;
		}
		
		void swapTiles(idx_t a, idx_t b) {
			tile_t t = (*this)[a];
			(*this)[a] = b;
			(*this)[b] = t;
		}
		
		void moveBlank(idx_t i) {
			(*this)[mBlankPos] = (*this)[i];
			(*this)[i] = 0;
			mBlankPos = i;
		}
		
		idx_t getBlankPos() const {
			return mBlankPos;
		}
		
		void prettyPrint(std::ostream& out) const {
			for(unsigned i=0; i<H; i++) {
				for(unsigned j=0; j<W; j++)
					out << (*this)[i*W + j] << " ";
				out << "\n";
			}
		}
		

		idx_t mBlankPos;
	};
	
	
	
	
	
	
	// Stores the positions of Sz tiles only, the other N-Sz are ignored.
	// To get the positions of tile t, get t's index idx ([0, Sz-1]), then this[idx] = t's position.
	// tile 0 (the blank) must always be included, and will always have array index 0.
	//
	// The mapping of indices to tiles is not stored here. To execute a move of the blank, only the index of 
	//  the blank is needed.
	//
	// Stored as a k-permutation of N (where k == Sz).
	template<int H, int W, int Sz>
	struct BoardStateP : public Permutation<H*W, Sz> {
		
		using base_t = Permutation<H*W, Sz>;
		using packed_t = typename base_t::Rank_t;
		
		static const size_t Max_Packed = base_t::maxRankTrunc();
		static_assert(base_t::maxRankTrunc() == static_data::Factorial<H*W, Sz>::getValue()-1, "");
		
		BoardStateP() = default;
		
		BoardStateP(std::array<tile_t, Sz> const& pPosArray) :
			base_t(pPosArray)
		{}
		
		BoardStateP(BoardStateV<H, W> const& pV, IndexMap<H*W, Sz> const& pMap) {
			int nmapped = 0;
			
			for(int i=0; i<H*W; i++) {
				tile_t t = pV[i];
				
				if(pMap.isMapped(t)) {
					(*this)[pMap.indexOf(t)] = (tile_t) i;
					nmapped++;
				}
			}
			
			slow_assert(nmapped == Sz);
		}


		// If a tile has board position pPos,
		// set out = array index where that tile's position is recorded and return true.
		bool tryFindAt(idx_t pPos, idx_t& out) const {
			
			for(idx_t i=0; i<Sz; i++)
				if((*this)[i] == pPos) {
					out = i;
					return true;
				}
			
			return false;
		}
		
		void moveBlank(idx_t dest) {
			idx_t idxOfCollide;
			
			if(tryFindAt(dest, idxOfCollide))
				(*this)[idxOfCollide]  = (*this)[0];
			
			(*this)[0] = dest;
			slow_assert(this->valid());
		}
		
		packed_t getPacked() const {
			return base_t::getRank();
		}
		
		void fromPacked(packed_t const& pkd) {
			base_t::setRank(pkd);
		}
		
		idx_t getBlankPos() const {
			return (*this)[0];
		}
		
		void prettyPrint(IndexMap<H*W, Sz> const& pMap, std::ostream& out) const {
			std::array<tile_t, H*W> board;
			board.fill(-1);
			
			for(unsigned i=0; i<Sz; i++) {
				tile_t t = pMap.tileAt(i);
				idx_t pos = (*this)[i];
				
				board[pos] = t;// 
			}
			
			for(unsigned i=0; i<H*W; i++) {
				tile_t t = board[i];
				
				if(t < 0)
					out << ".\t";
					
				else
					out << std::to_string(t) << "\t";
				
				if((i+1) % W == 0)
					out << "\n";
			}
			
		}
	};













}}
