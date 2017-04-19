#pragma once

#include "domain/gridnav/dim2/common.hpp"


namespace mjon661 { namespace gridnav {




namespace starabt {

	struct HubPrioComp {
		bool operator()(std::pair<unsigned, unsigned> const& a, std::pair<unsigned, unsigned> const& b) const {
			return a.first != b.first ? a.first > b.first : a.second < b.second;
		}
	};
	
	template<typename AdjEdgeIt>
	struct StarAbtInfo {
		

		using Trns_t = std::vector<unsigned>;
		using Edge_t = std::pair<unsigned, Cost_t>;
		using StateEntry_t = std::vector<Edge_t>;
		using LevelEntry_t = std::vector<StateEntry_t>;
		
		

		std::vector<EdgeList_t> mLevels;
		std::vector<Trns_t> mAllTrns;
		const unsigned mMaxDepth;
		
		
		bool gen_base(CellMap2D<> const& pCellMap) {
			mAllTrns.emplace_back();
			mAllTrns[0].resize(pCellMap.cells().size(), (unsigned)-1);
			
			unsigned c = 0;
			for(unsigned i=0; i<pCellMap.cells().size(); i++) {
				if(pCellMap.cells()[i] == CellMap2D<>::Cell_t::Open)
					mAllTrns[0][i] = c++;
			}
		
			mLevels.emplace_back();
			mLevels[0].resize(c);
			
			bool isTrivial = true;
			
			for(unsigned i=0; i<pCellMap.cells().size(); i++) {
				if(pCellMap.cells()[i] == CellMap2D<>::Cell_t::Open)
					AdjEdgeIt it(pCellMap, i);
					
					while(!it.finished()) {
						mLevel[0][ mAllTrns[0][i] ].push_back(mAllTrns[0][it.state()], it.cost());
						it.next();
						isTrivial = false;
					}
			}
			
			return isTrivial;
		}
		
		unsigned mapAbtRec(unsigned curlvl, unsigned s, unsigned a, unsigned d) {
			if(mAllTrns[curlvl][s] != (unsigned)-1 || d > mMaxDepth)
				return 0;
			
			mAllTrns.at(curlvl).at(s) = a;
			
			unsigned c = 1;
			for(auto const& e : mLevels[curlvl][s])
				c += mapAbtRec(curlvl, e.first, a, d+1);
			return c;
		}
		
		
		unsigned gen_trns(unsigned curlvl) {
			std::vector<std::pair<unsigned,unsigned>> hublist;
			for(unsigned i=0; i<mLevels.at(curlvl).size(); i++)
				hublist.push_back(mLevels[curlvl][i].size(), i);
				
			std::sort(hublist.begin(), hublist.end(), HubPrioComp());
			
			std::vector<unsigned> sgl;
			
			mAllTrns.emplace_back();
			mAllTrns.at(curlvl).resize(mLevels.at(curlvl).size(), (unsigned)-1);
			
			for(auto const& hb : hublist) {
				if(mapAbtRec(curlvl, s, s, 0) == 1)
					sgl.push_back(s);
			}
			
			for(unsigned s : sgl) {
				for(auto const& e : mLevels[curlvl][s]) {
					mAllTrns[curlvl][s] = mAllTrns[curlvl][e.first];
					break;
				}
			}
			
			std::map<unsigned, unsigned rlblmap;
			unsigned c = 0;
			
			for(unsigned i=0; i<mAllTrns[curlvl].size(); i++) {
				unsigned a = mAllTrns[curlvl][i];
				if(rlblmap.count(a) == 0)
					rlblmap[a] = c++;
				mAllTrns[curlvl][i] = rlblmap.at(a);
			}
			
			return c;
		}
		
		void gen_edges(unsigned curlvl, unsigned nstates) {
			mLevels.emplace_back();
			mLevels.at(curlvl).resize(nstates);
			
			for(unsigned bsrc=0; bsrc<mLevels.at(curlvl-1).size(); bsrc++) {
				
				for(auto const& be : mLevels[curlvl-1][bsrc]) {
					unsigned bdst = be.first;
					
					bool existing = false;
					
					for(auto& e : mLevels[curlvl][mAllTrns[bsrc]]) {
						if(e.first == mAllTrns.at(bdst)) {
							existing = true;
							
							if(e.second > be.second)
								e.second = be.second;
							break;
						}
					}
					
					if(!existing)
						mLevels[curlvl][mAllTrns[bsrc]].push_back({mAllTrns.at(bdst), be.cst});
				}	
			}
		}
		
		StarAbtInfo(CellMap2D<> const& mCellMap, unsigned pMaxDepth) :
			mMaxDepth(pMaxDepth)
		{
			mLevels.clear();
			mAllTrns.clear();
			
			bool isTrivial = gen_base(mCellMap);
			
			unsigned curlvl = 1;
			
			while(!isTrivial) {
				unsigned nstates = gen_trns(curlvl-1);
				isTrivial = gen_edges(curlvl, nstates);
			}
		}
	};
	


}





}}
