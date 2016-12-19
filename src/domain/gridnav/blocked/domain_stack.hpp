#pragma once

#include <array>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

#include "domain/gridnav/defs.hpp"
#include "domain/gridnav/blocked/domain.hpp"
#include "domain/gridnav/blocked/maps.hpp"

#include "util/json.hpp"
#include "util/exception.hpp"




namespace mjon661 { namespace gridnav { namespace blocked {


	template<unsigned Height, unsigned Width, template<unsigned, unsigned> typename DB>
	struct GridNav_DomainStack_single {
		
		using selfStack_t = GridNav_Stack_single<Height, Width, DB>;
		
		
		template<unsigned L>
		struct Domain : GridNav_Dom<Height, Width, DB> {
			
			static_assert(L == 0, "");
			
			Domain(selfStack_t& pStack) :
				GridNav_Dom<Height, Width, DB>(pStack.mMap, pStack.mInitPos, pStack.mGoalPos)
			{}
		};
		
		
		
		GridNav_DomainStack_single(Json const& jConfig) :
			mInitPos(readCoord(jConfig, "init")),
			mGoalPos(readCoord(jConfig, "goal")),
			mMap(Height, Width)
		{
			std::ifstream ifs(jConfig.at("map"));
			
			if(!ifs)
				throw ConfigException("Could not open map file");
			
			mMap->read(ifs);
		}
		
		
		idx_t readCoord(Json const& jConfig, std::string const& key) {
			idx_t ret;
			
			if(jConfig.at(key).is_number())
				ret = jConfig.at(key).get<idx_t>();
			
			else if(jConfig.at(key).is_array()) {
				idx_t x = jConfig.at(key).at(0);
				idx_t y = jConfig.at(key).at(1);
				
				if(x < 0 || x >= Width || y < 0 || y >= Height)
					throw ConfigException("Coords out of range");
				
				ret = y * Width + x;
			} else
				throw ConfigException("Coords have bad type");
			
			if(ret < 0 || ret >= Height*Width)
				throw ConfigException("Coords out of range");
			
			return ret;
		}
		
		const idx_t mInitPos, mGoalPos;
		GridNav_Map mMap;
	};





	template<	unsigned Height,
				unsigned Width, 
				bool Use_EightWay, 
				bool Use_LifeCost,
				unsigned Max_Abt_Lvls, 
				unsigned Merge_Height_Factor,
				unsigned Merge_Width_Factor,
				unsigned Merge_Fill_Factor
			>

	struct GridNav_DomainStack_MergeAbt {
		
		using selfStack_t = GridNav_DomainStack_MergeAbt<Height, Width, Max_Abt_Lvls, Use_EightWay, Use_LifeCost>;
		
		template<unsigned H, unsigned W>
		using DomBase = typename GridNavBase<Use_EightWay, Use_LifeCost, false>::type;
		
		
		
		static const unsigned Top_Abstract_Level = min(Max_Abt_Lvls, maxPossibleAbtLevel());

		
		static constexpr unsigned heightAtAbtLevel(unsigned L, unsigned n = 0, unsigned s = Height) {
			return L == n ? s : heightAtAbtLevel(L, n+1, s/Merge_Height_Factor);
		}
		
		static constexpr unsigned widthAtAbtLevel(unsigned L, unsigned n = 0, unsigned s = Width) {
			return L == n ? s : widthAtAbtLevel(L, n+1, s/Merge_Width_Factor);
		}
		
		static constexpr unsigned maxPossibleAbtLevel(unsigned tryLvl = 0) {
			return heightAtAbtLevel(tryLvl+1, 0, Height) == 0 || widthAtAbtLevel(tryLvl+1, 0, Width) == 0 ?
					tryLvl :
					maxPossibleAbtLevel(tryLvl+1);
		}
		
		template<unsigned L>
		struct Domain : GridNav_Dom<heightAtAbtLevel(L), widthAtAbtLevel(L), DomBase> {

			Domain(selfStack_t& pStack) :
				GridNav_Dom<Height, Width, DomBase>(pStack.mMapStack.getLevel(L), pStack.mInitPos, pStack.mGoalPos)
			{}
		};
		
		template<unsigned L>
		struct Abstractor {
			
			using SelfState = typename Domain<L>::State;
			using AbtState = typename Domain<L+1>::State;
			
			Abstractor(selfStack_t& pStack) {}
			
			AbtState operator()(SelfState const& pState) {
				AbtState abtSt();
				
				abtSt.pos = pState.pos;
				//Heuristics aren't used with abstraction, so don't need to deal with abtState.set_h() / set_d().
				
				return abtSt;
			}
		};
		
		
		
		GridNav_DomainStack_MergeAbt(Json const& jConfig) :
			mInitPos(readCoord(jConfig, "init")),
			mGoalPos(readCoord(jConfig, "goal")),
			mMapStack(Height, Width)
		{
			std::ifstream ifs(jConfig.at("map"));
			
			if(!ifs)
				throw ConfigException("Could not open map file");
			
			mMapStack->read(ifs);
		}
		
		
		idx_t readCoord(Json const& jConfig, std::string const& key) {
			idx_t ret;
			
			if(jConfig.at(key).is_number())
				ret = jConfig.at(key).get<idx_t>();
			
			else if(jConfig.at(key).is_array()) {
				idx_t x = jConfig.at(key).at(0);
				idx_t y = jConfig.at(key).at(1);
				
				if(x < 0 || x >= Width || y < 0 || y >= Height)
					throw ConfigException("Coords out of range");
				
				ret = y * Width + x;
			} else
				throw ConfigException("Coords have bad type");
			
			if(ret < 0 || ret >= Height*Width)
				throw ConfigException("Coords out of range");
			
			return ret;
		}
		
		const idx_t mInitPos, mGoalPos;
		GridNav_MapStack_MergeAbt mMapStack;		
		
	};


}}}
