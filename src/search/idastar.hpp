#pragma once

#include <vector>
#include <string>
#include <limits>
#include "util/debug.hpp"
#include "util/json.hpp"

namespace mjon661 { namespace algorithm {
				
	template<typename D>
	class IDAstarImpl {
		
		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;
		
		public:
		
		/* Struct for per-round data */
		struct SearchStats {
			unsigned expd;
		};
		
		IDAstarImpl(D& pDomStack) :
			mStats				(),
			mDomain				(pDomStack),
			mTotExpd			(0)
		{}	
		
		void search(Solution<Domain>& sol, State& s0) {
			
			Cost 		nextCostBound		= mDomain.heuristicValue(s0);
			bool 		goalFound 			= false;
			bool		canUpBound			= true;
			unsigned 	roundCount 			= 1;
			
			
			while(!goalFound && canUpBound) {
				Cost costBound = nextCostBound;
				nextCostBound = std::numeric_limits<Cost>::max();
				
				const Operator No_Op = mDomain.noOp;
				goalFound = doSearch(s0, Cost(0), costBound, No_Op, nextCostBound);
				
				canUpBound = costBound != nextCostBound;
				
				mRoundInfo.push_back(
					std::string("Round ") + std::to_string(roundCount) + ": " +
					"bound=" + std::to_string(nextCostBound) +
					"  expd=" + std::to_string(mStats.expd)
				);

				
				mTotExpd += mStats.expd;
				mStats.expd = 0;
				roundCount++;
			}
			
			prepareSolution(sol);
		}
		
		bool doSearch(State& pState,
					  Cost const& pCost, 
					  Cost const& pBound, 
					  Operator const& pParentOp,
					  Cost& pMin) {

			Cost pState_f = pCost + mDomain.heuristicValue(pState);
			
			if(pState_f > pBound) {
				
				if(pMin > pState_f)
					pMin = pState_f;
				
				return false;
			}
				
			if(mDomain.checkGoal(pState)) {
				mSolutionPath.push_back(pState);
				return true;
			}
			
			mStats.expd++;
			
			OperatorSet opSet = mDomain.createOperatorSet(pState);
			
			for(unsigned i=0; i<opSet.size(); i++) {
				
				if(opSet[i] == pParentOp)
					continue;
				
				Edge outEdge = mDomain.createEdge(pState, opSet[i]);
				Cost kid_g = pCost + outEdge.cost();
				
				if(doSearch(outEdge.state(), kid_g, pBound, outEdge.parentOp(), pMin)) {
					mDomain.destroyEdge(outEdge);
					
					mSolutionPath.push_back(pState);
					mSolutionOperators.push_back(opSet[i]);
					
					return true;
				}

				mDomain.destroyEdge(outEdge);
			}
			
			return false;
		}
		
		void prepareSolution(Solution<typename D::template Domain<0>>& sol) {
			sol.states.clear();
			sol.operators.clear();
			
			fast_assert(mSolutionPath.size() == mSolutionOperators.size() + 1);
			
			for(unsigned i=mSolutionPath.size()-1; i!=(unsigned)-1; i--)
				sol.states.push_back(mSolutionPath[i]);
			
			for(unsigned i=mSolutionOperators.size()-1; i!=(unsigned)-1; i--)
				sol.operators.push_back(mSolutionOperators[i]);	
		}
		
		Json report() {
			Json j;
			j["Rounds"] = mRoundInfo;
			j["State size"] = sizeof(State);
			j["expd"] = mTotExpd;
			
			return j;
		}
		
		void reset() {
			mRoundInfo.clear();
			mSolutionPath.clear();
			mSolutionOperators.clear();
		}
		
		std::vector<std::string> 	mRoundInfo;
		std::vector<State>			mSolutionPath;
		std::vector<Operator> 		mSolutionOperators;
		
		SearchStats 		mStats;		
		const Domain		mDomain;
		unsigned 			mTotExpd;
	};



	template<typename D>
	class IDAstar {
		public:
		using Domain = typename D::template Domain<0>;
		using State = typename Domain::State;
		
		IDAstar(D& pDomStack, Json const&) :
			mAlgo(pDomStack),
			mDomain(pDomStack)
		{}
		
		void execute(Solution<Domain>& sol) {
			State s0 = mDomain.createState();
			mAlgo.search(sol, s0);
		}
		
		Json report() {
			return mAlgo.report();
		}
		
		void reset() {
			mAlgo.reset();
		}
		
		private:
		IDAstarImpl<D> mAlgo;
		const Domain mDomain;
	};	
}}
