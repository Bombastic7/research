#pragma once

#include <iostream>
#include <vector>
#include <cmath>

#include "search/openlist.hpp"
#include "search/closedlist.hpp"
#include "search/nodepool.hpp"
#include "search/cache_store.hpp"
#include "structs/object_pool.hpp"
#include "util/json.hpp"
#include "util/debug.hpp"

#include "search/ugsa/v6/defs.hpp"

namespace mjon661 { namespace algorithm { namespace ugsav6 {



	template<typename D, unsigned L, unsigned Bound>
	class UGSAv6_Abt {
		public:

		using Domain = typename D::template Domain<L>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;


		struct GoalPin {
			GoalPin() {nxt = nullptr;}
			void destroy() {if(nxt) nxt->destroy(); delete this;}
			PackedState goalPkd;
			unsigned mindepth, maxdepth;
			Cost cost;
			unsigned dist;
			GoalPin* nxt;
		};
		

		struct ExactCacheEntry {
			PackedState pkd;
			GoalPin* goalPinsHead;
			
			ExactCacheEntry() { goalPinsHead = nullptr; }
			
			GoalPin* find(bool& newPin, Domain const& dom, PackedState const& goalPkd) {
				
				
				if(goalPinsHead != nullptr) {
					GoalPin* gp = goalPinsHead;
					while(true) {
						if(dom.compare(gp->goalPkd, goalPkd)) {
							newPin = false;
							return gp;
						}
						if(gp->nxt == nullptr)
							break;
						gp = gp->nxt;
					}
				
					gp->nxt = new GoalPin;
					gp->goalPkd = goalPkd;
					newPin = true;
					return gp;
				}
				else {
					goalPinsHead = new GoalPin;
					goalPinsHead->goalPkd = goalPkd;
					newPin = true;
					return goalPinsHead;
				}
			}
		};


		struct Node {
			Cost g;
			unsigned depth;
			Util_t ug, uf;
			
			PackedState pkd;
			Node* parent;
			Operator in_op, parent_op;
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
				if(a->uf != b->uf)
					return a->uf < b->uf;
				if(a->ug != b->ug)
					return a->ug > b->ug;
				if(a->depth != b->depth)
					return a->uf > b->uf;
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

		using CacheStore_t = CacheStore<Domain, ExactCacheEntry>;

		

		UGSAv6_Abt(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mExactCache			(mDomain),
			mParams_wf			(jConfig.at("wf")),
			mParams_wt			(jConfig.at("wt")),
			mParams_k			(0),
			mParams_bf			(0)
		{
			fast_assert(mParams_wf >= 0 && mParams_wt >= 0);
		}

		
		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
		}
		
		void clearCache() {
			for(auto it=mExactCache.begin(); it!=mExactCache.end(); ++it)
				(*it)->goalPinsHead->destroy();
			
			mExactCache.clear();
		}
		
		void submitStats() {
		}
		
		void resetParams(double k, double bf) {
			mParams_k = k;
			mParams_bf = bf;
			//clear cache
		}
		
		
		template<typename PS>
		void computeRemainingEffort_parentState(PS const& pParentState, Cost& out_cost, Util_t& out_remexp) {
			State s0 = mDomain.abstractParentState(pParentState);
			computeRemainingEffort(s0, out_cost, out_remexp);
		}
		
		void computeRemainingEffort(State const& s0, Cost& out_cost, Util_t& out_remexp) {
			Cost remcost;
			unsigned remdist;
			if(tryFindExactCached(s0, remcost, remdist)) {
				out_cost = remcost;
				out_remexp = compRemExp(remdist);
				return;
			}
			
			doSearch(s0);
			
			//slow_assert(tryFindExactCached(s0, remcost, remdist));
			
			out_cost = 0;//remcost;
			out_remexp = 0;//compRemExp(remdist);
		}
		
		//private:
		//void compPartialPathUtil()
		
		
		bool tryFindExactCached(State const& s, Cost&, unsigned&) {
			return false;
		}
		
		
		void doSearch(State const& s0) {
			Node* n0 = mNodePool.construct();

			n0->g = 		Cost(0);
			n0->depth =		0;
			n0->ug = 		0;
			n0->uf =		0;//u-heuristic
			n0->in_op = 	mDomain.getNoOp();
			n0->parent_op = mDomain.getNoOp();
			n0->parent = 	nullptr;

			mDomain.packState(s0, n0->pkd);
			
			mOpenList.push(n0);
			mClosedList.add(n0);
			
			Node* goalNode = nullptr;
			
			while(true) {				
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s)) {
					goalNode = n;
					break;
				}
				
