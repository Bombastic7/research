#pragma once

#include <array>
#include <iostream>
#include <random>
#include <vector>
#include "util/json.hpp"
#include "util/debug.hpp"



namespace mjon661 { namespace bubbles {

	using Cell_t = unsigned short;
	
	const Cell_t NullCell = -1;
	const unsigned NullInt = -1;
	

	template<unsigned H, unsigned W, unsigned Sz>
	struct State_t : public std::array<Cell_t, H*W> {
		bool valid() const {
			for(unsigned i=0; i<H*W; i++)
				if((*this)[i] >= Sz)
					return false;
			return true;
		}
		
		size_t fnv_hash() const {
			size_t h = 2166136261;

			for(unsigned i=0; i<H*W; i++) {
				h = (h * 16777619) ^ (*this)[i];
			}

			return h;
		}
		
		void prettyPrint(std::ostream& out) const {
			for(unsigned h=0; h<H; h++) {
				for(unsigned w=0; w<W; w++) {
					Cell_t c = (*this)[w+h*W];
					if(c != NullCell)
						out << c << " ";
					else
						out << ". ";
				}
				out << "\n";
			}
		}
		
		void clearBubbles() {
			for(unsigned i=0; i<H*W; i++)
				(*this)[i] = NullCell;
		}
		
		void setRandom(unsigned r) {
			std::mt19937 randgen(r);
			std::uniform_int_distribution<unsigned> dist(0,Sz-1);
			
			for(unsigned i=0; i<H*W; i++)
				(*this)[i] = dist(randgen);
			
			fast_assert(valid());
		}
	};
	
	
	template<unsigned H, unsigned W, unsigned Sz>
	struct BaseEdgeIterator {
		
		BaseEdgeIterator(State_t<H,W,Sz> const& s) :
			mOrigState(s)
		{
			mOp = 0;
			prepGroups();
			if(!finished())
				adv();
		}
		
		State_t<H,W,Sz> state() {
			return mAdjState;
		}
		
		int cost() {
			return 1;
		}
		
		bool finished() {
			return mOp == mGroups.size();
		}
		
		void next() {
			slow_assert(!finished());			
			mOp++;
			adv();
		}
		
		void adv() {
			while(!finished()) {
				applyMove(mGroups[mOp]);
				break;
				//if(!shouldPrune())
				//	break;
				mOp++;
			}
		}
		
		
		void prepGroups() {
			mVisited.fill(NullCell);
			
			for(unsigned i=0; i<H*W; i++) {
				if(mOrigState[i] == NullCell)
					continue;
				
				unsigned sz = recVisit(i, i, mOrigState[i]);
				
				if(sz > 1)
					mGroups.push_back(i);
				else if(sz == 1)
					mSingletons.push_back(i);
			}
		}
		
		unsigned recVisit(unsigned p, unsigned g, Cell_t c) {
			if(mVisited[p] != NullCell || mOrigState[p] != c)
				return 0;
			
			mVisited[p] = g;
			
			unsigned s = 1;
			
			if(p >= W)			s += recVisit(p-W, g, c);
			if(p < (H-1)*W)		s += recVisit(p+W, g, c);
			if(p%W != 0)		s += recVisit(p-1, g, c);
			if((p+1)%W != 0)	s += recVisit(p+1, g, c);
			return s;
		}
		
		void recDelete(unsigned p, Cell_t c) {
			slow_assert(c != NullCell);
			if(mAdjState[p] != c)
				return;
			
			mAdjState[p] = NullCell;
			mPoppedCols[p%W] = true;
			
			if(p >= W)			recDelete(p-W, c);
			if(p < (H-1)*W)		recDelete(p+W, c);
			if(p%W != 0)		recDelete(p-1, c);
			if((p+1)%W != 0)	recDelete(p+1, c);
		}
		
