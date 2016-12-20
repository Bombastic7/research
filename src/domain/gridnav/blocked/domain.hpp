#pragma once

#include <array>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

#include "domain/gridnav/defs.hpp"
#include "domain/gridnav/blocked/maps.hpp"

#include "util/json.hpp"
#include "util/exception.hpp"
#include "util/debug.hpp"
#include "util/math.hpp"

#include <string>
#include <sstream>


namespace mjon661 { namespace gridnav { namespace blocked {
	
	
	
	/*
	 * GridNav domain implementations for 4-way and 8-way versions.
	 * 
	 * flatlayout refers to the underlying CellArray (that GridNav_Dom is given a reference to) is
	 * a flat, row major array of cell_ts. Each position (cell) has at most 4 / 8 moves.
	 * 
	 * FourWayBase and EightWayBase provide specific details to specialise GridNav_Dom into a suitable domain class.
	 * 
	 * index(x, y) = y * Width + x
	 * 
	 * North / South = -y / +y
	 * 
	 * West / East = -x / +x
	 */
	
	
	enum struct MoveDir {
		N, NE, E, SE, S, SW, W, NW, NoOp
	};
	
	template<typename = void>
	MoveDir oppositeDir(MoveDir pDir) {	
		int i = (int)pDir;
		slow_assert(i >= 0 && i < 8);
		i += 4;
		i %= 8;
		return (MoveDir)i;
	}
	
	template<typename = void>
	bool isDiagDir(MoveDir pDir) {
		
		return (int)pDir % 2 == 1;
	}
		
	template<unsigned H, unsigned W>
	bool tryMoveDir(idx_t pPos, MoveDir pDir) {

		if(pDir == MoveDir::N && pPos >= W) ;
		
		else if(pDir == MoveDir::S && pPos < (H-1)*W) ;
			
		else if(pDir == MoveDir::W && pPos % W != 0) ;
		
		else if(pDir == MoveDir::E && (pPos+1) % W != 0) ;
		
		else if(pDir == MoveDir::NE && tryMoveDir<H,W>(pPos, MoveDir::N) && tryMoveDir<H,W>(pPos, MoveDir::E)) ;
			
		else if(pDir == MoveDir::SE && tryMoveDir<H,W>(pPos, MoveDir::S) && tryMoveDir<H,W>(pPos, MoveDir::E)) ;
		
		else if(pDir == MoveDir::NW && tryMoveDir<H,W>(pPos, MoveDir::N) && tryMoveDir<H,W>(pPos, MoveDir::W)) ;
			
		else if(pDir == MoveDir::SW && tryMoveDir<H,W>(pPos, MoveDir::S) && tryMoveDir<H,W>(pPos, MoveDir::W)) ;
		
		else
			return false;
		
		return true;
	}
	
	template<unsigned H, unsigned W>
	idx_t applyMove(idx_t pPos, MoveDir pDir) {
		
		if(pDir == MoveDir::N)
			return pPos-W;
		
		else if(pDir == MoveDir::S)
			return pPos+W;
			
		else if(pDir == MoveDir::W)
			return pPos-1;
		
		else if(pDir == MoveDir::E)
			return pPos+1;
		
		else if(pDir == MoveDir::NE)
			return applyMove<H,W>( applyMove<H,W>(pPos, MoveDir::N) , MoveDir::E);

		else if(pDir == MoveDir::SE)
			return applyMove<H,W>( applyMove<H,W>(pPos, MoveDir::S) , MoveDir::E);
		
		else if(pDir == MoveDir::NW)
			return applyMove<H,W>( applyMove<H,W>(pPos, MoveDir::N) , MoveDir::W);
			
		else if(pDir == MoveDir::SW)
			return applyMove<H,W>( applyMove<H,W>(pPos, MoveDir::S) , MoveDir::W);
		
		fast_assert(false);
		
		return pPos;
	}
	
	


