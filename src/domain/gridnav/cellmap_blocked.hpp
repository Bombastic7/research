#pragma once


#include <array>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <random>
#include <cmath>
#include <random>
#include <utility>
#include <tuple>
#include <algorithm>

#include "util/math.hpp"


namespace mjon661 { namespace gridnav {

	template<typename = void>
	class CellMapBlocked {
		
		public:
		
		enum struct Cell_t {
			Open, Blocked, Null
		};


		CellMapBlocked() = default;
		
		
		void loadFile(std::string const& pMapFile) {
			std::ifstream ifs(pMapFile);
				
			if(!ifs)
				throw std::runtime_error("Could not open map file");
			
			unsigned sz;
			ifs >> sz;
			mCells.resize(sz);
			
			for(unsigned i=0; i<mCells.size(); i++) {
				int v;
				Cell_t c;
				ifs >> v;
				c = (Cell_t)v;
				
				gen_assert(c == Cell_t::Open || c == Cell_t::Blocked);
				mCells[i] = c;
			}
		}
		
		
		void dumpFile() const {
			std::ofstream out(pMapFile);
				
			if(!out)
				throw std::runtime_error("Could not open map file");
			
			out << mSize << " ";
			
			for(unsigned i=0; i<mSize; i++)
				out << mCells[i] << " ";
		}
		
		
		void size() const {
			return mCells.size();
		}
		
		
		std::vector<Cell_t> const& cells() const {
			return mCells;
		}
		
		protected:
		std::vector<Cell_t> mCells;
	};
		
		
		

	
		//~ class OpenCellIterator {
			//~ public:
			
			//~ OpenCellIterator& operator++() {
				//~ if(mIdx == mInst.size())
					//~ return *this;
				//~ ++mIdx;
				//~ while(true) {
					//~ bool v = mIdx < mInst.size();
					//~ if(!v)
						//~ break;
					
					//~ bool o = mInst.isOpen(mIdx);
					//~ if(o)
						//~ break;
					//~ ++mIdx;
				//~ }
				//~ /*
				//~ do {
					//~ ++mIdx;
				//~ } while(mIdx < mInst.getSize() && !mInst.isOpen(mIdx));
				//~ */
				//~ return *this;
			//~ }
			
			//~ bool operator==(OpenCellIterator const& o) {
				//~ return mIdx == o.mIdx;
			//~ }
			
			//~ bool operator!=(OpenCellIterator const& o) {
				//~ return mIdx != o.mIdx;
			//~ }
			
			//~ unsigned operator*() {
				//~ return mIdx;
			//~ }
			
			//~ private:
			//~ friend CellMapBlocked<void>;
			
			//~ OpenCellIterator(CellMapBlocked<void> const& pInst, bool pAtEnd) :
				//~ mInst(pInst),
				//~ mIdx(0)
			//~ {
				//~ slow_assert(pInst.mCells.size() == pInst.size());
				//~ if(pAtEnd)
					//~ mIdx = mInst.size();
				//~ else
					//~ while(mIdx < mInst.size() && !mInst.isOpen(mIdx))
						//~ ++mIdx;
			//~ }
			
			//~ CellMapBlocked<void> const& mInst;
			//~ unsigned mIdx;
		//~ };
		
		
		//~ CellMapBlocked(unsigned pSize, ) :
			//~ mSize(pSize),
			//~ mCells(mSize)
		//~ {
			//~ fast_assert(pMapFile.size() > 0);
			
			//~ if(pMapFile[0] != ',') {
				//~ std::ifstream ifs(pMapFile);
				
				//~ if(!ifs)
					//~ throw std::runtime_error("Could not open map file");

				//~ for(unsigned i=0; i<mSize; i++) {
					//~ int v;
					//~ Cell_t c;
					//~ ifs >> v;
					//~ c = (Cell_t)v;
					
					//~ gen_assert(c == Cell_t::Open || c == Cell_t::Blocked);
					//~ mCells[i] = c;
				//~ }
				//~ return;
			//~ } 
			
