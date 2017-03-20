#pragma once

#include <array>
#include <vector>
#include <random>
#include <algorithm>

#include "util/json.hpp"
#include "util/math.hpp"
#include "util/debug.hpp"
#include "util/exception.hpp"

#include "domain/tiles/defs.hpp"
#include "domain/tiles/domain.hpp"
#include "domain/tiles/board_state.hpp"


namespace mjon661 { namespace tiles { 
	

	template<unsigned Height, unsigned Width, bool Use_H, bool Use_Weight, unsigned Abt1Sz>
	struct TilesGeneric_DomainStack {
		
		static_assert(Abt1Sz < Height*Width, "");
		static_assert(Height > 1 && Width > 1, "");
		
		using domStack_t = TilesGeneric_DomainStack<Height, Width, Use_H, Use_Weight, Abt1Sz>;

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
		struct Domain<0, Ign> : public CompleteTilesDomain<Height, Width, Use_H, Use_Weight> {
			
			Domain(domStack_t const& pStack) :
				CompleteTilesDomain<Height, Width, Use_H, Use_Weight>(pStack.mGoalState)
			{}
		};
		
		
		typename Domain<0>::State getInitState() const {
			typename Domain<0>::State s = mInitState;
			Domain<0> dom(*this);
			dom.initialiseState(s);
			return s;
		}
		
		void setInitState(BoardState<Height,Width> const& pState) {
			mInitState = pState;
		}
		
		BoardState<Height, Width> randInitState(unsigned pN) const {
			BoardState<Height, Width> s;
			for(unsigned i=0; i<Height*Width; i++)
				s[i] = i;
			
			std::mt19937 gen;
			
			for(unsigned i=0; i<(pN+3)*17; i++) {
				do {
					std::shuffle(s.begin(), s.end(), gen);
				} while(!isSolvable(s, mGoalState));
			}
			
			s.initBlankPos();
			return s;
		}
		
		TilesGeneric_DomainStack(Json const& jConfig) :
			mAbtSpec(jConfig.at("kept").get<std::vector<unsigned>>()),
			mGoalState(jConfig.at("goal").get<std::vector<tile_t>>())
		{
			if(jConfig.count("init"))
				mInitState = BoardState<Height, Width>(jConfig.at("goal").get<std::vector<tile_t>>());
			else
				mInitState = randInitState(0);
			
			if(!mInitState.valid())
				throw ConfigException("Bad init");
			
			if(!mGoalState.valid())
				throw ConfigException("Bad goal");
		}
		
		TilesAbtSpec<Height*Width> mAbtSpec;
		BoardState<Height, Width> mInitState, mGoalState;
	};

}}
