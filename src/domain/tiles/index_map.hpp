#pragma once

#include <array>
#include <iostream>
#include "domain/tiles/defs.hpp"
#include "util/debug.hpp"


namespace mjon661 { namespace tiles {

	//Creates and stores a mapping of Sz unique tiles (with values [0, N-1]) to array indices [0, Sz-1].
	//Blank/0 must be one of those tiles, and it will always be assigned index 0.
	template<int N, int Sz>
	struct IndexMap {

		IndexMap() {
			for(int i=0; i<Sz; i++) {
				mIdx_Tile[i] = i;
				mTile_Idx[i] = i;
			}
		}
		
		//Iterator to Sz tiles. 0 must be among them.
		template<typename InputIt>
		IndexMap(InputIt first, InputIt last) :
			mIdx_Tile{},
			mTile_Idx{},
			mTileMapped{}
		{
			mIdx_Tile[0] = 0;
			mTile_Idx[0] = 0;
			
			
			for(idx_t i=1; i<Sz; ++first) {
				fast_assert(first != last);
				
				tile_t t = *first;
				fast_assert(t >= 0 && t < N);
				
				fast_assert(!mTileMapped[t]);
				mTileMapped[t] = true;
				
				if(t == 0)
					continue;
				
				mTile_Idx[t] = i;
				mIdx_Tile[i] = t;
				i++;

			}
				
			fast_assert(first == last || Sz==1, "%d %d", *first, *last);
		}

		idx_t indexOf(tile_t t) const {
			fast_assert(mTileMapped[t] && t >= 0 && t < N);
			return mTile_Idx[t];
		}
		
		tile_t tileAt(idx_t i) const {
			fast_assert(i >= 0 && i < Sz);
			return mIdx_Tile[i];
		}
		
		bool isMapped(tile_t t) const {
			fast_assert(t >= 0 && t < N);
			return mTileMapped[t];
		}

		//For each tile t mapped by this IndexMap, with assigned array position idx, produce an
		// array of Sz indices Tr such that Tr[idx] is the index of t in pOrgMap.
		template<int OrgSz>
		std::array<idx_t, Sz> translateIndices(IndexMap<N, OrgSz> const& pOrgMap) const {
			
			std::array<idx_t, Sz> trns;
			
			for(idx_t selfIdx=0; selfIdx<Sz; selfIdx++) {
				tile_t t = mTile_Idx[selfIdx];
				trns[selfIdx] = pOrgMap.indexOf(t); // This will throw if t is not in pOrgMap
			}
			return trns;
		}

		private:
		
		std::array<idx_t, Sz> mIdx_Tile;
		std::array<tile_t, N> mTile_Idx;
		std::array<bool, N> mTileMapped;

	};
	
	template<int N, int Sz>
	std::ostream& operator<<(std::ostream& out, IndexMap<N,Sz> const& pMap) {
		
		out << "IndexMap<" << N << "," << Sz << ">: ";
		
		for(int i=0; i<Sz; i++)
			out << "(" << i << ", " << pMap.tileAt(i) << ") ";
			
		return out;
	}
}}