	template<typename = void>
	const char* prettyPrintDir(MoveDir pDir) {
		if(pDir == MoveDir::N)
			return "(North) ";
		
		else if(pDir == MoveDir::S)
			return "(South) ";
			
		else if(pDir == MoveDir::W)
			return "(West) ";
		
		else if(pDir == MoveDir::E)
			return "(East) ";
		
		else if(pDir == MoveDir::NE)
			return "(North East) ";

		else if(pDir == MoveDir::SE)
			return "(South East) ";
		
		else if(pDir == MoveDir::NW)
			return "(North West) ";
			
		else if(pDir == MoveDir::SW)
			return "(South West) ";
		
		fast_assert(false);
		return nullptr;
	}
	
	/*
	template<unsigned H, unsigned W>
	void drawFlatCellArray( CellArray<H,W> const& 			pCells,
							std::vector<const char*> const& pSymbols, 
							std::vector<idx_t> const& 		pSpecialPositions,
							const char* 					pSpecialSymbol,
							std::ostream& 					out
						  )
	{
		for(idx_t i=0; i<H*W; i++) {
			
			if(contains(pSpecialPositions, i))
				out << pSpecialSymbol << " ";
			
			else
				out << pSymbols.at(pCells[i]) << " ";
			
			if((i+1) % W == 0)
				out << "\n";
		}
	}
	*/


	template<unsigned H, unsigned W>
	struct FourWayMoves {
		
		FourWayMoves(idx_t pPos) :
			mMoves{},
			mN(0)
		{
			
			prepMove(pPos, MoveDir::N);
			prepMove(pPos, MoveDir::S);
			prepMove(pPos, MoveDir::E);
			prepMove(pPos, MoveDir::W);
		}			
		

		std::array<MoveDir, 4> mMoves;
		int mN;
		
		private:
		void prepMove(idx_t pPos, MoveDir pDir) {
			if(tryMoveDir<H,W>(pPos, pDir))
				mMoves[mN++] = pDir;
		}
	};
	
	
	template<unsigned H, unsigned W>
	struct EightWayMoves {

		EightWayMoves(idx_t pPos) :
			mMoves{},
			mN(0)
		{
				
			prepMove(pPos, MoveDir::N);
			prepMove(pPos, MoveDir::S);
			prepMove(pPos, MoveDir::E);
			prepMove(pPos, MoveDir::W);
			prepMove(pPos, MoveDir::NE);
			prepMove(pPos, MoveDir::NW);
			prepMove(pPos, MoveDir::SE);
			prepMove(pPos, MoveDir::SW);
		}
			

		std::array<MoveDir, 8> mMoves;
		int mN;
		
		private:
		void prepMove(idx_t pPos, MoveDir pDir) {
			if(tryMoveDir<H,W>(pPos, pDir))
				mMoves[mN++] = pDir;
		}
	};
	
	
	
	/* returns  sum of contiguous rows from pY to {mGoaly-1 / mGoaly+1} if {pY < mGoalY / pY > mGoalY} */
	template<typename = void>
	int verticalPathFactor(int pY, int goaly) {
		int d = std::abs(goaly - pY);

		if(d == 0)
			return 0;

		int s = (d * (d-1)) / 2;
		
		s += pY < goaly ? 
				 pY * d :
			(goaly+1) * d;

		return s;
	}
	
	
	template<bool H, typename Cost>
	struct StateImpl {
		idx_t pos;
		Cost h, d;
		void set_h(Cost ph) {h=ph;}
		void set_d(Cost pd) {d=pd;}
		Cost get_h() const {return h;}
		Cost get_d() const {return d;}
	};
	
	template<typename Cost>
	struct StateImpl<false, Cost> {
		idx_t pos;
		void set_h(Cost) {}
		void set_d(Cost) {}
		Cost get_h() const {return 0;}
		Cost get_d() const {return 0;}
	};

	
	
	
	template<unsigned H, unsigned W, bool Use_LC, bool Use_H>
	struct FourWayBase {
		
