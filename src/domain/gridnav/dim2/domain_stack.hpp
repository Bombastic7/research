#pragma once

#include "util/exception.hpp"
#include "util/json.hpp"
#include "util/debug.hpp"

#include "search/astar.hpp"

#include "domain/gridnav/dim2/common.hpp"
#include "domain/gridnav/dim2/starabt.hpp"


namespace mjon661 { namespace gridnav { namespace dim2 {

	template<typename BaseDomain_t>
	struct DomainStack_BaseOnly {
		static const unsigned Top_Abstract_Level = 0;
		
		template<unsigned L>
		struct Domain : BaseDomain_t {
			Domain(DomainStack_BaseOnly<BaseDomain_t> const& pStack) :
				BaseDomain_t(pStack.mCellMap)
			{
				this->setGoalState(pStack.mGoalState);
			}
		};
		
	
		DomainStack_BaseOnly(CellMap2D<> const& pCellMap, Json const& jConfig) :
			mCellMap(pCellMap)
		{
			if(jConfig.count("init"))
				setInitAndGoal(jConfig.at("init"), jConfig.at("goal"));				
		}
		
		void setInitAndGoal(unsigned i, unsigned g) {
			mInitState = i;
			mGoalState = g;
			fast_assert(mInitState < mCellMap.size() && mGoalState < mCellMap.size());
			fast_assert(mCellMap.cells()[mInitState] == CellMap2D<>::Cell_t::Open);
			fast_assert(mCellMap.cells()[mGoalState] == CellMap2D<>::Cell_t::Open);
		}
		
		unsigned getInitState() const {
			return mInitState;
		}

		
		CellMap2D<> const& mCellMap;
		unsigned mInitState, mGoalState;
	};
	
	
	
	
	
	
	template<typename BaseDomain_t, unsigned Top_Abt>
	struct DomainStack_StarAbt {
		static const unsigned Top_Abstract_Level = Top_Abt;
		
		template<unsigned L, typename = void>
		struct Domain : Domain_StarAbt<L, typename BaseDomain_t::Cost> {
			Domain(DomainStack_StarAbt<BaseDomain_t, Top_Abt>& pStack) :
				Domain_StarAbt<L, typename BaseDomain_t::Cost>(pStack.mAbtInfo, pStack.mGoalState)
			{}
		};
		
		template<typename Ign>
		struct Domain<0,Ign> : BaseDomain_t {
			Domain(DomainStack_StarAbt<BaseDomain_t, Top_Abt> const& pStack) :
				BaseDomain_t(pStack.mCellMap)
			{
				this->setGoalState(pStack.mGoalState);
			}
		};
		
		

		DomainStack_StarAbt(CellMap2D<> const& pCellMap, StarAbtInfo<typename BaseDomain_t::Cost> const& pAbtInfo, Json const& jConfig) :
			mCellMap(pCellMap),
			mAbtInfo(pAbtInfo)
		{
			if(jConfig.count("init"))
				setInitAndGoal(jConfig.at("init"), jConfig.at("goal"));
			
			fast_assert(mAbtInfo.getNLevels() == Top_Abstract_Level + 1);
		}
		
		void setInitAndGoal(unsigned i, unsigned g) {
			mInitState = i;
			mGoalState = g;
			fast_assert(mInitState < mCellMap.size() && mGoalState < mCellMap.size());
			fast_assert(mCellMap.cells()[mInitState] == CellMap2D<>::Cell_t::Open);
			fast_assert(mCellMap.cells()[mGoalState] == CellMap2D<>::Cell_t::Open);
			
			unsigned abt_init = mAbtInfo.abstractBaseState(mInitState);
			unsigned abt_goal = mAbtInfo.abstractBaseState(mGoalState);
			
			abt_init = mAbtInfo.abstractState(abt_init, 0, Top_Abstract_Level);
			abt_goal = mAbtInfo.abstractState(abt_goal, 0, Top_Abstract_Level);
			
			fast_assert(abt_init == abt_goal);
		}
		
		unsigned getInitState() const {
			return mInitState;
		}
		

		std::pair<unsigned,unsigned> genRandomInitAndGoal(double pMinDist, unsigned pSeed, unsigned pSkip) {
			std::mt19937 randgen(pSeed);
			std::uniform_int_distribution<unsigned> dist(0, mCellMap.size()-1);
			
			unsigned tries = 0;
			
			while(true) {
				tries++;
				if(tries == 10000)
					throw std::runtime_error("genRandomInitAndGoal: too many tries.");
					
				unsigned i = dist(randgen), g = dist(randgen);
				
				double ix = i%mCellMap.getWidth(), iy = i/mCellMap.getWidth();
				double gx = g%mCellMap.getWidth(), gy = g/mCellMap.getWidth();
				
				if(std::hypot(ix-gx, iy-gy) < pMinDist)
					continue;
				
				if(mCellMap.cells()[i] != CellMap2D<>::Cell_t::Open || mCellMap.cells()[g] != CellMap2D<>::Cell_t::Open)
					continue;

				unsigned abt_init = mAbtInfo.abstractBaseState(i);
				unsigned abt_goal = mAbtInfo.abstractBaseState(g);
				
				abt_init = mAbtInfo.abstractState(abt_init, 0, Top_Abstract_Level);
				abt_goal = mAbtInfo.abstractState(abt_goal, 0, Top_Abstract_Level);
				
				if(abt_init != abt_goal)
					continue;
				
				if(pSkip > 0) {
					pSkip--;
					continue;
				}
				
				return {i, g};
			}
		}

		
		CellMap2D<> const& mCellMap;
		StarAbtInfo<typename BaseDomain_t::Cost> const& mAbtInfo;
		unsigned mInitState, mGoalState;
	};
	
	
	

	template<typename BaseDomain_t>
	std::pair<unsigned,unsigned> genRandomInitAndGoal(CellMap2D<> const& pCellMap, double pMinDist, unsigned pSeed) {
		std::mt19937 randgen(pSeed);
		std::uniform_int_distribution<unsigned> dist(0, pCellMap.size()-1);
		
		using D = DomainStack_BaseOnly<BaseDomain_t>;
		D domStack(pCellMap, Json());
		
		unsigned tries = 0;
		
		while(true) {
			tries++;
			if(tries == 1000)
				throw std::runtime_error("");
					
			unsigned i = dist(randgen), g = dist(randgen);
			
			double ix = i%pCellMap.getWidth(), iy = i/pCellMap.getWidth();
			double gx = g%pCellMap.getWidth(), gy = g/pCellMap.getWidth();
			
			if(std::hypot(ix-gx, iy-gy) < pMinDist)
				continue;
			
			if(pCellMap.cells()[i] != CellMap2D<>::Cell_t::Open || pCellMap.cells()[g] != CellMap2D<>::Cell_t::Open)
				continue;
			
			domStack.setInitAndGoal(i, g);
			
			using Alg_t = algorithm::Astar<D, algorithm::AstarSearchMode::Speedy, algorithm::AstarHrMode::DomainHr>;
			
			Alg_t alg(domStack, Json());
			
			try {
				alg.execute(domStack.getInitState());
			} catch(NoSolutionException const&) {
				continue;
			}
			
			return {i, g};
		}
	}
	
	
	
	
}}}
