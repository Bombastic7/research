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
#include <cstdint>

#include "util/math.hpp"


namespace mjon661 { namespace gridnav { namespace hypernav_blocked {


	

	template<typename CellVal_t>
	class CellMapReal {
		
		public:

		CellMapReal(unsigned pSize, std::string const& pMapFile) :
			mSize(pSize),
			mCells(mSize)
		{
			fast_assert(pMapFile.size() > 0);
			
			if(pMapFile[0] != ',') {
				std::ifstream ifs(pMapFile);
				
				if(!ifs)
					throw std::runtime_error("Could not open map file");

				for(unsigned i=0; i<mSize; i++) {
					CellVal_t c;
					ifs >> c;
					mCells[i] = c;
				}
				return;
			} 
			
			std::stringstream ss(pMapFile);
			std::string funcStr, t;
			
			std::getline(ss, funcStr, ',');
			std::getline(ss, funcStr, ',');
			
			if(funcStr == "fill") {
				std::fill(mCells.begin(), mCells.end(), 0);
				logDebug("CellMap filled.");
			}
			
			else if(funcStr == "random_norm") {
				unsigned seed;
				double mean, stddev;
				std::getline(ss, t, ',');
				seed = std::stoul(t);
				std::getline(ss, t, ',');
				mean = std::stod(t);
				std::getline(ss, t, ',');
				stddev = std::stod(t);
			

				std::mt19937 gen(5489u + seed);
				std::normal_distribution<double> d(mean, stddev);
			
				for(unsigned i=0; i<mSize; i++) {
					mCells[i] = d(gen);
				}
				
				logDebugStream() << "random_norm init. seed=" << seed << ", mean=" << mean << ", stdev=" << stddev << "\n";
			}
			
			else if(funcStr == "random_hills") {
				unsigned width;
				unsigned seed, nhills;
				double noisemean, noisestddev;
				
				//std::getline(ss, t, ',');
				//height = std::stoul(t);
				std::getline(ss, t, ',');
				width = std::stoul(t);
				std::getline(ss, t, ',');
				seed = std::stoul(t);
				std::getline(ss, t, ',');
				nhills = std::stod(t);
				std::getline(ss, t, ',');
				noisemean = std::stod(t);
				std::getline(ss, t, ',');
				noisestddev = std::stod(t);
				
				const double diagLength = std::hypot(width, mSize/width);
				
				std::vector<std::pair<unsigned, unsigned>> hillpos;
				std::vector<double> hillradius;
				std::vector<double> hilldepth;
				
				std::mt19937 randgen(5489u + seed);
				std::uniform_int_distribution<unsigned> hillposdist(0, mSize);
				std::uniform_real_distribution<double> hillradiusdist(0, diagLength/4);
				std::uniform_real_distribution<double> hilldepthdist(0, diagLength/4);

				std::uniform_real_distribution<double> noisedist(noisemean, noisestddev);
				
				for(unsigned i=0; i<nhills; i++) {
					unsigned pkd = hillposdist(randgen);
					unsigned x = pkd % width, y = pkd / width;
					hillpos.push_back({x,y});
					hillradius.push_back(hillradiusdist(randgen));
					hilldepth.push_back(hilldepthdist(randgen));
				}
				
				std::fill(mCells.begin(), mCells.end(), 0);
				
				for(unsigned i=0; i<nhills; i++) {
					double r2 = std::pow(hillradius[i], 2);
					
					for(unsigned pkd=0; pkd<mSize; pkd++) {
						double tgtx = pkd % width, tgty = pkd / width;
						double d = std::hypot(tgtx-hillpos[i].first, tgty-hillpos[i].second);
						
						if(d > hillradius[i])
							continue;

						mCells[pkd] += mathutil::max(0.0, std::sqrt(r2 - std::pow(d, 2)) - hilldepth[i]);
						slow_assert(std::isfinite(mCells[pkd]));
						//std::cout << mCells[pkd] << "\n";
					}
				}
				
				logDebugStream() << "random_hills init. seed=" << seed << ", nhills=" << nhills << ", noisemean="
					<< noisemean << ", noisestddev=" << noisestddev << "\n";
			}
			
			else
				gen_assert(false);
		}
		
		
		std::vector<CellVal_t> const& cells() const {
			return mCells;
		}
		
		unsigned getSize() const {
			return mSize;
		}
		
		void dumpHeatMap(unsigned height, unsigned width) const {
			CellVal_t minval, maxval;
			minval = std::numeric_limits<CellVal_t>::max();
			maxval = std::numeric_limits<CellVal_t>::min();
			
			for(auto& i : mCells) {
				if(i < minval) minval = i;
				if(i > maxval) maxval = i;
			}
			
			double range = maxval - minval;
			
			std::vector<uint8_t> vGray(mSize);
			
			for(unsigned i=0; i<mSize; i++) {
				double v = (mCells[i] - minval) / range * 255.0;
				slow_assert(v >= 0 && v <= 255);
				vGray[i] = v;
			}

			mathutil::writeImgPPM(vGray, vGray, vGray, height, width, "cellmap_real.ppm");
		}
		
		private:
		

		
		const unsigned mSize;
		std::vector<CellVal_t> mCells;
	};
}}}