		using cost_t = int;
		using OpSetBase = FourWayMoves<H,W>;
		using state_t = StateImpl<Use_H, cost_t>;
		
		static const unsigned Height = H, Width = W;
		
		
		FourWayBase(idx_t pGoal) :
			mGoalx(pGoal % W),
			mGoaly(pGoal / W)
		{}
		
		cost_t getMoveCost(idx_t pPos, MoveDir pDir) const {
			return Use_LC ? pPos / W : 1;
		}
		
		void getHeuristicValues(idx_t pPos, state_t& pState) const {
			if(!Use_H)
				return;
			
			else if(!Use_LC) {
				cost_t c = manhat(pPos);
				pState.set_h(c);
				pState.set_d(c);
			}
			
			else {
				cost_t h, d;
				lifeCostHeuristics(pPos, h, d);
				pState.set_h(h);
				pState.set_d(d);
			}
		}
		
		
		private:
		
		int manhat(idx_t pState) const {
			int x = pState % W, y = pState / W;
			return std::abs(mGoalx - x) + std::abs(mGoaly - y);
		}
		
		void lifeCostHeuristics(idx_t pPos, int& out_h, int& out_d) const {
			int x = pPos % W, y = pPos / W;
			
			int dx = std::abs(x-mGoalx), miny = min(y, mGoaly);
			
			// Horizontal segment at the cheaper of y/gy. Vertical segment straight from y to goaly.
			int p1 = dx * miny + verticalPathFactor(y, mGoaly);
			
			// From (x,y) to (x,0), then to (gx, 0), then to (gx, gy). Note that horizontal segment is free (row 0).
			int p2 = verticalPathFactor(y, 0) + verticalPathFactor(0, mGoaly);
			
			if(p1 < p2) {
				out_h = p1;
				out_d = dx + std::abs(y - mGoaly);
			} else {
				out_h = p2;
				out_d = dx + y + mGoaly;
			}
		}

		const int mGoalx, mGoaly;
		
	};
	
	
	
	
	/**
	 * Use_LC ? Unit cost : life cost
	 * Use_H ?  Manhattan distance based heuristics : none
	 */
	
	template<unsigned H, unsigned W, bool Use_LC, bool Use_H>
	struct EightWayBase {
		
		using cost_t = float;
		using OpSetBase = EightWayMoves<H,W>;
		using state_t = StateImpl<Use_H, cost_t>;
		
		static const unsigned Height = H, Width = W;
		
		
		EightWayBase(idx_t pGoal) :
			mGoalx(pGoal % W),
			mGoaly(pGoal / W)
		{}
		
		cost_t getMoveCost(idx_t pPos, MoveDir pDir) const {
			float c = isDiagDir(pDir) ? SQRT2 : 1;
			
			if(Use_LC)
				c *= pPos / W;
			return c;
		}
		
		void getHeuristicValues(idx_t pPos, state_t& pState) const {
			if(!Use_H)
				return;
			
			else if(!Use_LC) {
				int dx = std::abs(pPos % W - mGoalx), dy = std::abs(pPos / W - mGoaly);
				
				pState.set_h(std::abs(dx-dy) + min(dx, dy) * SQRT2);
				pState.set_d(max(dx, dy));
			}
			
			else {
				cost_t h, d;
				lifeCostHeuristics(pPos, h, d);
				pState.set_h(h);
				pState.set_d(d);
			}
		}
		

		private:
		

		void lifeCostHeuristics(idx_t pPos, cost_t& out_h, cost_t& out_d) const {
			int x = pPos % W, y = pPos / W;
			
			int dx = std::abs(x - mGoalx);
			int dy = std::abs(x - mGoaly);
			
			if(dx <= dy) {
				out_h = verticalPathFactor(pPos, mGoaly);
				out_d = dy;
				return;
			}
			
			//int maxdown = min(y, mGoaly);
			int extra = dx - dy;
			
			int down = min(min(y, mGoaly), (dx-dy)/2);
			int botRow = min(y, mGoaly) - down;
			int across = extra - 2*down;
			
			out_h = verticalPathFactor(y, botRow) + across * botRow + verticalPathFactor(botRow, mGoaly);
			out_d = dx;
		}
		
