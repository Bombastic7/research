"""
Prints generated C++ code to stdout.

Factorial<N, K>
	Represents N!/(N-K)!

	type : unsigned int type big enough to hold value.
	getValue() : static, returns value. constexpr too if type is built-in.


"""

import math


def dataType(val):
	if val < 2**8:
		return "uint8_t", True
	if val < 2**16:
		return "uint16_t", True
	if val < 2**32:
		return "uint32_t", True
	if val < 2**64:
		return "uint64_t", True
	if val < 2**128:
		return "uint128_t", False
	if val < 2**256:
		return "uint256_t", False
	if val < 2**512:
		return "uint512_t", False
	if val < 2**1024:
		return "uint1024_t", False
	
	raise ValueError


def factEntryInfo(N, K):
	val = math.factorial(N) / math.factorial(N-K)
	
	dt, isStatic = dataType(val)
	
	valStr = None
	attrStr = None
	
	if not isStatic:
		valStr = "\"" + str(val) + "\""
		attrStr = "static"
	else:
		valStr = str(val) + "u"
		attrStr = "static constexpr"
	
	
	return (N, K, dt, attrStr, valStr)


print """
#pragma once

#include <cstdint>
#include <boost/multiprecision/cpp_int.hpp>

namespace mjon661 { namespace static_data {
	using uint128_t = boost::multiprecision::uint128_t;
	using uint256_t = boost::multiprecision::uint256_t;
	using uint512_t = boost::multiprecision::uint512_t;
	using uint1024_t = boost::multiprecision::uint1024_t;

	template<unsigned N, unsigned K>
	struct Factorial;
	
"""

for i in range(1, 30):
	for j in range(0, i+1):
		print """
	template<>
	struct Factorial<{0}, {1}> {{
		using type = {2};
		{3} type getValue() {{ return type({4}); }}
	}};""".format(*factEntryInfo(i, j))


print """

}}
"""