				expand(n, s);
			}

			doCaching(goalNode);
			//mStatsAcc.s_end();
		}
		

		
		void expand(Node* n, State& s) {
			//mStatsAcc.a_expd();
			
			OperatorSet ops = mDomain.createOperatorSet(s);
			
			for(unsigned i=0; i<ops.size(); i++) {
				if(ops[i] == n->parent_op)
					continue;
				
				considerkid(n, s, ops[i]);
			}
		}
		
		
		void considerkid(Node* pParentNode, State& pParentState, Operator const& pInOp) {
			Edge		edge 	= 		mDomain.createEdge(pParentState, pInOp);
			Cost 		kid_g   = 		pParentNode->g + edge.cost();
			unsigned	kid_depth = 	pParentNode->depth + 1;
			Util_t		kid_ug =		mParams_wf * kid_g + mParams_wt * mParams_k * compRemExp(kid_depth);
			Util_t		kid_uf = 		kid_ug;// + //..
			
			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				//mStatsAcc.a_dups();
				if(kid_dup->ug > kid_ug) {
					kid_dup->uf			-= kid_dup->ug;
					kid_dup->uf			+= kid_ug;
					
					kid_dup->g			= kid_g;
					kid_dup->depth		= kid_depth;
					kid_dup->ug			= kid_ug;

					kid_dup->in_op		= pInOp;
					kid_dup->parent_op	= edge.parentOp();
					kid_dup->parent		= pParentNode;
					
					if(!mOpenList.contains(kid_dup)) {
						//mStatsAcc.a_reopnd();
					}

					mOpenList.pushOrUpdate(kid_dup);
				}
			}
			else {
				Node* kid_node 		= mNodePool.construct();

				kid_node->g 		= kid_g;
				kid_node->depth		= kid_depth;
				kid_node->ug		= kid_ug;
				kid_node->uf		= kid_uf;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;

				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
			}
			
			mDomain.destroyEdge(edge);
		}


		void doCaching(Node* goalNode) {
			//if goalNode is exact...
			
			Cost goal_cost = goalNode->g;
			unsigned goal_depth = goalNode->depth;
			
			
			for(Node* n=goalNode; n!=nullptr; n=n->parent) {
				State s;
				mDomain.unpackState(s, n->pkd);
				
				ExactCacheEntry* ent;
				mExactCache.get(n->pkd, ent);

				bool newPin;
				GoalPin* goalpin = ent->find(newPin, mDomain, goalNode->pkd);
				
				if(newPin) {
					goalpin->mindepth = goalpin->maxdepth = n->depth;
					goalpin->cost = goal_cost - n->g;
					goalpin->dist = goal_depth - n->depth;
				} else {
					goalpin->mindepth = mathutil::min(goalpin->mindepth, n->depth);
					goalpin->maxdepth = mathutil::max(goalpin->maxdepth, n->depth);
					slow_assert(goalpin->cost == goal_cost - n->g);
					slow_assert(goalpin->dist == goal_depth - n->depth);
				}
			}
		}
		
		Util_t compRemExp(unsigned depth) {
			Util_t acc = 0;
			for(unsigned i=1; i<=depth; i++)
				acc += std::pow(mParams_bf, i);
			return acc;
		}
		
		void dumpExactCache(std::ostream& out) {
			for(auto it=mExactCache.begin(); it!=mExactCache.end(); ++it) {
				State s;
				mDomain.unpackState(s, (*it)->pkd);
				mDomain.prettyPrint(s, out);
				out << "\n\nPINS:\n";
				
				for(GoalPin* gp=(*it)->goalPinsHead; gp; gp=gp->nxt) {
					out << gp->mindepth << " " << gp->maxdepth << " " << gp->cost << " " << gp->dist << "\n";
					State goalState;
					mDomain.unpackState(goalState, gp->goalPkd);
					mDomain.prettyPrint(goalState, out);
					out << "\n\n";
				}
				
				out << "\n\n\n\n";
			}
		}
		

		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		CacheStore_t mExactCache;
		double mParams_wf, mParams_wt, mParams_k, mParams_bf;
	};
}}}
