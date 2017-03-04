#pragma once

#include <algorithm>
#include <iostream>
#include <fstream>
#include <utility>
#include <vector>

#include "search/openlist.hpp"
#include "search/closedlist.hpp"
#include "search/nodepool.hpp"
#include "search/cache_store.hpp"
#include "structs/object_pool.hpp"
#include "util/json.hpp"
#include "util/debug.hpp"

#include "search/admissible_abtsearch2.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav6 {


	

	template<typename D>
	class UGSA_CostPure_impl {
		public:

		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;

		using AbtSearch = AdmissibleAbtSearch2<D, 1, D::Top_Abstract_Level, true>;
		
		struct Node {
			Cost g, f;
			unsigned depth;
			double u;
			
			PackedState pkd;
			Node* parent;
			Operator in_op, parent_op;
			
			double log_expCount;
			double log_remexp;
		};
		
		
		struct ClosedOps {
			ClosedOps(Domain const& pDomain) :
				mDomain(pDomain)
			{}
			
			bool operator()(Node * const n, PackedState const& pkd) const {
				return mDomain.compare(n->pkd, pkd);
			}
			
			size_t operator()(Node * const n) const {
				return mDomain.hash(n->pkd);
			}
			
			size_t operator()(PackedState const& pkd) const {
				return mDomain.hash(pkd);
			}
			
			private:
			Domain const& mDomain;
		};
		
		struct OpenOps {
			bool operator()(Node * const a, Node * const b) const {
				if(a->u != b->u)
					return a->u < b->u;
				return a->g > b->g;
			}
		};
		
		
		using OpenList_t = OpenList<Node, Node, OpenOps>;
		
		using ClosedList_t = ClosedList<Node, 
										typename OpenList_t::Wrapped_t, 
										PackedState, 
										ClosedOps,
										ClosedOps,
										Domain::Is_Perfect_Hash>;
									  
		using NodePool_t = NodePool<Node, typename ClosedList_t::Wrapped_t>;



		UGSA_CostPure_impl(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mAbtSearch			(pDomStack, jConfig),
			mParams_wf			(jConfig.at("wf")),
			mParams_wt			(jConfig.at("wt"))
		{
			fast_assert(mParams_wf >= 0 && mParams_wt >= 0);
		}

		
		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mAbtSearch.reset();
			
			mResort_n = 0;
			mResort_next = 16;
			
			mLog_exp_f.clear();
			mLog_exp_u.clear();
			mLog_expd = mLog_gend = mLog_dups = mLog_reopnd = 0;
			
			mParams_bf = 0;
		}
		
		void clearCache() {
			mAbtSearch.clearCache();
		}
		

		
		void doSearch(State const& s0, Solution<D>& pSolution) {
			reset();
			
			{
				Node* n0 = mNodePool.construct();

				n0->g = 		Cost(0);
				n0->depth =		0;
				n0->in_op = 	mDomain.getNoOp();
				n0->parent_op = mDomain.getNoOp();
				n0->parent = 	nullptr;

				evalHr(n0, s0);

				mDomain.packState(s0, n0->pkd);
				
				mOpenList.push(n0);
				mClosedList.add(n0);
			}			
			
			Node* goalNode;
			
			while(true) {				
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s)) {
					prepareSolution(pSolution, n);
					goalNode = n;
					break;
				}
				
				expand(n, s);
				
				if(mLog_expd == mResort_next) {
					doCompBF();
					mResort_n++;
					mResort_next *= 2;
				}
				
				//if(mLog_expd == mResort_next)
					//doResort();
			}
			
			for(Node* n=goalNode; n; n=n->parent) {
				mLog_solpathRemExp.push_back({n->log_expCount, n->log_remexp});
			}
		}
		
		
		Json report() {
			Json j;
			j["exp_f"] = mLog_exp_f;
			j["exp_u"] = mLog_exp_u;
			j["resort_k"] = mLog_resort_k;
			j["resort_bf"] = mLog_resort_bf;
			j["resort_n"] = mResort_n;
			j["resort_next"] = mResort_next;
			j["sol_remexp"] = mLog_solpathRemExp;
			
			j["expd"] = mLog_expd;
			j["gend"] = mLog_gend;
			j["reopnd"] = mLog_reopnd;
			j["dups"] = mLog_dups;
			
			j["bf_stats"] = mathutil::sampleStats(mLog_resort_bf);
			
			std::vector<double> remexpError;
			for(auto p : mLog_solpathRemExp) {
				double trueRemExp = mLog_expd - p[0];
				if(trueRemExp == 0)
					continue;
				remexpError.push_back(p[1] / trueRemExp);
			}
			
			j["remexp_errors"] = remexpError;
			j["remexp_stats"] = mathutil::sampleStats(remexpError);
			
			//j["abt"] = mAbtSearch.report();
			return j;
		}
		
		
		private:
		
		void prepareSolution(Solution<D>& sol, Node* pGoalNode) {
			std::vector<Node*> reversePath;
			
			for(Node *n = pGoalNode; n != nullptr; n = static_cast<Node*>(n->parent))
				reversePath.push_back(n);
			
			sol.states.clear();
			sol.operators.clear();
			
			for(unsigned i=reversePath.size()-1; i!=(unsigned)-1; i--) {
				State s;
				mDomain.unpackState(s, reversePath[i]->pkd);
				
				sol.states.push_back(s);
				
				if(i != reversePath.size()-1)
					sol.operators.push_back(reversePath[i]->in_op);	
				else
					fast_assert(reversePath[i]->in_op == mDomain.getNoOp());
			}
		}
		
		
		void expand(Node* n, State& s) {
			//inform expansion
			mLog_expd++;
			
			informExpansion(n);
			
			OperatorSet ops = mDomain.createOperatorSet(s);
			
			for(unsigned i=0; i<ops.size(); i++) {
				if(ops[i] == n->parent_op)
					continue;

				mLog_gend++;
				considerkid(n, s, ops[i]);
			}
		}
		
		
		void considerkid(Node* pParentNode, State& pParentState, Operator const& pInOp) {
			Edge		edge 	= mDomain.createEdge(pParentState, pInOp);
			Cost 		kid_g   = pParentNode->g + edge.cost();
			unsigned	kid_depth = pParentNode->depth + 1;
			
			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				mLog_dups++;
				if(kid_dup->g > kid_g) {
					kid_dup->g			= kid_g;
					kid_dup->depth		= kid_depth;
					kid_dup->in_op		= pInOp;
					kid_dup->parent_op	= edge.parentOp();
					kid_dup->parent		= pParentNode;
					
					evalHr(kid_dup, edge.state());
					
					if(!mOpenList.contains(kid_dup)) {
						mLog_reopnd++;
					}

					mOpenList.pushOrUpdate(kid_dup);
				}
			}
			else {
				Node* kid_node 		= mNodePool.construct();

				kid_node->g 		= kid_g;
				kid_node->depth		= kid_depth;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				
				evalHr(kid_node, edge.state());

				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
			}
			
			mDomain.destroyEdge(edge);
		}
		
		
		double compRemExp(unsigned depth) {
			Util_t acc = 0;
			for(unsigned i=1; i<=depth; i++) {
				acc += std::pow(mParams_bf, i);
			}
			
			return acc;
		}
		
		
		void evalHr(Node* n, State const& s) {
			Cost remcost;
			unsigned remdepth;

			mAbtSearch.doSearch(s, remcost, remdepth);
			double remexp = compRemExp(remdepth);
			
			n->f = n->g + remcost;
			//n->u = mParams_wf*n->f + mParams_wt*remexp;
			n->u = n->f;
			
			n->log_expCount = mLog_expd;
			n->log_remexp = remexp;
		}
		
		
		void informExpansion(Node* n) {
			mLog_exp_f.push_back(n->f);
			mLog_exp_u.push_back(n->u);
		}
		
		void doCompBF() {
			std::map<Cost, unsigned> fcount;
			for(auto it=mLog_exp_f.begin(); it!=mLog_exp_f.end(); ++it) {
				if(fcount.count(*it) == 0)
					fcount[*it] = 1;
				else
					fcount[*it]++;
			}
			
			std::vector<double> bfsamples;
			
			auto fcntit = fcount.begin();
			slow_assert(fcntit != fcount.end());
			
			unsigned prevcount = fcntit->second;
			++fcntit;
			for(; fcntit!=fcount.end(); ++fcntit) {
				bfsamples.push_back((double)fcntit->second / prevcount);
				prevcount = fcntit->second;
			}
			
			double bf = 0;
			if(bfsamples.size() > 0)
				bf = bfsamples[(bfsamples.size()-1)/2];
			mLog_resort_bf.push_back(bf);
		}
		
		/*
		void doResort() {
			std::map<Cost, unsigned> fcount;
			for(auto it=mLog_exp_f.begin(); it!=mLog_exp_f.end(); ++it) {
				if(fcount.count(*it) == 0)
					fcount[*it] = 1;
				else
					fcount[*it]++;
			}
			
			std::vector<double> bfsamples;
			
			auto fcntit = fcount.begin();
			slow_assert(fcntit != fcount.end());
			
			unsigned prevcount = fcntit->second;
			++fcntit;
			for(; fcntit!=fcount.end(); ++fcntit) {
				bfsamples.push_back((double)fcntit->second / prevcount);
				prevcount = fcntit->second;
			}
			
			std::sort(bfsamples.begin(), bfsamples.end());
			slow_assert(bfsamples.size() >= 1);
			
			double bf = bfsamples[(bfsamples.size()-1)/2];
			double k = mOpenList.size();
			
			mAbtSearch.resetParams(k, bf);
			
			mLog_resort_bf.push_back(bf);
			mLog_resort_k.push_back(k);
			
			mResort_n++;
			mResort_next *= 2;
			
			for(unsigned i=0; i<mOpenList.size(); i++) {
				Node* n = mOpenList.at(i);
				State s;
				mDomain.unpackState(s, n->pkd);
				evalHr(n, s, n->g);
			}
			
			mOpenList.reinit();
		}
		*/
		
		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		AbtSearch mAbtSearch;
		double mParams_wf, mParams_wt, mParams_bf;
		
		unsigned mResort_n, mResort_next;
		
		std::vector<Cost> mLog_exp_f;
		std::vector<Util_t> mLog_exp_u;
		std::vector<double> mLog_resort_bf;
		std::vector<double> mLog_resort_k;
		unsigned mLog_expd, mLog_gend, mLog_dups, mLog_reopnd;
		
		std::vector<std::vector<double>> mLog_solpathRemExp;
	};


	template<typename D>
	struct UGSA_CostPure {
		
		using State = typename D::template Domain<0>::State;
		
		UGSA_CostPure(D& pDomStack, Json const& jConfig) :
			mAlgo(pDomStack, jConfig)
		{}
		
		void execute(State const& s0, Solution<D>& pSol) {
			mAlgo.doSearch(s0, pSol);
		}
		
		Json report() {
			return mAlgo.report();
		}
		
		UGSA_CostPure_impl<D> mAlgo;
	};
	
}}}
