#pragma once

#include <string>

#include "util/json.hpp"

	

namespace mjon661 { namespace algorithm { namespace hastarv2 {
	template<typename T>
	T getParamOrDefault(Json const& jConfig, std::string const& pKey, T pDef) {
		if(jConfig.count(pKey))
			return static_cast<T>(jConfig.at(pKey));
		else
			return pDef;
	}
	

}}}
