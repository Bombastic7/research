#pragma once

#include <algorithm>
#include <vector>
#include <string>
#include <utility>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/exception.hpp"

#include "search/admissible_abtsearch2.hpp"

namespace mjon661 { namespace algorithm {
	
	enum struct EnumUtilSolutionsHrMode {
		DomainHr, AbtHr
	};
	

	template<typename = void>
	std::string enumUtilSolutions_HrModeStr(EnumUtilSolutionsHrMode m) {
		if(m == EnumUtilSolutionsHrMode::DomainHr)
			return "DomainHr";
		else if(m == EnumUtilSolutionsHrMode::AbtHr)
			return "AbtHr";
		else
			gen_assert(false);
		return "";
	}
	



	template<typename D, EnumUtilSolutionsHrMode Hr_Mode>
	class EnumUtilSolutions {
		public:

		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;

		
		
		template<EnumUtilSolutionsHrMode, typename = void>
		struct HrModule;
		
		template<typename Ign>
		struct HrModule<EnumUtilSolutionsHrMode::DomainHr, Ign> {
			HrModule(D& pDomStack, Domain const& pDomain, Json const& jConfig) :
				mDomain(pDomain)
			{}
			
			Cost costHeuristic(State const& s) {
				return mDomain.costHeuristic(s);
			}
			
			Cost distanceHeuristic(State const& s) {
				return mDomain.distanceHeuristic(s);
			};
						
			void insertReport(Json& jReport) {}
			
			private:
			Domain const& mDomain;
		};
		
		template<typename Ign>
		struct HrModule<EnumUtilSolutionsHrMode::AbtHr, Ign> {
			HrModule(D& pDomStack, Domain const& pDomain, Json const& jConfig) :
				mAbtSearch_cost(pDomStack, jConfig),
				mAbtSearch_dist(pDomStack, jConfig)
			{}
			
			Cost costHeuristic(State const& s) {
				return mAbtSearch_cost.getHrVal(s);
			}
			
			Cost distanceHeuristic(State const& s) {
				return mAbtSearch_dist.getHrVal(s);
			};
			
			void insertReport(Json& jReport) {
				Json j;
				j["cost_abt"] = Json();
				mAbtSearch_cost.insertReport(j.at("cost_abt"));
				j["dist_abt"] = Json();
				mAbtSearch_dist.insertReport(j.at("dist_abt"));
				jReport["hrmod"] = j;
			}
			
			private:
			AdmissibleAbtSearch<D,1,D::Top_Abstract_Level+1,true> mAbtSearch_cost;
			AdmissibleAbtSearch<D,1,D::Top_Abstract_Level+1,false> mAbtSearch_dist;
		};
		
		
		struct Node {
			Cost g;
			unsigned depth;
			PackedState pkd;
		};
		
		
		using OpenList_t = std::vector<Node>;
		
		
		EnumUtilSolutions(D& pDomStack, Json const& jConfig) :
			mDomStack			(pDomStack),
			mDomain				(pDomStack),
			mHrModule			(pDomStack, mDomain, jConfig)
		{}

		void reset() {
			mOpenList.clear();
			mSolutionsInfo.clear();
			mMinCost = mMaxCost = mMinDist = mMaxDist = 0;
			mLog_expd = mLog_gend = 0;
		}

		
		void execute(State const& s0) {
			{
				AdmissibleAbtSearch2<D, 0, D::Top_Abstract_Level, true> search_mincost(mDomStack, Json());
				AdmissibleAbtSearch2<D, 0, D::Top_Abstract_Level, false> search_mindist(mDomStack, Json());
				
				auto mincostres = search_mincost.execute(s0);
				auto mindistres = search_mindist.execute(s0);
				
				mMinCost = mincostres.first;
				mMaxCost = mindistres.second;
				mMinDist = mindistres.first;
				mMaxDist = mincostres.second;
			}
			doSearch(s0);
		}
		
		void doSearch(State const& s0) {
			reset();
			{
				Node n0;

				n0.g = 		Cost(0);
				n0.depth =	0;

				mDomain.packState(s0, n0.pkd);
				
				mOpenList.push_back(n0);
			}			
			

			while(true) {
				if(mOpenList.empty())
					break;
				
				Node n = mOpenList.back();
				mOpenList.pop_back();
				
				State s;
				mDomain.unpackState(s, n.pkd);

				if(mDomain.checkGoal(s)) {
					bool newSolInfo = true;
					
					for(auto& solinfo : mSolutionsInfo) {
						if(solinfo.first == n.g) {
							if(n.depth < solinfo.second)
								solinfo.second = n.depth;
							newSolInfo = false;
							break;
						}
						
						if(solinfo.second == n.depth) {
							if(n.g < solinfo.first)
								solinfo.first = n.g;
							newSolInfo = false;
							break;
						}
					}
					
					if(newSolInfo)
						mSolutionsInfo.push_back({n.g, n.depth});
				}
				
				expand(n, s);
			}
		}
		
		
		Json report() {
			Json j;
			
			j["expd"] = mLog_expd;
			j["gend"] = mLog_gend;
			j["cost_bound"] = {mMinCost, mMaxCost};
			j["dist_bound"] = {mMinDist, mMaxDist};
			j["n_sol_info"] = mSolutionsInfo.size();
			
			j["solinfo"] = {};
			for(auto& solinfo : mSolutionsInfo)
				j["solinfo"].push_back({{"g", solinfo.first}, {"depth", solinfo.second}});

			mHrModule.insertReport(j);
			return j;
		}
		

		
		void expand(Node const& n, State& s) {
			mLog_expd++;

			typename Domain::AdjEdgeIterator edgeIt = mDomain.getAdjEdges(s);
			
			for(; !edgeIt.finished(); edgeIt.next()) {
				
				PackedState kid_pkd;
				mDomain.packState(edgeIt.state(), kid_pkd);

				mLog_gend++;
				
				Cost kid_g = n.g + edgeIt.cost();
				unsigned kid_depth = n.depth + 1;
				
				slow_assert(kid_g >= mMinCost && kid_depth >= mMinDist);
				
				if(kid_g + mHrModule.costHeuristic(s) <= mMaxCost &&
					kid_depth + mHrModule.distanceHeuristic(s) <= mMaxDist) {
						
					Node kid_node;
					kid_node.g 		= kid_g;
					kid_node.depth	= kid_depth;
					kid_node.pkd 	= kid_pkd;
					
					mOpenList.push_back(kid_node);
				}
			}
		}

		D& mDomStack;
		Domain mDomain;
		OpenList_t mOpenList;
		HrModule<Hr_Mode> mHrModule;
		
		std::vector<std::pair<Cost, unsigned>> mSolutionsInfo;

		Cost mMinCost, mMaxCost;
		unsigned mMinDist, mMaxDist;
		
		unsigned mLog_expd, mLog_gend;
	};
}}