		void dropBubbles() {
			for(unsigned i=0; i<W; i++) {
				if(!mPoppedCols[i])
					continue;
				
				unsigned em = NullInt;
				
				for(unsigned j=0; j<H; j++) {
					if(em == NullInt && mAdjState[i+j*W] == NullCell) {
						em = j;
						continue;
					}
					
					if(mAdjState[i+j*W] != NullCell && em != NullInt) {
						mAdjState[i+em*W] = mAdjState[i+j*W];
						mAdjState[i+j*W] = NullCell;
						em++;
					}
				}
			}
		}
		
		void applyMove(unsigned p) {
			mAdjState = mOrigState;
			mPoppedCols.fill(false);
			recDelete(p, mAdjState[p]);
			dropBubbles();
		}
		
		//~ bool shouldPrune() {
			//~ std::array<std::array<bool,Sz>, W> colorPresentMap;
			
			//~ for(auto& a : colorPresentMap) a.fill(false);
			
			//~ for(unsigned i=0; i<H*W; i++) {
				//~ if(mAdjState[i] == NullCell)
					//~ continue;
				//~ unsigned w = i % W;
				//~ colorPresentMap[w][mAdjState[i]] = true;
			//~ }
			
			//~ for(auto s : mSingletons) {
				//~ unsigned w = s % W;
				//~ slow_assert(mAdjState[s] != NullCell);	
				//~ if(colorPresentMap[w][mAdjState[s]]) return false;
				//~ if(w != 0 && colorPresentMap[w-1][mAdjState[s]]) return false;
				//~ if(w != W-1 && colorPresentMap[w+1][mAdjState[s]]) return false;
			//~ }
			
			//~ return true;

			
		//~ }
		
		
		State_t<H,W,Sz> const& mOrigState;
		State_t<H,W,Sz> mAdjState;
		std::array<unsigned, H*W> mVisited;
		std::vector<unsigned> mGroups, mSingletons;
		std::array<bool, W> mPoppedCols;
		unsigned mOp;
	};
	
	
	
	template<unsigned H, unsigned W, unsigned Sz>
	struct BaseDomain {
		
		using Cost = int;
		using State = State_t<H,W,Sz>;
		using PackedState = State_t<H,W,Sz>;
		using AdjEdgeIterator = BaseEdgeIterator<H,W,Sz>;
		
		BaseDomain() = default;
		

		void packState(State const& s, PackedState& pkd) const {
			pkd = s;
		}
		
		void unpackState(State& s, PackedState const& pkd) const {
			s = pkd;
		}
		
		AdjEdgeIterator getAdjEdges(State const& s) const {
			return AdjEdgeIterator(s);
		}
		
		size_t hash(PackedState pPacked) const {
			return pPacked.fnv_hash;
		}
		
		Cost costHeuristic(unsigned pState) const {
			return 0;
		}
		
		Cost distanceHeuristic(unsigned pState) const {
			return 0;
		}
		
		std::pair<Cost, Cost> pairHeuristics(unsigned pState) const {
			return {0,0};

		}
		
		bool checkGoal(State pState) const {
			for(unsigned i=0; i<H*W; i++)
				if(pState[i] != 0)
					return false;
			return true;
		}


		void prettyPrintState(State const& s, std::ostream& out) const {
			s.prettyPrint(out);
		}
	};
	

	template<unsigned H, unsigned W, unsigned Sz>
	struct DomainStack {
		
		static const unsigned Top_Abstract_Level = 0;
		
		template<unsigned L>
		struct Domain : public BaseDomain<H,W,Sz> {
			Domain(DomainStack<H,W,Sz>& pStack) {}
		};
			
		
		
		DomainStack() {
			mInitState.clearBubbles();
		}
		
		void setInitState(unsigned r) {
			mInitState.setRandom(r);
		}
		
		State_t<H,W,Sz> getInitState() const {
			return mInitState;
		}
		
		
		
		
		State_t<H,W,Sz> mInitState;
		
	};
}}
