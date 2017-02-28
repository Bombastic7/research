#pragma once

#include <array>
#include <vector>

#include "util/json.hpp"
#include "util/math.hpp"
#include "util/debug.hpp"
#include "util/exception.hpp"

#include "domain/tiles/defs.hpp"
#include "domain/tiles/domain.hpp"
#include "domain/tiles/board_state.hpp"


namespace mjon661 { namespace tiles { 
	

	template<unsigned Height, unsigned Width, bool Use_Weight, bool Use_H, unsigned Abt1Sz>
	struct TilesGeneric_DomainStack {
		
		static_assert(Abt1Sz < Height*Width, "");
		static_assert(Height > 1 && Width > 1, "");
		
		using domStack_t = TilesGeneric_DomainStack<Height, Width, Use_Weight, Use_H, Abt1Sz>;

		static const unsigned Top_Abstract_Level = Abt1Sz;
		
		
		static constexpr unsigned tilesAtLevel(unsigned L) {
			return L == 0 ? Height*Width : Abt1Sz-L+1;
		}
		
		template<unsigned L, typename = void>
		struct Domain : public SubsetTilesDomain<Height, Width, tilesAtLevel(L), Use_Weight> {
			Domain(domStack_t const& pStack) :
				SubsetTilesDomain<Height, Width, tilesAtLevel(L), Use_Weight>(pStack.mAbtSpec, pStack.mGoalState)
			{}
		};
		
		template<typename Ign>
		struct Domain<0, Ign> : public CompleteTilesDomain<Height, Width, Use_Weight, Use_H> {
			
			Domain(domStack_t const& pStack) :
				CompleteTilesDomain<Height, Width, Use_Weight, Use_H>(pStack.mGoalState)
			{}
		};
		
		
		BoardState<Height, Width> getInitState() const {
			return mInitState;
		}
		
		TilesGeneric_DomainStack(Json const& jConfig) :
			mAbtSpec(jConfig.at("kept").get<std::vector<unsigned>>()),
			mInitState(jConfig.at("init").get<std::vector<tile_t>>()),
			mGoalState(jConfig.at("goal").get<std::vector<tile_t>>())
		{
			if(!mInitState.valid())
				throw ConfigException("Bad init");
			
			if(!mGoalState.valid())
				throw ConfigException("Bad goal");
		}
		
		TilesAbtSpec<Height*Width> mAbtSpec;
		BoardState<Height, Width> mInitState, mGoalState;
	};

}}
