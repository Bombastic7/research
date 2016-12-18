#pragma once

#include <array>

#include "util/json.hpp"

#include "domain/tiles/defs.hpp"
#include "domain/tiles/domain.hpp"


namespace mjon661 { namespace tiles { 
	
	namespace eight_impl {

	

	}
	
	
	
	template<bool Use_Weight, bool Use_H>
	struct TilesEightStack {
		
		using this_t = TilesEightStack<Use_Weight,Use_H>;
		using DomBase0 = CompleteTilesBase<3,3,Use_Weight,Use_H>;
		
		template<unsigned Sz>
		using DomBaseAbt = SubsetTilesBase<3, 3, Sz, Use_Weight>;
		
		static const unsigned Height = 3, Width = 3;
		
		
		static const unsigned Top_Abstract_Level = 5;
		
		
		template<unsigned L, typename = void>
		struct Domain;
		
		
		
		template<typename Ign>
		struct Domain<0, Ign> : public TilesDomain<DomBase0> {
			
			using base_t = TilesDomain<DomBase0>;
			
			Domain(this_t& pStack) :
				base_t(pStack.mDomBase0)
			{}
		};
		
		
		template<typename Ign>
		struct Domain<1, Ign> : public TilesDomain<DomBaseAbt<5>> {
			Domain(this_t& pStack) :
				TilesDomain<DomBaseAbt<5>>(DomBaseAbt<5>(pStack.mGoalState, pStack.mIM1))
			{}
		};
		
		template<typename Ign>
		struct Domain<2, Ign> : public TilesDomain<DomBaseAbt<4>> {
			Domain(this_t& pStack) :
				TilesDomain<DomBaseAbt<4>>(DomBaseAbt<4>(pStack.mGoalState, pStack.mIM2))
			{}
		};
		
		template<typename Ign>
		struct Domain<3, Ign> : public TilesDomain<DomBaseAbt<3>> {
			Domain(this_t& pStack) :
				TilesDomain<DomBaseAbt<3>>(DomBaseAbt<3>(pStack.mGoalState, pStack.mIM3))
			{}
		};
		
		template<typename Ign>
		struct Domain<4, Ign> : public TilesDomain<DomBaseAbt<2>> {
			Domain(this_t& pStack) :
				TilesDomain<DomBaseAbt<2>>(DomBaseAbt<2>(pStack.mGoalState, pStack.mIM4))
			{}
		};
		
		template<typename Ign>
		struct Domain<5, Ign> : public TilesDomain<DomBaseAbt<1>> {
			Domain(this_t& pStack) :
				TilesDomain<DomBaseAbt<1>>(DomBaseAbt<1>(pStack.mGoalState, pStack.mIM5))
			{}
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