		const int mGoalx, mGoaly;
	};
	
	
	
	template<bool Use_EightWay, bool Use_LifeCost, bool Use_H>
	struct GridNavBase {
		
		template<unsigned H, unsigned W>
		using type = FourWayBase<H, W, Use_LifeCost, Use_H>;
	};
	
	
	template<bool Use_LifeCost, bool Use_H>
	struct GridNavBase<true, Use_LifeCost, Use_H> {
		
		template<unsigned H, unsigned W>
		using type = EightWayBase<H, W, Use_LifeCost, Use_H>;
	};
	
	
	
	
	/*
	 * GridNav_Dom is a top level class implementing domain functionality.
	 * It handles OperatorSet and Edge creation specifically; everything is done with help from DomBase.
	 * 
	 */
	
	
	/*
	 * DomBase<H, W> provides:
	 * 
	 * 	type cost_t
	 * 	
	 * 	struct state_t {
	 * 		cost_t get_h();	//Returns 0 if not using heuristics.
	 * 		cost_t get_d();	//same as above.
	 * 
	 * 		idx_t pos;
	 * 	}
	 *  
	 * 	struct OpSetBase {
	 * 		OpSetBase(idx_t pos);  //populates mMoves for position, assuming all neighbouring tiles can be moved to.
	 * 		MoveDir mMoves[]
	 * 		unsigned mN      //Number of moves
	 * 	}
	 * 
	 * 	cost_t getMoveCost(idx_t pos, MoveDir dir);
	 * 
	 * 	void getHeuristicValues(idx_t pos, state_t& pState);	//Called during State creation to initialise h/d.
	 * 
	 * 	DomBase(idx_t pGoalState);
	 */
	template<unsigned Height, unsigned Width, template<unsigned, unsigned> class DB>
	struct GridNav_Dom : public DB<Height, Width> {
		
		using DomBase = DB<Height, Width>;
		using PackedState = idx_t;
		using Cost = typename DomBase::cost_t;
		using Operator = MoveDir;


		static const size_t Hash_Range = Height * Width;
		

		Operator noOp;

		using State = typename DomBase::state_t;


		
		struct OperatorSet : public DomBase::OpSetBase {
			
			unsigned size() {
				return this->mN;
			}
			
			Operator operator[](unsigned i) {
				return this->mMoves[i];
			}
			
			OperatorSet(GridNav_Map const& pMap, idx_t pState) :
				DomBase::OpSetBase(pState)
				//mTestCopy(*this)//...........
			{
				int canMove = 0;
				
				for(int pos=0, i=0; i<this->mN; i++) {
					
					idx_t dest = applyMove<Height, Width>(pState, this->mMoves[i]);
					
					if(pMap[dest] != 0)
						continue;
				
					this->mMoves[pos] = this->mMoves[i];
					canMove++;
					pos++;
				}
				
				this->mN = canMove;
				//slow_assert(assertCheck(pCells, pState));
			}
			
