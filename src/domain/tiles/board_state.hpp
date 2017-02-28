#pragma once

#include <algorithm>
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
	struct BoardState : public Permutation<H*W, H*W> {
		static_assert(H > 0 && W > 0, "");
		
		using base_t = Permutation<H*W, H*W>;
		
		using packed_t = typename base_t::Rank_t;
		
		static const size_t Max_Packed = base_t::maxRankTrunc();

		BoardState() = default;
		
		BoardState(std::array<tile_t, H*W> const& o) :
			base_t(o)
		{
			initBlankPos();
			slow_assert(base_t::valid());
		}
		
		BoardState(std::vector<tile_t> const& pVec) :
			base_t(pVec.begin(), pVec.end())
		{
			initBlankPos();
			slow_assert(base_t::valid());
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
	
	template<unsigned N, unsigned Sz>
	struct IndexMap {
		
		static const unsigned Null_Idx = (unsigned)-1;

		IndexMap(std::array<unsigned, N> const& pTileDropLevel, unsigned pLevel) {
			std::fill(mIdxOfTile.begin(), mIdxOfTile.end(), Null_Idx);
			
			unsigned pos = 0;
			
			for(unsigned t=1; t<N; t++) {
				if(pTileDropLevel[t] >= pLevel) {
					fast_assert(pos < Sz);
					mTileAtIdx[pos] = i;
					mIdxOfTile[i] = pos;
					++pos;
				}
			}
			
			fast_assert(pos == Sz);
			
			//for(unsigned i=0; i<Sz; i++)
		}
		
		unsigned indexOf(tile_t t) {
			slow_assert(t < N);
			i = mIdxOfTile[t];
			slow_assert(i != Null_Idx);
			return i;
		}
		
		tile_t tileAt(unsigned i) {
			slow_assert(i < Sz);
			return mTileAtIdx[i];
		}
		
		bool isMapped(tile_t t) {
			return mIdxOfTile[t] != Null_Idx;
		}
		
		private:
		std::array<tile_t, Sz> mTileAtIdx;
		std::array<unsigned, N> mIdxOfTile;
	};
	
	
	
	template<unsigned H, unsigned W, unsigned Nkept>
	struct SubsetBoardState : public BoardState<H, W> {
		using packed_t = typename Permutation<H*W, Nkept>::Rank_t;
		
		static const tile_t Null_Tile = (unsigned)-1;
		
		SubsetBoardState(BoardState const& bs, IndexMap<H*W, Nkept> const& pIdxMap) {
			for(unsigned i=0; i<H*W; i++) {
				tile_t t = bs[i];
				if(pIdxMap.isMapped[t])
					(*this)[i] = t;
				else
					(*this)[i] = Null_Tile;
			}
		}

		
		packed_t getPacked(IndexMap const& pIdxMap) const {
			Permutation<H*W, Nkept> pkd;
			
			for(unsigned i=0; i<H*W; i++) {
				if((*this)[i] != Null_Tile)
					pkd[pIdxMap.indexOf((*this)[i])] = i;
			}
			
			return pkd.getRank();
		}
		
		void fromPacked(packed_t const& pkd, IndexMap const& pIdxMap) const {
			std::fill(this->begin(), this->end(), Null_Tile);
			
			for(unsigned i=0; i<pIdxMap.size(); i++)
				(*this)[pkd[i]] = pIdxMap.tileAt(i);
		}
		
		void prettyPrint(std::ostream& out) const {
			for(unsigned i=0; i<H; i++) {
				for(unsigned j=0; j<W; j++) {
					if((*this)[i*W + j] == Null_Tile)
						out << ". ";
					else
						out << (*this)[i*W + j] << " ";
				out << "\n";
			}
		}
		
		bool valid() {
			unsigned n = 0;
			bool foundBlank = false;
			
			for(unsigned i=0; i<H*W; i++) {
				if((*this)[i] != Null_Tile)
					++n;
				else if((*this)[i] >= H*W)
					return false;
				if((*this)[i] == 0) {
					if(foundBlank)
						return false;
					else
						foundBlank = true;
				}
			}
			
			return n == Nkept && foundBlank;
		}
		
		bool valid(IndexMap const& pIdxMap) {
			unsigned n = 0;
			bool foundBlank = false;
			
			for(unsigned i=0; i<H*W; i++) {
				if((*this)[i] != Null_Tile)
					++n;
				else if((*this)[i] >= H*W)
					return false;
				else if(!pidxMap.isMapped((*this)[i])
					return false;

				if((*this)[i] == 0) {
					if(foundBlank)
						return false;
					else
						foundBlank = true;
				}
			}
			
			return n == Nkept && foundBlank;
		}
	};

}}
