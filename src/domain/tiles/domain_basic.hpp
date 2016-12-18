#pragma once

#include <algorithm>
#include <string>
#include <type_traits>
#include <vector>

#include "domain/tiles/defs.hpp"
#include "domain/tiles/tiles64_basic.hpp"
#include "util/exception.hpp"
#include "util/json.hpp"


namespace mjon661 { namespace tiles {

	template<unsigned H, unsigned W, bool Use_Lehmer, bool Use_MD, bool Weighted_Cost>
	struct TilesDomain {
		
		using this_t = TilesDomain<H, W, Use_Lehmer, Use_MD, Weighted_Cost>;
		

		template<unsigned L, typename = void>
		struct Domain;
		
		template<typename Ign>
		struct Domain<0, Ign> : public Tiles64_basic<H, W, Use_Lehmer, Use_MD, Weighted_Cost> {
			struct Methods : public Tiles64_basic<H,W,Use_Lehmer, Use_MD, Weighted_Cost>::Methods {
				Methods(this_t& d) :
					Tiles64_basic<H,W,Use_Lehmer,Use_MD,Weighted_Cost>::Methods(d.mInitBoard, d.mGoalBoard)
				{}
			};
		};


		
		TilesDomain(Json const& j) : 
			mInitBoard(j.at("init").get<std::vector<tile_t>>()),
			mGoalBoard(j.at("goal").get<std::vector<tile_t>>())
		{}

		
		BoardValues<H*W> mInitBoard, mGoalBoard;
	};
	
	
	
	template<unsigned H, unsigned W, unsigned AbtDropped, bool Use_Lehmer, bool Use_MD, bool Weighted_Cost>
	struct TilesAbtDomain_DropAndTaper : public TilesDomain<H, W, Use_Lehmer, Use_MD, Weighted_Cost> {
		
		using this_t = TilesAbtDomain_DropAndTaper<H, W, AbtDropped, Use_Lehmer, Use_MD, Weighted_Cost>;
		using base_t = TilesDomain<H, W, Use_Lehmer, Use_MD, Weighted_Cost>;
		
		static const unsigned Top_Abstract_Level = H*W - AbtDropped;
		
		template<unsigned L>
		using AbtDomImpl = Tiles64Relaxed_basic<H,W,H*W - AbtDropped - L + 1, Weighted_Cost>;
		
		template<unsigned L, typename = void>
		struct Domain : public AbtDomImpl<L> {
			struct Methods : public AbtDomImpl<L>::Methods {
				Methods(this_t& pDom) :
					AbtDomImpl<L>::Methods(pDom.mInitBoard, pDom.mGoalBoard, pDom.mCheckedTiles[L])
				{}
			};
		};
		
		template<typename Ign>
		struct Domain<0, Ign> : public base_t::template Domain<0> {};
		
		
		
		
		template<unsigned L, typename = void>
		struct Abstractor : PositionTruncateAbstractor<H*W, H*W - AbtDropped - L + 1> {
			
			static const unsigned Tracked_Tiles = H*W - AbtDropped - L + 1;
			Abstractor(this_t& pDom) :
				PositionTruncateAbstractor<H*W, Tracked_Tiles>(pDom.mCheckedTiles[L], pDom.mCheckedTiles[L+1])
			{}
		};
		
		template<typename Ign>
		struct Abstractor<0, Ign> : public ValueToPositionAbstractor<H*W, H*W-AbtDropped> {
			Abstractor(this_t& pDom) :
				ValueToPositionAbstractor<H*W, H*W-AbtDropped>(pDom.mCheckedTiles[1])
			{}
		};
		
		
		
		TilesAbtDomain_DropAndTaper(Json const& j) : 
			base_t(j)
		{
			{
				std::vector<tile_t> v;
				for(unsigned i=0; i<H*W; i++)
					v.push_back(i);
				mCheckedTiles.push_back(v);
			}
			
			if(j.count("checked tiles")) {
				for(unsigned i = 1; i <= Top_Abstract_Level; i++) {
					mCheckedTiles.push_back(j["checked tiles"][std::to_string(i)]);
					
					if(*std::min_element(mCheckedTiles[i].begin(), mCheckedTiles[i].end()) != 0)
						throw ConfigException("");
					
					if((unsigned)*std::max_element(mCheckedTiles[i].begin(), mCheckedTiles[i].end()) >= H*W)
						throw ConfigException("");
					
					if(i != 0 && !subset(mCheckedTiles[i], mCheckedTiles[i-1], true))
						throw ConfigException("");
				}
			} else {
				
				for(unsigned pLvl=1; pLvl<=Top_Abstract_Level; pLvl++) {
					std::vector<tile_t> v;
					for(unsigned i=0; i<H*W - AbtDropped - pLvl + 1; i++)
						v.push_back(i);
					mCheckedTiles.push_back(v);
				}
					
			}
		}
		
		
		std::vector<std::vector<tile_t>> mCheckedTiles;
	};
}}
