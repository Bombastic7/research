#pragma once

#include "util/json.hpp"


namespace mjon661 {
		
	double setCpuTimeLimit(double pSeconds);
	double setVirtMemLimit(double pMB);
	Json resourceReport();
}
