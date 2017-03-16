#pragma once

#include <stdexcept>
#include <string>

namespace mjon661 {
	class ConfigException : public std::runtime_error {
		using runtime_error::runtime_error;
	};
	
	class ResourceLimitException : public std::runtime_error {
		using runtime_error::runtime_error;
	};
	
	class NoSolutionException : public std::runtime_error {
		using runtime_error::runtime_error;
	};
}
