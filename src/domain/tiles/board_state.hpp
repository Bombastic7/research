#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <vector>
#include "domain/tiles/defs.hpp"
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
	
	
	//Keeps a specification of how tiles are to be eliminated during abstraction.
	template<unsigned N>
	struct TilesAbtSpec {
		
		static const unsigned Null_Idx = (unsigned)-1;
		static const tile_t Null_Tile = (unsigned)-1;

		template<typename V>
		TilesAbtSpec(V const& pSpec)
		{
			fast_assert(pSpec.size() == N-1);
			
			for(unsigned i=0; i<N-1; i++)
				mAbtSpec[i] = pSpec[i];
			
			std::vector<unsigned> checkVec(pSpec.begin(), pSpec.end());
			std::sort(checkVec.begin(), checkVec.end());
			
			for(unsigned i=0; i<N-1; i++) {
				if(i != 0)
					fast_assert(checkVec[i] != 0 && (checkVec[i] == checkVec[i-1] || checkVec[i] == checkVec[i-1]+1));
			}
			
			mTopLevel = checkVec.back();
			
			unsigned nullIdx = Null_Idx;
			tile_t nullTile = Null_Tile;
			
			std::fill(mIdxOfTile.begin(), mIdxOfTile.end(), nullIdx);
			std::fill(mTileAtIdx.begin(), mTileAtIdx.end(), nullTile);
			mIdxOfTile[0] = 0;
			mTileAtIdx[0] = 0;
			unsigned pos = 1;
			
			for(unsigned lvl=mTopLevel; lvl>=1; lvl--) {
				for(unsigned i=0; i<N-1; i++) {
					if(mAbtSpec[i] == lvl) {
						mIdxOfTile[i+1] = pos;
						mTileAtIdx[pos] = i+1;
						++pos;
					}
				}
			}
		}
		
		unsigned tilesKeptAtLevel(unsigned pLevel) const {
			unsigned n=0;
			for(unsigned i=0; i<N-1; i++)
				if(mAbtSpec[i] > pLevel)
					++n;
			return n;
		}
		
		unsigned idxOfTile(tile_t t) const {
			slow_assert((unsigned)t < N);
			return mIdxOfTile[t];
		}
		
		tile_t tileAtIdx(unsigned i) const {
			slow_assert(i < N);
			tile_t t = mTileAtIdx[i];
			slow_assert(t != Null_Tile);
			return t;
		}
		
		private:
		std::array<unsigned, N-1> mAbtSpec;
		std::array<unsigned, N> mIdxOfTile;
		std::array<tile_t, N> mTileAtIdx;
		unsigned mTopLevel;

		
	
		
	};
	

	template<unsigned H, unsigned W, unsigned Nkept>
	struct SubsetBoardState : public BoardState<H, W> {
		using packed_t = typename Permutation<H*W, Nkept>::Rank_t;
		
		static const unsigned Null_Idx = (unsigned)-1;
		static const tile_t Null_Tile = (unsigned)-1;
		
		SubsetBoardState() = default;
		
		template<typename BS>
		SubsetBoardState(BS const& bs, TilesAbtSpec<H*W> const& pAbtSpec) {
			tile_t nullTile = Null_Tile;
			std::fill(this->begin(), this->end(), nullTile);
			
			for(unsigned i=0; i<H*W; i++) {
				tile_t t = bs[i];
				
				if(t == Null_Tile)
					continue;

				unsigned idx = pAbtSpec.idxOfTile(t);
				
				if(idx < Nkept)
					(*this)[i] = t;
				else
					(*this)[i] = Null_Tile;
				
				if(t == 0)
					this->mBlankPos = i;
			}
		}

		
		packed_t getPacked(TilesAbtSpec<H*W> const& pAbtSpec) const {
			Permutation<H*W, Nkept> pkd;
			
			for(unsigned i=0; i<H*W; i++) {
				if((*this)[i] != Null_Tile)
					pkd[pAbtSpec.idxOfTile((*this)[i])] = i;
			}
			
			return pkd.getRank();
		}
		
		void fromPacked(packed_t const& r, TilesAbtSpec<H*W> const& pAbtSpec) {
			tile_t nullTile = Null_Tile;
			std::fill(this->begin(), this->end(), nullTile);
			
			Permutation<H*W, Nkept> pkd;
			pkd.setRank(r);
			
			for(unsigned i=0; i<Nkept; i++)
				(*this)[pkd[i]] = pAbtSpec.tileAtIdx(i);
			
			this->initBlankPos();
		}
		
		void prettyPrint(std::ostream& out) const {
			for(unsigned i=0; i<H; i++) {
				for(unsigned j=0; j<W; j++) {
					if((*this)[i*W + j] == Null_Tile)
						out << ". ";
					else
						out << (*this)[i*W + j] << " ";
					}
				out << "\n";
			}
		}
		
		bool valid() const {
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
		
		bool valid(TilesAbtSpec<H*W> const& pAbtSpec) const {
			if(!valid())
				return false;

			for(unsigned i=0; i<H*W; i++) {
				if((*this)[i] != Null_Tile && pAbtSpec.idxOfTile((*this)[i]) >= Nkept)
					return false;
			}
			
			return true;
		}
	};

}}
