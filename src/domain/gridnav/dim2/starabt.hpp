#pragma once

#include "domain/gridnav/dim2/common.hpp"


namespace mjon661 { namespace gridnav { namespace dim2 {


	//Generates abstract domains for a given CellMap2D, abstraction radius and edge iterator.
	//Uses STAR abstraction method.
	
	//"base" level is the level of the argument CellMap2D. Each state is an open cell, represented by a number between 0 and the map 
	//	size minus 1, which is its position.
	
	//StarAbtInfo has levels numbered 0 onwards. Each level has a associated list of state edges, and a abstract mapping from each 
	//	level to the next. Each state of level i is a number from 0 onwards. 
	
	//mLevels holds edge information for all levels. An edge is a pair {dst, cost}. To retrieve the list of edges for state s of level l,
	//	use mLevels[l][s]. Each edge is to another state of level l.
	
	//mAllTrns is a list of mappings that abstract each state of some level to a state of the next level. To abstract state s of level l,
	//	to its abstract state in level l+1, use mAllTrns[l][s].
	
	//Level 0 is a bijective mapping of states from the base domain. Each open cell is represented by a number 0 onwards, instead
	//	of position, to allow for efficient indexing.
	
	//To map a base state bs to a level 0 state, use mBaseTrns[bs].
	
	//Note: number of levels is [mLevels.size()], the highest level is [mLevels.size()-1].
	//	Number of states in level i is [mLevels[i].size()-1].
	//	[mAllTrns.size()] == number of levels - 1.
	
	
	template<typename Cost_t, typename AdjEdgeIt>
	struct StarAbtInfo {

		using Trns_t = std::vector<unsigned>;
		using Edge_t = std::pair<unsigned, Cost_t>;
		using StateEntry_t = std::vector<Edge_t>;
		using LevelEntry_t = std::vector<StateEntry_t>;
		
		
		
