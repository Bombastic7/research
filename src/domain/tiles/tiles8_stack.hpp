#pragma once

#include <array>

#include "util/json.hpp"

#include "domain/tiles/defs.hpp"
#include "domain/tiles/domain.hpp"


namespace mjon661 { namespace tiles { 
	

	template<unsigned Height, unsigned Width, bool Use_Weight, bool Use_H, unsigned Abt1Sz>
	struct TilesGeneric_DomainStack {
		
		static_assert(Abt1Sz < Height*Width, "");
		static_assert(Height > 1 && Width > 1, "");
		
		using domStack_t = Tiles8_DomainStack_single<Use_Weight,Use_H>;
		
		
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
			
			static const Sz = tilesPerLevel(L);
			
			
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

			Abstractor(this_t& pStack) :
				mAbt1Map(pStack.getIndexMap<1>())
			{}

			BoardStateP<Height, Width, tilesPerLevel(1)> operator()(BoardStateV<Height, Width> const& pState) {				
				
				BoardStateP<Height, Width, tilesPerLevel(1)> abtState(pState, mIM1);

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
				
				if(!contains(v, 0) || !withinInclusive(v, 0, Height*Width-1) || !unique(v))
					throw ConfigException("Bad kept tiles");
				
				for(unsigned i=0; i<Abt1Sz; i++)
					mAbtKept[i] = v[i];
			}
			else
				for(unsigned i=0; i<Abt1Sz; i++)
					mAbtKept[i] = i;
		}
		
		std::array<tile_t, Abt1Sz> mAbt1Kept;
		BoardStateV<Height*Width> mInitState, mGoalState;
		
	};
		
	
		
		template<unsigned L, typename = void>
		struct Abstractor;
		
		template<typename Ign>
		struct Abstractor<0, Ign> {
			
			Abstractor(this_t& pStack) :
				mIM1(pStack.mIM1)
			{}
			
			
			BoardStateP<Height, Width, 5> operator()(BoardStateV<Height, Width> const& pState) {				
				BoardStateP<Height, Width, 5> abtState(pState, mIM1);

				return abtState;
			}
			
			
			IndexMap<Height*Width, 5> const& mIM1;
			
		};
		
		
		template<typename Ign>
		struct Abstractor<1, Ign> {
			Abstractor(this_t& pStack) : mTrns(pStack.mIM2.translateIndices(pStack.mIM1)) {
			
			}
			
			BoardStateP<Height, Width, 4> operator()(BoardStateP<Height, Width, 5> const& pState) {
				BoardStateP<Height, Width, 4> abtState;
				
				for(unsigned i=0; i<abtState.size(); i++)
					abtState[i] = pState[mTrns[i]];
					
				return abtState;
			}
			
			std::array<idx_t, 4> mTrns;
		};
		
		template<typename Ign>
		struct Abstractor<2, Ign> {
			Abstractor(this_t& pStack) : mTrns(pStack.mIM3.translateIndices(pStack.mIM2)) {}
			
			BoardStateP<Height, Width, 3> operator()(BoardStateP<Height, Width, 4> const& pState) {
				BoardStateP<Height, Width, 3> abtState;
				
				for(unsigned i=0; i<abtState.size(); i++)
					abtState[i] = pState[mTrns[i]];
					
				return abtState;
			}
			
			std::array<idx_t, 3> mTrns;
		};
		
		template<typename Ign>
		struct Abstractor<3, Ign> {
			Abstractor(this_t& pStack) : mTrns(pStack.mIM4.translateIndices(pStack.mIM3)) {}
			
			BoardStateP<Height, Width, 2> operator()(BoardStateP<Height, Width, 3> const& pState) {
				BoardStateP<Height, Width, 2> abtState;
				
				for(unsigned i=0; i<abtState.size(); i++)
					abtState[i] = pState[mTrns[i]];
					
				return abtState;
			}
			
			std::array<idx_t, 2> mTrns;
		};
		
		template<typename Ign>
		struct Abstractor<4, Ign> {
			Abstractor(this_t& pStack) : mTrns(pStack.mIM5.translateIndices(pStack.mIM4)) {}
			
			BoardStateP<Height, Width, 1> operator()(BoardStateP<Height, Width, 2> const& pState) {
				BoardStateP<Height, Width, 1> abtState;
				
				for(unsigned i=0; i<abtState.size(); i++)
					abtState[i] = pState[mTrns[i]];
					
				return abtState;
			}
			
			std::array<idx_t, 1> mTrns;
		};
		
		
		TilesEightStack(Json const& jConfig) :
			mInitState(prepState(jConfig, "init")),
			mGoalState(prepState(jConfig, "goal")),
			mDomBase0(mInitState, mGoalState),
			mAbtSpec{0,1,2,3,4},
			mIM1(mAbtSpec.begin(), mAbtSpec.end()),
			mIM2(mAbtSpec.begin(), mAbtSpec.end()-1),
			mIM3(mAbtSpec.begin(), mAbtSpec.end()-2),
			mIM4(mAbtSpec.begin(), mAbtSpec.end()-3),
			mIM5(mAbtSpec.begin(), mAbtSpec.end()-4)
		{}
		
		
		
		BoardStateV<3,3> prepState(Json const& jConfig, std::string const& pKey) {
			
			std::vector<tile_t> vec = jConfig.at(pKey);
			
			BoardStateV<3,3> s(vec);
			
			return s;
		}
		
		
		
		const BoardStateV<3,3> mInitState, mGoalState;
		
		DomBase0 mDomBase0;
		
		std::array<tile_t, 5> mAbtSpec;
		
		const IndexMap<Height*Width, 5> mIM1;
		const IndexMap<Height*Width, 4> mIM2;
		const IndexMap<Height*Width, 3> mIM3;
		const IndexMap<Height*Width, 2> mIM4;
		const IndexMap<Height*Width, 1> mIM5;
	};
	
	
}}
