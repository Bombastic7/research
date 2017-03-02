#pragma once

#include <algorithm>
#include <array>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <random>

#include "util/json.hpp"
#include "util/debug.hpp"

namespace mjon661 { namespace testgraph {


	template<unsigned N>
	struct TestGraphDomain {
		
		using State = unsigned;
		using PackedState = unsigned;
		using Cost = unsigned;
		using Operator = unsigned;
		
		static const Cost Null_Cost = (Cost)-1;
		
		static const bool Is_Perfect_Hash = true;
		
		struct OperatorSet {
			OperatorSet(std::vector<Cost> const& pAdjMatrix, unsigned s) {
				for(unsigned i=s*N; i<(s+1)*N; i++) {
					if(pAdjMatrix[i] != Null_Cost) {
						mOps.push_back(i%N);
					}
				}
			}
			
			unsigned size() { return mOps.size(); }
			unsigned operator[](unsigned i) { return mOps.at(i); }

			std::vector<unsigned> mOps;
		};
		
		
		struct Edge {
			Edge(unsigned s, Cost pCost, unsigned p) :
				mState(s),
				mCost(pCost),
				mParent(p)
			{}
			
			unsigned state() { return mState; }
			Cost cost() { return mCost; }
			unsigned parentOp() { return mParent; }
			
			unsigned mState;
			Cost mCost;
			unsigned mParent;
		};
		
		
		TestGraphDomain() {
			std::uniform_int_distribution<unsigned> d(1,4);
			std::mt19937 gen;
			
			mAdjMatrix.resize(N*N);
			
			for(unsigned i=0; i<N*N; i++)
				mAdjMatrix[i] = d(gen);
			
		}


		TestGraphDomain(Json const& jConfig) {
			mAdjMatrix = jConfig.at("adj_matrix").get<decltype(mAdjMatrix)>();
			gen_assert(mAdjMatrix.size() == N*N);
		}
		
		Operator getNoOp() const {
			return (unsigned)-1;
		}
		
		void packState(unsigned s, unsigned& pkd) const {
			pkd = s;
		}
		
		void unpackState(unsigned& s, unsigned pkd) const {
			s = pkd;
		}
		
		OperatorSet createOperatorSet(unsigned s) const {
			return OperatorSet(mAdjMatrix, s);
		}
		
		Edge createEdge(unsigned s, unsigned op) const {
			return Edge(op, mAdjMatrix.at(s*N+op), s);
		}
		
		void destroyEdge(Edge&) const {}
		
		bool checkGoal(unsigned s) const {
			return s == N-1;
		}
		
		size_t hash(unsigned s) const {
			return s;
		}
		
		bool compare(unsigned a, unsigned b) const {
			return a == b;
		}
		
		void prettyPrint(unsigned s, std::ostream& out) const {
			out << s;
		}
	
		void dump(std::ostream& out) const {
			for(unsigned i=0; i<N; i++) {
				out << i << ": ";
				for(unsigned j=0; j<N; j++) {
					if(mAdjMatrix[i*N+j] != Null_Cost)
						out << mAdjMatrix[i*N+j];
					out << ",";
					if((j+1)%5 == 0)
						out << " ";
				}
				out << "\n";
			}
		}
		
		Cost& getCostEntry(unsigned src, unsigned dst) {
			return mAdjMatrix.at(src*N+dst);
		}
		
		std::vector<Cost> mAdjMatrix;
		
	};
	
	
	template<unsigned N>
	struct GridTestGraphDomain : public TestGraphDomain<N*N> {
		
		using TestGraphDomain<N*N>::Null_Cost;
		
		
		GridTestGraphDomain() :
			TestGraphDomain<N*N>()
		{
			unsigned nullCost = Null_Cost;
			std::fill(this->mAdjMatrix.begin(), this->mAdjMatrix.end(), nullCost);
			
			std::uniform_int_distribution<unsigned> d(1,4);
			std::mt19937 gen;
			
			for(unsigned i=0; i<N*N; i++) {
				if(i >= N) this->getCostEntry(i, i-N) = d(gen);
				if(i < N*(N-1)) this->getCostEntry(i, i+N) = d(gen);
				if(i % N != 0) this->getCostEntry(i, i-1) = d(gen);
				if((i+1) % N != 0) this->getCostEntry(i, i+1) = d(gen);
			}
		}
		
		
	};



	template<unsigned N>
	struct TestGraphStack {
		
		static const unsigned Top_Abstract_Level = 0;
		
		template<unsigned>
		struct Domain : public TestGraphDomain<N> {
			Domain(TestGraphStack<N> const& o) :
				TestGraphDomain<N>(o.mDom)
			{}
		};
		
		TestGraphStack() :
			mDom()
		{}
		
		unsigned getInitState() const {
			return 0;
		}
		
		TestGraphDomain<N> mDom;
	};




	template<unsigned N>
	struct GridTestGraphStack {
		
		static const unsigned Top_Abstract_Level = 0;
		
		template<unsigned>
		struct Domain : public GridTestGraphDomain<N> {
			Domain(GridTestGraphStack<N> const& o) :
				GridTestGraphDomain<N>(o.mDom)
			{}
		};
		
		GridTestGraphStack() :
			mDom()
		{}
		
		unsigned getInitState() const {
			return 0;
		}
		
		GridTestGraphDomain<N> mDom;
	};
}}
