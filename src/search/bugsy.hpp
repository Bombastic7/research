#pragma once

#include <array>
#include <string>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/time.hpp"
#include "util/exception.hpp"


namespace mjon661 { namespace algorithm {

	template<typename D, bool Use_Normed_Tradeoff>
	struct BugsyImpl {
		
		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;
		
		
		struct Node {
			PackedState pkd;
			Cost f, g;
			unsigned d;
			double u;
			
			Node *parent;
			Operator in_op, parent_op;
			
			unsigned expdGen;
		};
		
		struct ClosedOps {
			ClosedOps(Domain const& pDomain) : mDomain(pDomain) {}
			
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
				if (a->u != b->u)
					return a->u > b->u;
				if (a->f != b->f)
					return a->f < b->f;
				return a->g > b->g;
			}
		};
		
		
		using OpenList_t = OpenList<Node, Node, OpenOps>;
		
		using ClosedList_t = ClosedList<Node, 
										typename OpenList_t::Wrapped_t, 
										PackedState, 
										ClosedOps,
										ClosedOps,
										Domain::Hash_Range>;
									  
		using NodePool_t = NodePool<Node, typename ClosedList_t::Wrapped_t>;
		
		
		
		struct SearchStats {
			unsigned expd, gend, dups, reopnd, nresorts;
			
			void reset() {
				expd = gend = dups = reopnd = nresorts = 0;
			}
			
			Json report() {
				Json j;
				j["_all_expd"] = expd;
				j["_all_gend"] = gend;
				j["dups"] = dups;
				j["reopnd"] = reopnd;
				j["nresorts"] = nresorts;
				return j;
			}
			
			SearchStats() {
				reset();
			}
		};
		
		
		
		struct BugsyConfig {
			const double wf, wt;
			
			bool validate() {
				if(Use_Normed_Tradeoff && wt != 1)
					return false;

				return wf >= 0 && wt >= 0;
			}
			
			Json report() {
				Json j;
				j["wf"] = wf;
				j["wt"] = wt;
				j["used normalised exptime"] = Use_Normed_Tradeoff;
				return j;
			}
			
			BugsyConfig(Json const& j) :
				wf(j.at("wf")),
				wt(j.at("wt"))
			{
				if(!validate())
					throw ConfigException("");
			}
			
			bool mUseNormalisedExptime;
		};
		
		
		
		
		struct BugsyBehaviour : public SearchStats {
			static const unsigned First_Resort = 128;
			static const unsigned Next_Resort_Fact = 2;
			
			double last_avgExpDelay, last_avgExpTime;
			double next_accExpDelay;
			unsigned last_expd, next_expd;
			Timer mTimer;
			
			void reset() {
				SearchStats::reset();
				last_avgExpDelay = last_avgExpTime = 0;
				next_accExpDelay = 0;
				last_expd = 0;
				next_expd = Next_Resort_Fact;
				
				if(Use_Normed_Tradeoff)
					last_avgExpTime = 1;
				
				mTimer.start();
			}
			
			void notify_expansionDelay(unsigned pExpdAtGen) {
				next_accExpDelay += SearchStats::expd - pExpdAtGen;
			}
			
			bool shouldResort() {
				return SearchStats::expd >= next_expd;
			}
			
			void update() {
				mTimer.stop();
				double timeSinceLastUpdate = mTimer.seconds();
				mTimer.start();
				
				last_avgExpDelay = next_accExpDelay / (SearchStats::expd - last_expd);
				
				if(!Use_Normed_Tradeoff)
					last_avgExpTime = timeSinceLastUpdate / (SearchStats::expd - last_expd);
				
				next_accExpDelay = 0;
				
				last_expd = SearchStats::expd;
				next_expd = SearchStats::expd * Next_Resort_Fact;
			}
			
			Json report() {
				Json j = SearchStats::report();
				Json jB;
				jB["last_avgExpDelay"] = last_avgExpDelay;
				jB["last_avgExpTime"] = last_avgExpTime;
				jB["last_expd"] = last_expd;
				jB["next_expd"] = next_expd;
				
				j["bugsy"] = jB;
				return j;
			}
			
			BugsyBehaviour()
			{
				reset();
			}
		};
		
		
		BugsyImpl(D& pDomStack, Json const& jConfig) :
			mConfig				(jConfig),
			mBehaviour			(),
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			()
		{}
		
		Node* doSearch(State const& s0) {			
			{		
				Node* n0 = mNodePool.construct();
				
				n0->g = 		Cost(0);
				n0->f = 		mDomain.heuristicValue(s0);
				n0->d =			mDomain.distanceValue(s0);
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;
				
				n0->expdGen =	0;
				n0->u = 		computeUtil(n0->f, n0->d);
				
				mDomain.packState(s0, n0->pkd);

				mOpenList.push(n0);
				mClosedList.add(n0);
			}
			
			
			while(!mOpenList.empty()) {
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s))
					return n;
				
