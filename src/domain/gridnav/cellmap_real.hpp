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


namespace mjon661 { namespace gridnav {

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
					logDebugStream() << "(" << hillpos.back().first << "," << hillpos.back().second << ") " 
						<< hillradius.back() << " " << hilldepth.back() << "\n";
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
		
		void dumpHeatMap1(unsigned height, unsigned width) const {
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

			mathutil::writeImgPPM(vGray, vGray, vGray, height, width, "cellmap_real_1.ppm");
		}
		
		
		void dumpHeatMap2(unsigned height, unsigned width, std::vector<unsigned> const& pPackedStates) const {
			std::vector<double> brightness(mSize);
			std::vector<double> hue(mSize);

			std::fill(hue.begin(), hue.end(), 0);
			std::fill(brightness.begin(), brightness.end(), 0);
			
			for(unsigned i=0; i<pPackedStates.size(); i++) {
				hue[pPackedStates[i]] = (double)i / pPackedStates.size();
				brightness[pPackedStates[i]] = 0.5;
			}
			
			std::vector<std::tuple<double, double, double>> pxls(mSize);
			
			for(unsigned i=0; i<mSize; i++)
				pxls[i] = mathutil::HSLtoRGBcolor<double>(hue[i] * 315, 1, brightness[i]);

			
			std::vector<unsigned char> vR(mSize), vG(mSize), vB(mSize);
			
			for(unsigned i=0; i<mSize; i++) {
				vR[i] = std::get<0>(pxls[i]) * 255;
				vG[i] = std::get<1>(pxls[i]) * 255;
				vB[i] = std::get<2>(pxls[i]) * 255;
			}
			
			unsigned blankBarBegin = vR.size(), colorMapBarBegin = vR.size() + 5 * width, newImgEnd = vR.size() + 10 * width;
		
			vR.resize(newImgEnd);
			vG.resize(newImgEnd);
			vB.resize(newImgEnd);
			pxls.resize(newImgEnd);

			for(unsigned i=blankBarBegin; i<colorMapBarBegin; i++)
				vR[i] = vG[i] = vB[i] = 0;
			
			for(unsigned i=colorMapBarBegin; i<newImgEnd; i++) {
				std::tuple<double, double, double> mappxl = mathutil::HSLtoRGBcolor<double>((double)(i%width)/width * 315, 1, 0.5);
				vR[i] = std::get<0>(mappxl) * 255;
				vG[i] = std::get<1>(mappxl) * 255;
				vB[i] = std::get<2>(mappxl) * 255;
			}
			
			mathutil::writeImgPPM(vR, vG, vB, newImgEnd/width, width, "cellmap_real_2.ppm");
			
		}
		
		
		void dumpHeatMap3(unsigned height, unsigned width, std::vector<unsigned> const& pPackedStates) const {
			std::vector<uint8_t> vGray(mSize);
			
			std::fill(vGray.begin(), vGray.end(), 0);
			
			for(unsigned i=0; i<pPackedStates.size(); i++)
				vGray.at(pPackedStates[i]) = (double)i / pPackedStates.size() * 255;

			mathutil::writeImgPPM(vGray, vGray, vGray, height, width, "cellmap_real_3.ppm");	
		}
		
		
		private:
		

		
		const unsigned mSize;
		std::vector<CellVal_t> mCells;
	};
}}