			//~ std::stringstream ss(pMapFile);
			//~ std::string funcStr, t;
			
			//~ std::getline(ss, funcStr, ',');
			//~ std::getline(ss, funcStr, ',');
			
			//~ if(funcStr == "fill") {
				//~ std::fill(mCells.begin(), mCells.end(), Cell_t::Open);
				//~ logDebug("CellMap set to all open.");
			//~ }
			
			//~ else if(funcStr == "random") {
				//~ unsigned seed;
				//~ double prob;
				//~ std::getline(ss, t, ',');
				//~ seed = std::stoul(t);
				//~ std::getline(ss, t, ',');
				//~ prob = std::stod(t);
			
				//~ fast_assert(prob >= 0 && prob <= 1);
				
				//~ initRandomMap(seed, prob);
				//~ logDebugStream() << "Random CellMap init. seed=" << seed << ", blockedprob=" << prob << "\n";
			//~ }
			
			//~ else if(funcStr == "portals") {
				//~ std::getline(ss, t, ',');
				//~ unsigned height = std::stoul(t);
				//~ std::getline(ss, t, ',');
				//~ unsigned width = std::stoul(t);
				//~ std::getline(ss, t, ',');
				//~ unsigned wallInterval = std::stoul(t);

				//~ fast_assert(mSize == height*width);
				
				//~ std::fill(mCells.begin(), mCells.end(), Cell_t::Open);

				//~ std::mt19937 randgen;
				//~ std::uniform_int_distribution<unsigned> ud(0, wallInterval-2);

				//~ for(unsigned x = wallInterval-1; x<width; x+=wallInterval) {
					//~ for(unsigned y=0; y<height; y++)
						//~ mCells.at(x+width*y) = Cell_t::Blocked;
					
					//~ for(unsigned y=0; y<height; y+=wallInterval) {
						//~ unsigned portal_y = y + (ud(randgen) % (height-y));
					
						//~ mCells.at(x+width*portal_y) = Cell_t::Open;
					//~ }
				//~ }
				
				//~ for(unsigned y = wallInterval-1; y<height; y+=wallInterval) {
					//~ for(unsigned x=0; x<width; x++)
						//~ mCells.at(x+width*y) = Cell_t::Blocked;
					
					//~ for(unsigned x=0; x<width; x+=wallInterval) {
						//~ unsigned portal_x = x + (ud(randgen) % (width-x));
					
						//~ mCells.at(portal_x+width*y) = Cell_t::Open;
					//~ }
				//~ }
				
				//~ logDebugStream() << "Portal CellMap init. h=" << height << ", w=" << width << ", wallInterval=" << wallInterval << "\n";
				
			//~ }
			//~ else if(funcStr == "portalsSpanningTree") {
				//~ std::getline(ss, t, ',');
				//~ unsigned height = std::stoul(t);
				//~ std::getline(ss, t, ',');
				//~ unsigned width = std::stoul(t);
				//~ std::getline(ss, t, ',');
				//~ unsigned wallInterval = std::stoul(t);

				//~ fast_assert(mSize == height*width);
				
				//~ std::fill(mCells.begin(), mCells.end(), Cell_t::Open);
				
				//~ for(unsigned x = wallInterval-1; x<width; x+=wallInterval) {
					//~ for(unsigned y=0; y<height; y++)
						//~ mCells.at(x+width*y) = Cell_t::Blocked;
				//~ }
				
				//~ for(unsigned y = wallInterval-1; y<height; y+=wallInterval) {
					//~ for(unsigned x=0; x<width; x++)
						//~ mCells.at(x+width*y) = Cell_t::Blocked;
				//~ }
				
				//~ std::mt19937 randgen;
				//~ std::uniform_int_distribution<unsigned> ud(0, wallInterval-2);
				//~ std::uniform_real_distribution<> edgeCostDist(0, 1);
				
				
				
				
				//~ unsigned spacesX = width/(wallInterval), spacesY = height/(wallInterval);
				//~ if(width % wallInterval != 0)
					//~ spacesX++;
				//~ if(height % wallInterval != 0)
					//~ spacesY++;
					
