#pragma once

#include <array>
#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>

#include "util/json.hpp"
#include "util/exception.hpp"
#include "util/math.hpp"

#include "domain/gridnav/graph.hpp"



namespace mjon661 { namespace gridnav { namespace blocked {


	template<unsigned Height, unsigned Width, typename BaseFuncs, bool Use_LifeCost, bool Use_H>
	struct GridNav_DomainStack_StarAbt_ {
		
		using DomStack_t = GridNav_DomainStack_StarAbt_<Height, Width, BaseFuncs, Use_LifeCost, Use_H>;
		using BaseMap_t = CellMap<BaseFuncs, Use_LifeCost>;
		using AbtMaps_t = StarAbtCellMap<BaseFuncs, Use_LifeCost>;
		
		
		
		static constexpr size_t estimateHashRange(unsigned L) {
			return L == 0 ? Height*Width : 1000 + Height*Width / mathutil::pow(4, L);
		}
		
		
		template<unsigned L, typename = void>
		struct Domain : public GridNav_AbtDom<AbtMaps_t, estimateHashRange(L)> {
			Domain(DomStack_t& pStack) :
				GridNav_AbtDom<AbtMaps_t, estimateHashRange(L)>(
					pStack.mAbtMaps.getGroupEdges(L), 
					pStack.mAbtMaps.abstractBaseCell(mGoalState, L))
			{}
		};
		
		
		template<typename Ign>
		struct Domain<0, Ign> : public GridNav_BaseDom<BaseMap_t, Use_H, estimateHashRange(0)> {
			Domain(DomStack_t& pStack) :
				GridNav_AbtDom<BaseMap_t, Use_H, estimateHashRange(0)>(
					pStack.mBaseMap.getAdjCellsList(),
					pStack.mInitState,
					pStack.mGoalState)
			{}
		};
		
		template<unsigned L>
		struct Abstractor {
			
			Abstractor(DomStack_t& pStack) :
				mStack(pStack)
			{}
			
			unsigned operator()(unsigned pBaseState) {
				return mStack.getAbstractGroup(pBaseState, L+1);
			}
			
			private:
			DomStack_t& mStack;
		};
		
		
		
		GridNav_DomainStack_StarAbt_(Json const& jConfig) :
			mBaseMap(Height, Width, jConfig.at("map")),
			mAbtMaps(mBaseMap),
			mInitState(jConfig.at("init")),
			mGoalState(jConfig.at("goal"))
		{
		}
		
		
		private:
		CellMap<BaseFuncs, Use_LifeCost> mBaseMap;
		StarAbtCellMap<BaseFuncs, Use_LifeCost> mAbtMaps;
		unsigned const mInitState, mGoalState;
	};



/*
	template<	unsigned Height,
				unsigned Width, 
				bool Use_EightWay, 
				bool Use_LifeCost,
				bool Use_H,
				unsigned Max_Abt_Lvls, 
				unsigned Merge_Height_Factor,
				unsigned Merge_Width_Factor,
				unsigned Merge_Fill_Factor
			>

	struct GridNav_DomainStack_MergeAbt {
		
		using selfStack_t = GridNav_DomainStack_MergeAbt<	Height, 
															Width,
															Use_EightWay, 
															Use_LifeCost,
															Use_H, 
															Max_Abt_Lvls, 
															Merge_Height_Factor, 
															Merge_Width_Factor, 
															Merge_Fill_Factor>;
		
		

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
		
		

		static constexpr unsigned compressX(unsigned i, unsigned L, unsigned n = 0) {
			return L == n ? i : compressX( mathutil::min(i/Merge_Width_Factor, widthAtAbtLevel(n+1)-1), L, n+1);
		}
		
		static constexpr unsigned compressY(unsigned i, unsigned L, unsigned n = 0) {
			return L == n ? i : compressY( mathutil::min(i/Merge_Height_Factor, heightAtAbtLevel(n+1)-1), L, n+1);
		}
		
		static constexpr unsigned basePosAtLevel(idx_t pos, unsigned L) {
			return compressX(pos%Width, L) + widthAtAbtLevel(L) * compressY(pos/Width, L);
		}
		
		static constexpr unsigned posUpOneLevel(idx_t pos, unsigned L) {
			return mathutil::min( (pos%widthAtAbtLevel(L)) / Merge_Width_Factor , widthAtAbtLevel(L+1)-1 ) +
					mathutil::min( (pos/widthAtAbtLevel(L)) / Merge_Height_Factor , heightAtAbtLevel(L+1)-1 ) * widthAtAbtLevel(L+1);
		}
		
		
		
		static const unsigned Top_Abstract_Level = mathutil::min(Max_Abt_Lvls, maxPossibleAbtLevel());
		
		
		
		template<unsigned L>
		using domain_base = GridNav_Dom<selfStack_t::heightAtAbtLevel(L), 
										selfStack_t::widthAtAbtLevel(L),
										Use_EightWay,
										Use_LifeCost,
										Use_H,
										(L > 0)>;
		
		
		
		template<unsigned L>
		struct Domain : public domain_base<L> {

			Domain(selfStack_t& pStack) :
				domain_base<L>(
					pStack.mMapStack.getLevel(L), 
					basePosAtLevel(pStack.mInitPos, L), 
					basePosAtLevel(pStack.mGoalPos, L),
					pStack.relaxedCostAtLevel(L),
					mEffectiveRow)
			{
				for(unsigned i=0; i<heightAtAbtLevel(L); i++)
					mEffectiveRow[i] = compressY(i, L);
			
			}
			
			std::array<unsigned, heightAtAbtLevel(L)> mEffectiveRow; //Abt row -> cheapest base row represented.
		};
		
		template<unsigned L>
		struct Abstractor {
			
			using SelfState = typename Domain<L>::State;
			using AbtState = typename Domain<L+1>::State;
			
			Abstractor(selfStack_t& pStack) {}
			
			AbtState operator()(SelfState const& pState) {
				AbtState abtSt;
				
				abtSt.pos = posUpOneLevel(pState.pos, L);
				
				return abtSt;
			}
		};
		
		
		
		GridNav_DomainStack_MergeAbt(Json const& jConfig) :
			mInitPos(readCoord(jConfig, "init")),
			mGoalPos(readCoord(jConfig, "goal")),
			mMapStack(Height, Width, Max_Abt_Lvls, Merge_Height_Factor, Merge_Width_Factor, Merge_Fill_Factor)
		{
			std::ifstream ifs(jConfig.at("map").get<std::string>());
			
			if(!ifs)
				throw ConfigException("Could not open map file");
			
			mMapStack.read(ifs);
			
			if(jConfig.count("dump_maps")) {
				std::string dumpStrPfx = jConfig.at("dump_maps");
				mMapStack.dumpMaps(dumpStrPfx);
			}
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
		
		
		unsigned relaxedCostAtLevel(unsigned L) {
			if(L == 0)
				return 2 * mathutil::max(Merge_Height_Factor, Merge_Width_Factor);

			return mathutil::pow(mathutil::min(Merge_Height_Factor, Merge_Width_Factor), L);
		}
		
		const idx_t mInitPos, mGoalPos;
		GridNav_MapStack_MergeAbt mMapStack;
		
		
		
	};
*/

}}}
