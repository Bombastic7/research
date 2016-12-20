#pragma once

#include <array>
#include <vector>

#include "util/json.hpp"
#include "util/math.hpp"
#include "util/debug.hpp"
#include "util/exception.hpp"

#include "domain/tiles/defs.hpp"
#include "domain/tiles/domain.hpp"
#include "domain/tiles/index_map.hpp"
#include "domain/tiles/board_state.hpp"


namespace mjon661 { namespace tiles { 
	

	template<unsigned Height, unsigned Width, bool Use_Weight, bool Use_H, unsigned Abt1Sz>
	struct TilesGeneric_DomainStack {
		
		static_assert(Abt1Sz < Height*Width, "");
		static_assert(Height > 1 && Width > 1, "");
		
		using domStack_t = TilesGeneric_DomainStack<Height, Width, Use_Weight, Use_H, Abt1Sz>;
		
		
		static const unsigned Top_Abstract_Level = Abt1Sz;
		
		
		static constexpr unsigned tilesPerLevel(unsigned L) {
			return L == 0 ? Height*Width : Abt1Sz-L+1;
		}
		
		
		template<unsigned L>
		struct Domain : TilesDomain<Height, Width, tilesPerLevel(L), Use_Weight, Use_H> {
			
			Domain(domStack_t& pStack) :
				TilesDomain<Height, Width, tilesPerLevel(L), Use_Weight, Use_H>(
						pStack.mInitState, pStack.mGoalState, pStack.getIndexMap<L>())
				{}
		};
		
		
		template<unsigned L, typename = void>
		struct Abstractor {
			
			static const unsigned Sz = tilesPerLevel(L);
			
			
			Abstractor(domStack_t& pStack) :
				mTrns(pStack.getIndexMap<L+1>().translateIndices(pStack.getIndexMap<L>()))
			{}
			
			
			BoardStateP<Height, Width, Sz-1> operator()(BoardStateP<Height, Width, Sz> const& pState) {
				BoardStateP<Height, Width, Sz> abtState;
				
				for(unsigned i=0; i<abtState.size(); i++)
					abtState[i] = pState[mTrns[i]];
					
				return abtState;
			}
			
			std::array<idx_t, Sz> mTrns;
			
			
		};
		
		
		template<typename Ign>
		struct Abstractor<0, Ign> {

			Abstractor(domStack_t& pStack) :
				mAbt1Map(pStack.getIndexMap<1>())
			{}

			BoardStateP<Height, Width, tilesPerLevel(1)> operator()(BoardStateV<Height, Width> const& pState) {				
				
				BoardStateP<Height, Width, tilesPerLevel(1)> abtState(pState, mAbt1Map);

				return abtState;
			}
			
			IndexMap<Height*Width, tilesPerLevel(1)> mAbt1Map;
		};
		
		
		
		
		template<unsigned L>
		IndexMap<Height*Width, tilesPerLevel(L)> getIndexMap() {
			
			std::array<tile_t, Height*Width> tmp{};
			
			if(L > 0)
				for(unsigned i=0; i<tilesPerLevel(L); i++)
					tmp[i] = mAbt1Kept[i];
			
			return IndexMap<Height*Width, tilesPerLevel(L)>(tmp.begin(), tmp.begin() + tilesPerLevel(L));
		}
		
		
		
		TilesGeneric_DomainStack(Json const& jConfig) :
			mAbt1Kept{},
			mInitState(jConfig.at("init")),
			mGoalState(jConfig.at("goal"))
		{
			if(!mInitState.valid())
				throw ConfigException("Bad init");
			
			if(!mGoalState.valid())
				throw ConfigException("Bad goal");
				
			if(jConfig.count("kept")) {
				std::vector<tile_t> v = jConfig.at("kept");
				
				if(!contains(v, 0) || !withinInclusive(v, 0u, Height*Width-1) || !uniqueElements(v))
					throw ConfigException("Bad kept tiles");
				
				for(unsigned i=0; i<Abt1Sz; i++)
					mAbt1Kept[i] = v[i];
			}
			else
				for(unsigned i=0; i<Abt1Sz; i++)
					mAbt1Kept[i] = i;
		}
		
		std::array<tile_t, Abt1Sz> mAbt1Kept;
		BoardStateV<Height, Width> mInitState, mGoalState;
		
	};

}}