				//~ logDebug(std::string("spacesX: ") + std::to_string(spacesX) + " spacesY: " + std::to_string(spacesY));
				
				
				//~ using Edge_t = std::tuple<unsigned, unsigned, double>;
				//~ std::vector<Edge_t> spaceEdges;
				
				//~ for(unsigned i=0; i<spacesX*spacesY; i++) {
					//~ unsigned x = i % spacesX, y = i / spacesY;
					
					//~ if(y > 0)
						//~ spaceEdges.push_back(Edge_t(i, i-spacesX, edgeCostDist(randgen)));;
					//~ if(y < spacesY-1)
						//~ spaceEdges.push_back(Edge_t(i, i+spacesX, edgeCostDist(randgen)));
					//~ if(x != 0)
						//~ spaceEdges.push_back(Edge_t(i, i-1, edgeCostDist(randgen)));
					//~ if(x < spacesX-1)
						//~ spaceEdges.push_back(Edge_t(i, i+1, edgeCostDist(randgen)));
				//~ }
				
				//~ std::shuffle(spaceEdges.begin(), spaceEdges.end(), randgen);
			
				
				//~ std::vector<std::tuple<unsigned, unsigned, double>> stedges = mathutil::minSpanningTreePrims<double>(spacesX*spacesY, spaceEdges);
				

				//~ for(auto& e : stedges) {
					
					//~ unsigned src = std::get<0>(e);
					//~ unsigned dst = std::get<1>(e);
					
					//~ unsigned x = src % spacesX;
					//~ unsigned y = src / spacesX;
					//~ unsigned px = dst % spacesX;
					//~ unsigned py = dst / spacesX;
					
					//~ unsigned wally, wallx;
					//~ wally = wallx = (unsigned)-1;
					
					//~ if(y == py - 1 || y == py + 1) {
						//~ do {
							//~ wallx = x*wallInterval + ud(randgen);
						//~ } while(wallx >= width);
					//~ }
					
					//~ if(x == px - 1 || x == px + 1) {
						//~ do {
							//~ wally = y*wallInterval + ud(randgen);
						//~ } while(wally >= height);
					//~ }
					
					//~ if(y == py - 1) wally = py*wallInterval-1;
					//~ if(y == py + 1) wally = y*wallInterval-1;
					//~ if(x == px - 1) wallx = px*wallInterval-1;
					//~ if(x == px + 1) wallx = x*wallInterval-1;

					//~ mCells.at(wallx + wally*width) = Cell_t::Open;
				//~ }

				//~ logDebugStream() << "PortalMST CellMap init. h=" << height << ", w=" << width << ", wallInterval=" << wallInterval << "\n";
			//~ }
			//~ else
				//~ gen_assert(false);
		//~ }
		
		
		//~ std::vector<Cell_t> const& cells() const {
			//~ return mCells;
		//~ }
		
		//~ unsigned size() const {
			//~ return mSize;
		//~ }
		
		//~ bool isOpen(unsigned i) const {
			//~ slow_assert(i < mSize, "%u %u", i, mSize);
			//~ return cells()[i] == Cell_t::Open;
		//~ }
		
		//~ OpenCellIterator begin() const {
			//~ return OpenCellIterator(*this, false);
		//~ }
		
		//~ OpenCellIterator end() const {
			//~ return OpenCellIterator(*this, true);
		//~ }
		
		//~ private:
		
		//~ void initRandomMap(unsigned seed, double prob) {
			//~ fast_assert(prob >= 0 && prob <= 1);
			
			//~ std::mt19937 gen(5489u + seed);
			//~ std::uniform_real_distribution<double> d(0.0,1.0);
			
			//~ for(unsigned i=0; i<mSize; i++) {
				//~ mCells[i] = d(gen) < prob ? Cell_t::Blocked : Cell_t::Open;
			//~ }
		//~ }
		
		//~ const unsigned mSize;
		//~ std::vector<Cell_t> mCells;
	//~ };
}}
