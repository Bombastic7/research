#pragma once

#include <string>

#include "util/debug.hpp"


namespace mjon661 { namespace experiment {

	struct UtilityWeights {
		const double wf, wt;
		const std::string str;
		
		UtilityWeights(double pWf, double pWt, std::string const& pStr) :
			wf(pWf), wt(pWt), str(pStr)
		{
			fast_assert(wf >= 0 && wt >= 0 && str != "");
		}
	};
}}