			private:
			/*
			bool assertCheck(GridNavMap_OpOrBl<Height,Width> const& pCells, idx_t pState) {
				for(int i=0; i<mTestCopy.mN; i++) {
					
					cell_t c = pCells[applyMove<Height, Width>(pState, mTestCopy.mMoves[i])];
					
					std::stringstream ss;
					
					ss << "\n" << prettyPrintDir(this->mMoves[i]) << " " << applyMove<Height, Width>(pState, mTestCopy.mMoves[i]) << 
					" " << (int)c << "\n" << " " << pState << " " <<
					 (pState % Width) << " " << (pState/Width) << "\n" << std::to_string(this->mN) << " ";
					 
					for(int j=0; j<this->mN; j++)
						ss << prettyPrintDir(this->mMoves[j]) << " ";
					
					ss << "\n" << std::to_string(mTestCopy.mN) << " ";
					
						for(int j=0; j<mTestCopy.mN; j++)
							ss << prettyPrintDir(mTestCopy.mMoves[j]) << " ";
							
					std::string msg(ss.str());
					
					if(c == 0)
						slow_assert(contains(this->mMoves, mTestCopy.mMoves[i], this->mN), "%s", msg.c_str());
					else if(c == 1)
						slow_assert(!contains(this->mMoves, mTestCopy.mMoves[i], this->mN), "%s", msg.c_str());
					else
						slow_assert(false, "%d", c);
				}
				return true;
			}
			*/
			
			//const typename DomBase::OpSetBase mTestCopy; //...........
			
		};
		
		
		struct Edge {
			
			Edge(State pState, Cost pCost, Operator pParentOp) :
				mState(pState),
				mCost(pCost),
				mParentOp(pParentOp)
			{}
			
			State& state() {
				return mState;
			}
			
			Cost cost() {
				return mCost;
			}
			
			Operator parentOp() {
				return mParentOp;
			}
			
			State mState;
			Cost mCost;
			Operator mParentOp;			
		};
		

			
		
			
		GridNav_Dom(GridNav_Map const& pMap, idx_t pInitPos, idx_t pGoalPos) :
			DomBase(pGoalPos),
			noOp(MoveDir::NoOp),
			mMap(pMap),
			mInitPos(pInitPos),
			mGoalPos(pGoalPos)
		{
			fast_assert(mInitPos >= 0 && mInitPos < Height * Width);
			fast_assert(mGoalPos >= 0 && mGoalPos < Height * Width);
			fast_assert(pMap.getHeight() == Height);
			fast_assert(pMap.getWidth() == Width);
		}
		
		State createState() const {
			State s;
			s.pos = mInitPos;
			this->getHeuristicValues(mInitPos, s);
			
			return s;
		}
			
		void packState(State const& pState, PackedState& pPacked) const {
			pPacked = pState.pos;
			slow_assert(pPacked >= 0 && pPacked < Height*Width);
		}
		
		void unpackState(State& pState, PackedState const& pPacked) const {
			pState.pos = pPacked;
			slow_assert(pState.pos >= 0 && pState.pos < Height*Width);
			this->getHeuristicValues(pPacked, pState);
		}
		
		Edge createEdge(State& pState, Operator op) const {			
			State edgeState;
			edgeState.pos = applyMove<Height, Width>(pState.pos, op);
			
			this->getHeuristicValues(pState.pos, pState);
			
			Cost edgeCost = this->getMoveCost(pState.pos, op);
			
			return Edge(edgeState, edgeCost, oppositeDir(op));
		}
		
		void destroyEdge(Edge&) const {
		}
		
		OperatorSet createOperatorSet(State const& pState) const {
			return OperatorSet(mMap, pState.pos);
		}
		
		size_t hash(PackedState const& pPacked) const {
			return pPacked;
		}
		
		Cost heuristicValue(State const& pState) const {
			return pState.get_h();
		}
		
		Cost distanceValue(State const& pState) const {
			return pState.get_d();
		}
		
		bool checkGoal(State const& pState) const {
			return pState.pos == mGoalPos;
		}

		bool compare(State const& a, State const& b) const {
			return a.pos == b.pos;
		}

		bool compare(PackedState const& a, PackedState const& b) const {
			return a == b;
		}
		
		void prettyPrint(State const& s, std::ostream& out) const {
			out << "( " << s.pos % Width << ", " << s.pos / Width << " )\n";
		}
		
		void prettyPrint(Operator const& op, std::ostream &out) const {
			out << prettyPrintDir(op) << "\n";
		}
		

		private:
		GridNav_Map const& mMap;
		const idx_t mInitPos, mGoalPos;

	};
}}}
