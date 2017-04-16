#pragma once

#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include "util/debug.hpp"


namespace mjon661 { namespace gridnav { namespace dim2 {


	inline unsigned manhat(unsigned pState, unsigned pGoal, unsigned pWidth) {
		int x = pState % pWidth, y = pState / pWidth;
		int gx = pGoal % pWidth, gy = pGoal / pWidth;
		return std::abs(gx - x) + std::abs(gy - y);
	}

	/* returns  sum of contiguous rows from pY to {mGoaly-1 / mGoaly+1} if {pY < mGoalY / pY > mGoalY} */
	inline unsigned verticalPathFactor(int pY, int goaly) {
		int d = std::abs(goaly - pY);

		if(d == 0)
			return 0;

		int s = (d * (d-1)) / 2;
		
		s += pY < goaly ? 
				 pY * d :
			(goaly+1) * d;

		return s;
	}
	
	
	
	template<typename = void>
	class CellMap2D {
		public:
		
		enum struct Cell_t {
			Open, Blocked, Null
		};
		
		CellMap2D() : mHeight(0), mWidth(0) {}

		void loadFile(std::string const& pMapFile) {
			std::ifstream ifs(pMapFile);
				
			if(!ifs)
				throw std::runtime_error("Could not open map file");
			
			ifs >> mHeight >> mWidth;
			mCells.resize(mHeight*mWidth);
			
			for(unsigned i=0; i<mCells.size(); i++) {
				int v;
				Cell_t c;
				ifs >> v;
				c = (Cell_t)v;
				
				gen_assert(c == Cell_t::Open || c == Cell_t::Blocked);
				mCells[i] = c;
			}
		}
		
		
		void dumpFile(std::string const& pMapFile) const {
			std::ofstream out(pMapFile);
				
			if(!out)
				throw std::runtime_error("Could not open map file");
			
			out << mHeight << " " << mWidth << " ";
			
			for(unsigned i=0; i<mCells.size(); i++)
				out << mCells[i] << " ";
		}

		void setRandom(unsigned pHeight, unsigned pWidth, unsigned pSeed, double pBlockedProb) {
			mHeight = pHeight;
			mWidth = pWidth;
			
			mCells.resize(mHeight * mWidth);
			
			std::mt19937 gen(pSeed);
			std::uniform_real_distribution<> dist(0,1);
			
			for(unsigned i=0; i<mCells.size(); i++)
				mCells[i] = dist(gen) < pBlockedProb ? Cell_t::Blocked : Cell_t::Open;
		}
		
		unsigned getHeight() const { return mHeight;}
		unsigned getWidth() const { return mWidth;}
		
		void drawCells(std::ostream& out) const {
			for(unsigned h=0; h<mHeight(); h++) {
				for(unsigned w=0; w<mWidth(); w++) {
					if(mCells[h*mWidth() + w])
						out << ".";
					else
						out << "0";
					if((w+1) % 5 == 0)
						out << " ";
				}
				out << "\n";
				if((h+1) % 5 == 0)
					out << "\n";
			}
		}
		
		private:
		unsigned mHeight, mWidth;
		std::vector<Cell_t> mCells;
	};
	
	
}}}
