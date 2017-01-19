#pragma once

#include <vector>
#include <cmath>
#include "structs/simple_hashmap.hpp"
#include "util/json.hpp"



namespace mjon661 { namespace algorithm { namespace ugsav5 {	


	template<typename = void>
	struct ExpansionStats {

		void reset() {
			mFCounts.clear();
			mDepthCounts.clear();
		}
		
		void informExpansion(unsigned f, unsigned depth) {
			mFCounts[f].val++;
			mDepthCounts[depth].val++;
		}
		
		double computeHBF() {
			return doComputeBF(mFCounts);
		}
		
		double computeDepthBF() {
			return doComputeBF(mDepthCounts);
		}
		
		private:
		
		double doComputeBF(SimpleHashMap<unsigned, unsigned, 10000>& countMap) {
			std::vector<unsigned> const& obsrvdLvls = countMap.orderedKeys();
			double acc = 0;
			
			unsigned f0 = obsrvdLvls[0];
			double lnNf0 = std::log(countMap[f0].val);
		
			for(unsigned i=1; i<obsrvdLvls.size(); i++) {
				unsigned f = obsrvdLvls[i];
				
				double v = (std::log(countMap[f].val) - lnNf0) / (f - f0);
				slow_assert(std::isfinite(v));
				acc += v;
			}
			
			return std::exp(acc / (obsrvdLvls.size()-1));
			
		}
		
		SimpleHashMap<unsigned, unsigned, 10000> mFCounts, mDepthCounts;
	};
}}}
