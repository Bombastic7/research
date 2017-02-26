#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <iostream>

#include "util/debug.hpp"



namespace mjon661 { namespace starabt {
	
	template<typename BaseDomain>
	struct GroupEdge {
		unsigned dst;
		typename BaseDomain::Cost cost;
		
		bool operator()(GroupEdge<BaseDomain> const& a, GroupEdge<BaseDomain> const& b) {
			slow_assert(a.dst != b.dst);
			return a.dst < b.dst;
		}
	};
	
	using BaseGroupTrns = std::map<BaseState, unsigned>;
	using GroupTrns = std::vector<unsigned>;
	
	using GroupAdj = std::vector<GroupEdge>;
	using LevelGroupAdj = std::vector<GroupAdj>;

	static const unsigned Null_Group = (unsigned)-1;
	
	
	
	template<typename BaseDomain>
	void createBaseMap(	BaseDomain const& 								pDomain, 
						std::map<typename BaseDomain::State, unsigned>& pStateMap,
						std::vector<std::vector<GroupEdge>>&		 	pEdges)
	{
		using BaseState = typename BaseDomain::State;
		using Cost = typename BaseDomain::Cost;
		using OperatorSet = typename BaseDomain::OperatorSet;
		using Edge = typename BaseDomain::Edge;
		
		pStateMap.clear();
		pEdges.clear();
		
		unsigned curgrp = 0;
			
		for(auto it = pDomain.stateBegin(); it != pDomain.stateEnd(); ++it) {
			slow_assert(pStateMap.count(*it) == 0);
			pStateMap[*it] = curgrp++;
		}

		pEdges.resize(curgrp);
		
		for(auto it = pDomain.stateBegin(); it != pDomain.stateEnd(); ++it) {
			BaseState s = *it;
			OperatorSet opset = pDomain.createOperatorSet(s);
			
			for(unsigned i=0; i<opset.size(); i++) {
				Edge edge = pDomain.createEdge(s, opset[i]);
				pEdges[pStateMap[s]].push_back(GroupEdge{.dst=pStateMap.at(edge.state()), .cost=edge.cost()});
				mDomain.destroyEdge(edge);
			}
			
			std::sort(pEdges[pStateMap[s]].begin(), pEdges[pStateMap[s]].end(), GroupEdge());
		}
	}
	
	
	