		struct HubPrioComp {
			bool operator()(std::pair<unsigned, unsigned> const& a, std::pair<unsigned, unsigned> const& b) const {
				return a.first != b.first ? a.first > b.first : a.second < b.second;
			}
		};


		
		bool gen_base(CellMap2D<> const& pCellMap) {
			mBaseTrns.resize(pCellMap.cells().size(), (unsigned)-1);
			
			unsigned c = 0;
			for(unsigned i=0; i<pCellMap.cells().size(); i++) {
				if(pCellMap.cells()[i] == CellMap2D<>::Cell_t::Open)
					mBaseTrns[i] = c++;
			}
		
			mLevels.emplace_back();
			mLevels[0].resize(c);
			
			bool isTrivial = true;
			
			for(unsigned i=0; i<pCellMap.cells().size(); i++) {
				if(pCellMap.cells()[i] == CellMap2D<>::Cell_t::Open) {
					AdjEdgeIt it(pCellMap, i);
					
					while(!it.finished()) {
						mLevels[0][ mBaseTrns[i] ].push_back({mBaseTrns[it.state()], it.cost()});
						it.next();
						isTrivial = false;
					}
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
			fast_assert(mLevels.size() == curlvl+1);
			fast_assert(mAllTrns.size() == curlvl);
			
			std::vector<std::pair<unsigned,unsigned>> hublist;
			for(unsigned i=0; i<mLevels.at(curlvl).size(); i++)
				hublist.push_back({mLevels[curlvl][i].size(), i});
				
			std::sort(hublist.begin(), hublist.end(), HubPrioComp());
			
			std::vector<unsigned> sgl;
			
			mAllTrns.emplace_back();
			mAllTrns.at(curlvl).resize(mLevels.at(curlvl).size(), (unsigned)-1);
			
			for(auto const& hb : hublist) {
				if(mapAbtRec(curlvl, hb.second, hb.second, 0) == 1)
					sgl.push_back(hb.second);
			}
			
			for(unsigned s : sgl) {
				for(auto const& e : mLevels[curlvl][s]) {
					mAllTrns[curlvl][s] = mAllTrns[curlvl][e.first];
					break;
				}
			}
			
			std::map<unsigned, unsigned> rlblmap;
			unsigned c = 0;
			
			for(unsigned i=0; i<mAllTrns[curlvl].size(); i++) {
				unsigned a = mAllTrns[curlvl][i];
				if(rlblmap.count(a) == 0)
					rlblmap[a] = c++;
				mAllTrns[curlvl][i] = rlblmap.at(a);
			}
			
			return c;
		}
		
		bool gen_edges(unsigned curlvl, unsigned nstates) {
			fast_assert(mLevels.size() == curlvl);
			fast_assert(mAllTrns.size() == curlvl);
			
			mLevels.emplace_back();
			mLevels.at(curlvl).resize(nstates);
			
			bool isTrivial = true;
			
			for(unsigned bsrc=0; bsrc<mLevels.at(curlvl-1).size(); bsrc++) {
				
				for(auto const& be : mLevels[curlvl-1][bsrc]) {					
					unsigned bdst = be.first;
					unsigned asrc = mAllTrns[curlvl-1][bsrc];
					unsigned adst = mAllTrns[curlvl-1][bdst];
					
					if(asrc == adst)
						continue;
					
					isTrivial = false;
					
					bool existing = false;
					
					for(auto& e : mLevels[curlvl][asrc]) {
						if(e.first == adst) {
							existing = true;
							
							if(e.second > be.second)
								e.second = be.second;
							break;
						}
					}
					
					if(!existing)
						mLevels[curlvl][asrc].push_back({adst, be.second});
				}
			}
			
			return isTrivial;
		}
		
		//mLevels[i] has edges for level i.
		//mAllTrns[i] maps level i states to level i+1.
		
		StarAbtInfo(CellMap2D<> const& pCellMap, unsigned pMaxDepth) :
			mMaxDepth(pMaxDepth)
		{
			mLevels.clear();
			mAllTrns.clear();
			
			bool isTrivial = gen_base(pCellMap);

			unsigned curlvl = 0;
			
			while(!isTrivial) {
				unsigned nstates = gen_trns(curlvl);
				isTrivial = gen_edges(curlvl+1, nstates);
				curlvl++;
			}
		}
		
		
		void draw(CellMap2D<> const& pCellMap, std::ostream& out) const {
			
			std::vector<unsigned> s(pCellMap.cells().size(), (unsigned)-1);
			
			for(unsigned i=0; i<pCellMap.cells().size(); i++) {
				if(pCellMap.cells()[i] == CellMap2D<>::Cell_t::Open)
					s[i] = mBaseTrns[i];
			}
			
			unsigned curlvl = 0;
			
			while(true) {
				std::cout << curlvl << ":\n";
			
				for(unsigned i=0; i<pCellMap.getHeight(); i++) {
					for(unsigned j=0; j<pCellMap.getWidth(); j++) {
						if(s[i*pCellMap.getWidth()+j] == (unsigned)-1)
							std::cout << " ";
						else
							std::cout << s[i*pCellMap.getWidth()+j] % 10;
					}
					std::cout << "\n";
				}
				
				std::cout << "\n";
				
				for(unsigned i=0; i<mLevels[curlvl].size(); i++) {
					std::cout << i << ": ";
					for(auto const& e : mLevels[curlvl][i]) {
						std::cout << "(" << e.first << "," << e.second << ") ";
					}
					std::cout << "\n";
				}
				
				std::cout << "\n\n";
			
				if(curlvl == mLevels.size()-1)
					break;

				
				for(unsigned i=0; i<pCellMap.cells().size(); i++) {
					if(pCellMap.cells()[i] == CellMap2D<>::Cell_t::Open)
						s[i] = mAllTrns[curlvl][s[i]];
				}
			
				curlvl++;
			}
		}
		
		
		std::vector<LevelEntry_t> mLevels;
		std::vector<Trns_t> mAllTrns;
		Trns_t mBaseTrns;
		const unsigned mMaxDepth;
			
	};


}}}