				expand(n, s);
				
				if(mBehaviour.shouldResort()) {
					mBehaviour.update();
					resortOpenList();
				}
			}
			
			return nullptr;
		}
		
		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mBehaviour.reset();
		}

		void prepareSolution(Solution<typename D::template Domain<0>>& sol, Node* pGoalNode) {
			std::vector<Node*> reversePath;
			
			for(Node *n = pGoalNode; n; n = n->parent)
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
					fast_assert(reversePath[i]->in_op == mDomain.noOp);
			}
		}
		
		Json report() {
			Json j = mBehaviour.report();
			j["Node size"] = sizeof(Node);
			j["Wrapped Node Size"] = sizeof(typename ClosedList_t::Wrapped_t);
			j["closed fill"] = mClosedList.getFill();
			j["closed table size"] = mClosedList.size();
			j["open size"] = mOpenList.size();
			j["open capacity"] = mOpenList.capacity();
			j["bugsy config"] = mConfig.report();
			return j;
		}
		
		BugsyConfig const& getConfig() {
			return mConfig;
		}
		
		
		private:
		
		void expand(Node* n, State& s) {
			mBehaviour.expd++;

			mBehaviour.notify_expansionDelay(n->expdGen);
			
			OperatorSet ops = mDomain.createOperatorSet(s);

			for(unsigned i=0; i<ops.size(); i++) {
				if(ops[i] == n->parent_op)
					continue;
				
				mBehaviour.gend++;
				considerkid(n, s, ops[i]);
			}
		}
		
		void considerkid(Node* pParentNode, State& pParentState, Operator const& pInOp) {

			Edge		edge 	= mDomain.createEdge(pParentState, pInOp);
			Cost 		kid_g   = pParentNode->g + edge.cost();
			
			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {				
				mBehaviour.dups++;
				
				if(kid_dup->g > kid_g) {
					kid_dup->f 		   -= kid_dup->g;
					kid_dup->f 		   += kid_g;
					kid_dup->g 		   = kid_g;
					kid_dup->in_op	   = pInOp;
					kid_dup->parent_op = edge.parentOp();
					kid_dup->parent	   = pParentNode;
					
					kid_dup->u		   = computeUtil(kid_dup->f, kid_dup->d);
					
					if(!mOpenList.contains(kid_dup))
						mBehaviour.reopnd++;
					
					mOpenList.pushOrUpdate(kid_dup);
				}
			} else {
				Node* kid_node 		= mNodePool.construct();

				kid_node->g 		= kid_g;
				kid_node->f 		= kid_g + mDomain.heuristicValue(edge.state());
				kid_node->d			= mDomain.distanceValue(edge.state());
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				kid_node->expdGen	= mBehaviour.expd;
				
				kid_node->u 		= computeUtil(kid_node->f, kid_node->d);
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
			}
			
			mDomain.destroyEdge(edge);
		}

		double computeUtil(Cost const& pCost, unsigned pDist) {
			return -(mConfig.wf * pCost + 
				   mConfig.wt * mBehaviour.last_avgExpDelay * pDist * mBehaviour.last_avgExpTime);
		}
		
		void resortOpenList() {
			for(unsigned i=0; i<mOpenList.size(); i++) {
				Node* n = mOpenList.at(i);
				n->u = computeUtil(n->f, n->d);
			}
			mOpenList.reinit();
			mBehaviour.nresorts++;
		}

		BugsyConfig			mConfig;
		BugsyBehaviour		mBehaviour;
		
		const Domain		mDomain;
		
		OpenList_t 			mOpenList;
		ClosedList_t 		mClosedList;
		NodePool_t 			mNodePool;
	};
	
	
	template<typename D, bool Use_Normed_Tradeoff>
	struct BugsyFront {
		using Bugsy_t =			BugsyImpl<D, Use_Normed_Tradeoff>;
		using Domain =			typename D::template Domain<0>;
		using State = 			typename Domain::State;
		using Node =			typename Bugsy_t::Node;
		
		BugsyFront(D& pDomStack, Json const& jConfig) :
			mDomain(pDomStack),
			mAlgo(pDomStack, jConfig)
		{}
		
		void execute(Solution<Domain>& pSolution) {
			State s0 = mDomain.createState();
			
			Node* goalNode = mAlgo.doSearch(s0);
			
			mAlgo.prepareSolution(pSolution, goalNode);
		}
		
		void reset() {
			mAlgo.reset();
		}
		
		Json report() {
			Json j = mAlgo.report();
			return j;
		}

		
		const Domain mDomain;
		Bugsy_t mAlgo;
	};
	
	template<typename D>
	using Bugsy = BugsyFront<D, true>;
	
	template<typename D>
	using Bugsy_Norm = BugsyFront<D, false>;
}}