	template<typename BaseDomain>
	void createAbstractLevel(	BaseDomain const& 						pDomain,
								unsigned								pAbtRadius,
								std::vector<std::vector<GroupEdge>>& 	pEdges,		//in, edges of level being abstracted.
								std::vector<unsigned>& 					pTrns,		//out, maps this-level state to next level state.
								std::vector<std::vector<GroupEdge>>& 	pAbtEdges)	//out, edges of next level.
	{
		pTrns.clear();
		pAbtEdges.clear();
		
		pTrns.resize(pEdges.size(), Null_Group);

		std::vector<std::pair<unsigned, unsigned>> hubprio;
		
		for(unsigned i=0; i<mLevelGroupAdj.back().size(); i++)
			hubprio.push_back(std::pair<unsigned, unsigned>(pEdges[i].size(), i));
		
		std::sort(hubprio.begin(), hubprio.end(), HubPrioCmp());
		
		unsigned curAbtGrp = 0;
		std::vector<unsigned> singletonGroups;
		
		for(unsigned i=0; i<hubprio.size(); i++) {
			if(tryAssignGroupRec(hubprio[i].second, 0, curAbtGrp) == 1)
				singletonGroups.push_back(hubprio[i].second);
			curAbtGrp++;
		}
		
		for(unsigned sg : singletonGroups) {			
			if(pEdges[sg].size() > 0)
				pTrns[sg] = pTrns[pEdges[sg][0].dst];
		}
		
		std::map<unsigned, unsigned> groupRelabel;
		curAbtGrp = 0;
		
		for(unsigned i=0; i<pTrns.size(); i++) {
			if(groupRelabel.count(pTrns[i] == 0)
				groupRelabel[pTrns[i]] = curAbtGrp++;
			pTrns[i] = groupRelabel.at(pTrns[i]);
		}
		
		std::map<unsigned, std::map<unsigned, Cost>> abtEdgeMap; //srcgrp -> dstgrp -> edgecost
		
		for(unsigned i=0; i<pEdges.size(); i++) {
			for(unsigned j=0; j<pEdges[i].size(); j++) {
				unsigned srcgrp = pTrns[i];
				unsigned dstgrp = pTrns[pEdges[i][j].dst];
				
				if(srcgrp == dstgrp)
					continue;
				
				Cost edgecost = pEdges[i][j].cost;
				
				if(abtEdgeMap[srcgrp].count(dstgrp) == 0)
					abtEdgeMap[srcgrp][dstgrp] = edgecost;
				else if(abtEdgeMap[srcgrp][dstgrp] > edgecost)
					abtEdgeMap[srcgrp][dstgrp] = edgecost;
			}
		}
		
		bool isTrivial = true;
		
		pAbtEdges.resize(curAbtGrp);

		for(unsigned i=0; i<curAbtGrp; i++) {
			for(auto it=abtEdgeMap[i].begin(); it != abtEdgeMap[i].end(); ++it) {
				abtEdgeMap[i].push_back(GroupEdge{.dst=it->first, .cost=it->second});
				isTrivial = false;
			}
		}
		
		return isTrivial;
	}
	
	unsigned tryAssignGroupRec(	std::vector<std::vector<GroupEdge>> const& pEdges, 
								std::vector<unsigned>& pTrns,
								unsigned i, 
								unsigned depth, 
								unsigned pAbtRadius, 
								unsigned curAbtGrp) {
		if(pTrns[i] != Null_Group || depth > pAbtRadius)
			return 0;
		
		pTrns[i] = curAbtGrp;
		
		unsigned ret = 1;
		for(unsigned j=0; j<pEdges[i].size(); j++)
			ret += tryAssignGroupRec(pEdges, pTrns, pEdges[i][j].dst, depth+1, pAbtRadius, curAbtGrp);
		
		return ret;
	}
	
	struct HubPrioCmp {
		bool operator()(std::pair<unsigned, unsigned> const& a, std::pair<unsigned, unsigned> const& b) const {
			if(a.first != b.first)
				return a.first > b.first;
			return a.second < b.second;
		}
	};
		
}}

	

/*

	template<typename BaseDomain>
	class StarAbtStack {
	
		public:
		using BaseState = typename BaseDomain::State;
		using Cost = typename BaseDomain::Cost;
		using OperatorSet = typename BaseDomain::OperatorSet;
		using Edge = typename BaseDomain::Edge;

		struct GroupEdge {
			unsigned dst;
			Cost cost;
		};
		
		using BaseGroupTrns = std::unordered_map<BaseState, unsigned>;
		using GroupTrns = std::vector<unsigned>;
		
		using GroupAdj = std::vector<GroupEdge>;
		using LevelGroupAdj = std::vector<GroupAdj>;

		static const unsigned Null_Group = (unsigned)-1;
		static const unsigned Hard_Abt_Limit = 10;
		
		


		template<unsigned L>
		struct StarAbtDomain {
			static_assert(L > 0, "");
			using AbtStack_t = StarAbtStack<BaseDomain>;
			
			using State = unsigned;
			using PackedState = unsigned;
			using Cost = typename AbtStack_t::Cost;
			using Operator = unsigned;
			
			template<unsigned> struct LevelTag {};
			
			struct OperatorSet {
				OperatorSet(GroupAdj const& pAdj) :
					mAdj(pAdj)
				{}
				
				unsigned size() {
					return mAdj.size();
				}
				
				unsigned operator[](unsigned i) {
					return i;
				}
				
				private:
				GroupAdj const& mAdj;
			};
			
			struct Edge {
				Edge(unsigned pState, Cost pCost, unsigned pParentOp) :
					mState(pState),
					mCost(pCost),
					mParentOp(pParentOp)
				{}
				
				unsigned state() {
					return mState;
				}
				
				Cost cost() {
					return mCost;
				}
				
				unsigned parentOp() {
					return mParentOp;
				}
				
				private:
				const unsigned mState;
				const Cost mCost;
				const unsigned mParentOp;
			};
			
			StarAbtDomain(AbtStack_t const& pInst) :
				mInst(pInst),
				mGoalState(pInst.abstractBaseToLevel(pInst.baseGoalState(), L))
			{}
			
			template<typename BS>
			unsigned abstractParentState(BS const& bs) {
				return doAbstractParentState(bs, LevelTag<L>{});
			}
			
			unsigned packState(unsigned i) {
				return i;
			}
			
			unsigned unpackState(unsigned i) {
				return i;
			}
			
			OperatorSet createOperatorSet(unsigned pState) {
				return OperatorSet(mInst.mLevelGroupAdj[L][pState]);
			}
			
			Edge createEdge(unsigned pState, unsigned op) {
				unsigned revop = Null_Group;
				unsigned dst = mInst.mLevelGroupAdj[L][pState][op].dst;
				
				for(unsigned i=0; i<mInst.mLevelGroupAdj[L][dst].size(); i++) {
					if(mInst.mLevelGroupAdj[L][dst][i].dst == pState) {
						revop = i;
						break;
					}
				}
				
				slow_assert(revop != Null_Group);
				return Edge(mInst.mLevelGroupAdj[L][pState][op].dst, mInst.mLevelGroupAdj[L][pState][op].cost, revop);
			}
			
			void destroyEdge(Edge&) const {
			}
			
			
			unsigned getNoOp() {
				return (unsigned)-1;
			}

			size_t hash(PackedState pPacked) const {
				return pPacked;
			}
			
			bool isPerfectHash() const {
				return true;
			}

			bool checkGoal(unsigned pState) const {
				return pState == mGoalState;
			}

			bool compare(unsigned a, unsigned b) const {
				return a == b;
			}
			
			void prettyPrint(State const& s, std::ostream& out) const {
				out << "[" << s << "]";
			}

			private:

			unsigned doAbstractParentState(typename AbtStack_t::BaseState const& bs, LevelTag<1>) {
				slow_assert(mInst.mBaseTrns.count(bs) == 1);
				return mInst.mLevelGroupTrns[0][mInst.mBaseTrns[bs]];
			}
			
			template<unsigned Lvl>
			unsigned doAbstractParentState(unsigned bs, LevelTag<Lvl>) {
				return mInst.mLevelGroupTrns[Lvl-1][bs];
			}
			
			AbtStack_t const& mInst;
			const unsigned mGoalState;
		};



		StarAbtStack(BaseDomain const& pDomain, unsigned pAbtRadius) :
			mDomain(pDomain),
			mAbtRadius(pAbtRadius),
			mBaseGoalState(pDomain.getGoalState())
		{
			prepBaseGroups();
			while(!prepAbtGroups()) {}
		}
		
		unsigned abstractBaseToLevel(BaseState const& s, unsigned lvl) {
			unsigned a = mBaseTrns.at(s);
			
			for(unsigned i=0; i<lvl; i++)
				a = mLevelGroupTrns.at(i).at(a);
			
			return a;
		}
		
		BaseState const& baseGoalState() {
			return mBaseGoalState;
		}
		
		void printBaseTrns(std::ostream& out) {
			out << "Base trns\n";
			for(auto it = mDomain.stateBegin(); it != mDomain.stateEnd(); ++it) {
				mDomain.prettyPrint(*it, out);
				out << " " << mBaseTrns.at(*it) << "\n";
			}
		}
		
		void printAbtTrns(unsigned lvl, std::ostream& out) {
			out << "Level " << lvl << " trns\n";
			for(unsigned i=0; i<mLevelGroupTrns.at(lvl).size(); i++) {
				out << i << " " << mLevelGroupTrns[lvl][i] << "\n";
			}
		}
		
		void printGroupEdges(unsigned lvl, std::ostream& out) {
			out << "Level " << lvl << " edges\n";
			for(unsigned i=0; i<mLevelGroupAdj.at(lvl).size(); i++) {
				out << i << ": ";
				for(unsigned j=0; j<mLevelGroupAdj[lvl][i].size(); j++) {
					out << "(" << mLevelGroupAdj[lvl][i][j].dst << ", " << mLevelGroupAdj[lvl][i][j].cost << ") ";
				}
				out << "\n";
			}
		}
		
		unsigned softAbtLimit() {
			return mLevelGroupAdj.size()-1;
		}
		
		
		private:

		void prepBaseGroups() {
			unsigned curgrp = 0;
			
			for(auto it = mDomain.stateBegin(); it != mDomain.stateEnd(); ++it) {
				slow_assert(mBaseTrns.count(*it) == 0);
				mBaseTrns[*it] = curgrp++;
			}

			mLevelGroupAdj.push_back(LevelGroupAdj(curgrp));
			for(auto it = mDomain.stateBegin(); it != mDomain.stateEnd(); ++it) {
				OperatorSet opset = mDomain.createOperatorSet(*it);
				
				GroupAdj grpadj;
				
				for(unsigned i=0; i<opset.size(); i++) {
					Edge edge = mDomain.createEdge(*it, opset[i]);
					grpadj.push_back(GroupEdge{.dst=mBaseTrns.at(edge.state()), .cost=edge.cost()});
					mDomain.destroyEdge(edge);
				}
				
				mLevelGroupAdj[0][mBaseTrns[*it]] = grpadj;
			}
		}
		
		bool prepAbtGroups() {
			unsigned nullGroup = Null_Group;
			mLevelGroupTrns.push_back(GroupTrns(mLevelGroupAdj.back().size(), nullGroup));
			
			std::vector<std::pair<unsigned, unsigned>> hubprio;
			
			for(unsigned i=0; i<mLevelGroupAdj.back().size(); i++)
				hubprio.push_back(std::pair<unsigned, unsigned>(mLevelGroupAdj.back()[i].size(), i));
			
			std::sort(hubprio.begin(), hubprio.end(), HubPrioCmp());
			
			unsigned curAbtGrp = 0;
			std::vector<unsigned> singletonGroups;
			
			for(unsigned i=0; i<hubprio.size(); i++) {
				if(tryAssignGroupRec(hubprio[i].second, 0, curAbtGrp) == 1)
					singletonGroups.push_back(hubprio[i].second);
				curAbtGrp++;
			}
			
			for(unsigned i=0; i<singletonGroups.size(); i++) {
				for(unsigned j=0; j<mLevelGroupAdj.back()[singletonGroups[i]].size(); j++) {
					if(!(std::find(singletonGroups.begin(), singletonGroups.end(), mLevelGroupAdj.back()[singletonGroups[i]][j].dst) == singletonGroups.end())) {
						slow_assert(false);
					}
				}
				
				if(mLevelGroupAdj.back()[singletonGroups[i]].size() > 0) {
					slow_assert(mLevelGroupTrns.back()[mLevelGroupAdj.back()[singletonGroups[i]][0].dst] != Null_Group);
					mLevelGroupTrns.back()[singletonGroups[i]] = mLevelGroupTrns.back()[mLevelGroupAdj.back()[singletonGroups[i]][0].dst];
				}
			}
			
			std::map<unsigned, unsigned> groupRelabel;
			curAbtGrp = 0;
			
			for(unsigned i=0; i<mLevelGroupTrns.back().size(); i++) {
				if(groupRelabel.count(mLevelGroupTrns.back()[i]) == 0)
					groupRelabel[mLevelGroupTrns.back()[i]] = curAbtGrp++;
				mLevelGroupTrns.back()[i] = groupRelabel.at(mLevelGroupTrns.back()[i]);
			}
			
			std::map<unsigned, std::map<unsigned, Cost>> abtgroupadj; //srcgrp -> dstgrp -> edgecost
			
			for(unsigned i=0; i<mLevelGroupAdj.back().size(); i++) {
				for(unsigned j=0; j<mLevelGroupAdj.back()[i].size(); j++) {
					unsigned srcgrp = mLevelGroupTrns.back()[i];
					unsigned dstgrp = mLevelGroupTrns.back()[mLevelGroupAdj.back()[i][j].dst];
					
					if(srcgrp == dstgrp)
						continue;
					
					Cost edgecost = mLevelGroupAdj.back()[i][j].cost;
					
					if(abtgroupadj[srcgrp].count(dstgrp) == 0)
						abtgroupadj[srcgrp][dstgrp] = edgecost;
					else if(abtgroupadj[srcgrp][dstgrp] > edgecost)
						abtgroupadj[srcgrp][dstgrp] = edgecost;
				}
			}
			
			bool isTrivial = true;
			
			mLevelGroupAdj.push_back(LevelGroupAdj(curAbtGrp));
			
			for(unsigned i=0; i<curAbtGrp; i++) {
				for(auto it=abtgroupadj[i].begin(); it != abtgroupadj[i].end(); ++it) {
					mLevelGroupAdj.back()[i].push_back(GroupEdge{.dst=it->first, .cost=it->second});
					isTrivial = false;
				}
			}
			
			return isTrivial;
		}
		
		
		unsigned tryAssignGroupRec(unsigned i, unsigned depth, unsigned curAbtGrp) {
			if(mLevelGroupTrns.back()[i] != Null_Group || depth > mAbtRadius)
				return 0;
			
			mLevelGroupTrns.back()[i] = curAbtGrp;
			
			unsigned ret = 1;
			for(unsigned j=0; j<mLevelGroupAdj.back()[i].size(); j++)
				ret += tryAssignGroupRec(mLevelGroupAdj.back()[i][j].dst, depth+1, curAbtGrp);
			
			return ret;
		}
		
		
		private:
		
		struct HubPrioCmp {
			bool operator()(std::pair<unsigned, unsigned> const& a, std::pair<unsigned, unsigned> const& b) const {
				if(a.first != b.first)
					return a.first > b.first;
				return a.second < b.second;
			}
		};
		
		BaseGroupTrns mBaseTrns;
		std::vector<LevelGroupAdj> mLevelGroupAdj;
		std::vector<GroupTrns> mLevelGroupTrns;
		BaseDomain const& mDomain;
		const unsigned mAbtRadius;
		const BaseState mBaseGoalState;
	};
}
*/
