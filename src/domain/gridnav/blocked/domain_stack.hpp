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

}}}
