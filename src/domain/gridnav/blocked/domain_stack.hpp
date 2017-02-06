#pragma once

#include <array>
#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>

#include "util/json.hpp"
#include "util/exception.hpp"
#include "util/math.hpp"

#include "domain/gridnav/blocked/graph.hpp"
#include "domain/gridnav/blocked/domain.hpp"



namespace mjon661 { namespace gridnav { namespace blocked {


	template<unsigned Height, unsigned Width, typename BaseFuncs, bool Use_LifeCost, bool Use_H>
	struct GridNav_DomainStack_StarAbt_ {
		
		using DomStack_t = GridNav_DomainStack_StarAbt_<Height, Width, BaseFuncs, Use_LifeCost, Use_H>;
		using BaseMap_t = CellMap<BaseFuncs, Use_LifeCost>;
		using AbtMaps_t = StarAbtCellMap<BaseFuncs, Use_LifeCost>;
		
		static const unsigned Top_Abstract_Level = 9; //Is a guess.
		
		static constexpr size_t estimateHashRange(unsigned L) {
			return L == 0 ? Height*Width : 1000 + Height*Width / mathutil::pow(4u, L);
		}
		
		
		template<unsigned L, typename = void>
		struct Domain : public GridNav_AbtDom<AbtMaps_t, estimateHashRange(L)> {
			Domain(DomStack_t& pStack) :
				GridNav_AbtDom<AbtMaps_t, estimateHashRange(L)>(
					pStack.mAbtMaps.getGroupEdges(L), 
					pStack.mAbtMaps.abstractBaseCell(pStack.mGoalState, L))
			{}
		};
		
		
		template<typename Ign>
		struct Domain<0, Ign> : public GridNav_BaseDom<BaseMap_t, Use_H, estimateHashRange(0)> {
			Domain(DomStack_t& pStack) :
				GridNav_BaseDom<BaseMap_t, Use_H, estimateHashRange(0)>(
					pStack.mBaseMap,
					pStack.mInitState,
					pStack.mGoalState)
			{}
		};
		
		template<unsigned L>
		struct Abstractor {
			
			Abstractor(DomStack_t& pStack) :
				mStack(pStack)
			{
				//Ensure no further abstraction is possible.
				if(L == pStack.lastUsedAbstractLevel()) {
					for(auto v : mStack.mAbtMaps.getGroupEdges(L))
						gen_assert(v.size() == 0);
				}
			}
			
			unsigned operator()(unsigned pBaseState) {
				if(L == 0)
					return mStack.mAbtMaps.abstractBaseCell(pBaseState, 1);
				
				return mStack.mAbtMaps.getAbstractGroup(pBaseState, L);
			}
			
			private:
			DomStack_t& mStack;
		};
		
		
		
		GridNav_DomainStack_StarAbt_(Json const& jConfig) :
			mBaseMap(Height, Width, jConfig.at("map")),
			mAbtMaps(mBaseMap, jConfig.at("radius"))
		{
			mNabtLvlsUsed = mAbtMaps.getLevelSizes().size() - 1;
			
			if(jConfig.at("init").is_array())
				mInitState = jConfig.at("init")[0].get<unsigned>() + jConfig.at("init")[1].get<unsigned>() * Width;
			else
				mInitState = jConfig.at("init");
			
			if(jConfig.at("goal").is_array())
				mGoalState = jConfig.at("goal")[0].get<unsigned>() + jConfig.at("goal")[1].get<unsigned>() * Width;
			else
				mGoalState = jConfig.at("goal");
			
			//~ std::ofstream ofs("gridnav_test_out"); //...........
			//~ gen_assert(ofs);
			//~ dump(ofs);
		}
		
		unsigned lastUsedAbstractLevel() const {
			return mNabtLvlsUsed;
		}
		
		void dump(std::ostream& out) const {
			for(unsigned i : mAbtMaps.getLevelSizes()) 
				out << i << " ";
			
			out << "\n" << lastUsedAbstractLevel();
			out << "\n" << mAbtMaps.checkBaseConnected(mInitState, mGoalState) << " " << mInitState << " " << mGoalState;
			out << "\n" << mAbtMaps.abstractBaseCell(mInitState, lastUsedAbstractLevel());
			out << "\n" << mAbtMaps.abstractBaseCell(mGoalState, lastUsedAbstractLevel());
			out << "\n\n";
			mAbtMaps.dumpall(out);
		}
		
		
		private:
		unsigned mNabtLvlsUsed;
		CellMap<BaseFuncs, Use_LifeCost> mBaseMap;
		StarAbtCellMap<BaseFuncs, Use_LifeCost> mAbtMaps;
		unsigned mInitState, mGoalState;
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
