#pragma once

#include <array>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

#include "domain/gridnav/defs.hpp"
#include "domain/gridnav/flat_domain.hpp"

#include "util/json.hpp"
#include "util/exception.hpp"




namespace mjon661 { namespace gridnav { namespace blocked {


	template<unsigned Height, unsigned Width, template<unsigned, unsigned> typename DB>
	struct GridNav_Stack_single {
		
		using selfStack_t = GridNav_Stack_single<Height, Width, DB>;
		
		
		template<unsigned L>
		struct Domain : GridNav_Dom<Height, Width, DB> {
			
			static_assert(L == 0, "");
			
			Domain(selfStack_t& pStack) :
				GridNav_Dom<Height, Width, DB>(pStack.mMap, pStack.mInitPos, pStack.mGoalPos)
			{}
		};
		
		
		
		GridNav_Stack_single(Json const& jConfig) :
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
				unsigned Abt_Lvls, 
				unsigned Merge_Height_Factor,
				unsigned Merge_Width_Factor,
				unsigned Merge_Fill_Factor,
				bool Use_EightWay, 
				bool Use_LifeCost>
	struct GridNav_Stack {
		
		using selfStack_t = GridNav_Stack<Height, Width, Abt_Lvls, Use, EightWay, Use_LifeCost>;
		
		template<unsigned H, unsigned W>
		using DomBase = typename GridNavBase<Use_EightWay, Use_LifeCost, false>::type;
		
		
		static const unsigned Top_Abstract_Level = Abt_Lvls;
		static_assert(heightAtAbtLevel(Top_Abstract_Level, 0, Height) != 0, "");
		static_assert(widthAtAbtLevel(Top_Abstract_Level, 0, width) != 0, "");
		
		
		static constexpr heightAtAbtLevel(unsigned L, unsigned n, unsigned s) {
			return L == n ? s : heightAtAbtLevel(L, n+1, s/Merge_Height_Factor);
		}
		
		static constexpr widthAtAbtLevel(unsigned L, unsigned n, unsigned s) {
			return L == n ? s : heightAtAbtLevel(L, n+1, s/Merge_Width_Factor);
		}
		
		template<unsigned L>
		struct Domain : GridNav_Dom<heightAtAbtLevel(L, 0, Height), widthAtAbtLevel(L, 0, Width), DomBase> {

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
				//Heuristics aren't used with abstraction, so don't need to set abtState.h and .d .
				
				return abtSt;
			}
		};
		
		
		
		GridNav_Stack(Json const& jConfig) :
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
