
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
	


	template<>
	struct Factorial<1, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<1, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<2, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<2, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(2u); }
	};

	template<>
	struct Factorial<2, 2> {
		using type = uint8_t;
		static constexpr type getValue() { return type(2u); }
	};

	template<>
	struct Factorial<3, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<3, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(3u); }
	};

	template<>
	struct Factorial<3, 2> {
		using type = uint8_t;
		static constexpr type getValue() { return type(6u); }
	};

	template<>
	struct Factorial<3, 3> {
		using type = uint8_t;
		static constexpr type getValue() { return type(6u); }
	};

	template<>
	struct Factorial<4, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<4, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(4u); }
	};

	template<>
	struct Factorial<4, 2> {
		using type = uint8_t;
		static constexpr type getValue() { return type(12u); }
	};

	template<>
	struct Factorial<4, 3> {
		using type = uint8_t;
		static constexpr type getValue() { return type(24u); }
	};

	template<>
	struct Factorial<4, 4> {
		using type = uint8_t;
		static constexpr type getValue() { return type(24u); }
	};

	template<>
	struct Factorial<5, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<5, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(5u); }
	};

	template<>
	struct Factorial<5, 2> {
		using type = uint8_t;
		static constexpr type getValue() { return type(20u); }
	};

	template<>
	struct Factorial<5, 3> {
		using type = uint8_t;
		static constexpr type getValue() { return type(60u); }
	};

	template<>
	struct Factorial<5, 4> {
		using type = uint8_t;
		static constexpr type getValue() { return type(120u); }
	};

	template<>
	struct Factorial<5, 5> {
		using type = uint8_t;
		static constexpr type getValue() { return type(120u); }
	};

	template<>
	struct Factorial<6, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<6, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(6u); }
	};

	template<>
	struct Factorial<6, 2> {
		using type = uint8_t;
		static constexpr type getValue() { return type(30u); }
	};

	template<>
	struct Factorial<6, 3> {
		using type = uint8_t;
		static constexpr type getValue() { return type(120u); }
	};

	template<>
	struct Factorial<6, 4> {
		using type = uint16_t;
		static constexpr type getValue() { return type(360u); }
	};

	template<>
	struct Factorial<6, 5> {
		using type = uint16_t;
		static constexpr type getValue() { return type(720u); }
	};

	template<>
	struct Factorial<6, 6> {
		using type = uint16_t;
		static constexpr type getValue() { return type(720u); }
	};

	template<>
	struct Factorial<7, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<7, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(7u); }
	};

	template<>
	struct Factorial<7, 2> {
		using type = uint8_t;
		static constexpr type getValue() { return type(42u); }
	};

	template<>
	struct Factorial<7, 3> {
		using type = uint8_t;
		static constexpr type getValue() { return type(210u); }
	};

	template<>
	struct Factorial<7, 4> {
		using type = uint16_t;
		static constexpr type getValue() { return type(840u); }
	};

	template<>
	struct Factorial<7, 5> {
		using type = uint16_t;
		static constexpr type getValue() { return type(2520u); }
	};

	template<>
	struct Factorial<7, 6> {
		using type = uint16_t;
		static constexpr type getValue() { return type(5040u); }
	};

	template<>
	struct Factorial<7, 7> {
		using type = uint16_t;
		static constexpr type getValue() { return type(5040u); }
	};

	template<>
	struct Factorial<8, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<8, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(8u); }
	};

	template<>
	struct Factorial<8, 2> {
		using type = uint8_t;
		static constexpr type getValue() { return type(56u); }
	};

	template<>
	struct Factorial<8, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(336u); }
	};

	template<>
	struct Factorial<8, 4> {
		using type = uint16_t;
		static constexpr type getValue() { return type(1680u); }
	};

	template<>
	struct Factorial<8, 5> {
		using type = uint16_t;
		static constexpr type getValue() { return type(6720u); }
	};

	template<>
	struct Factorial<8, 6> {
		using type = uint16_t;
		static constexpr type getValue() { return type(20160u); }
	};

	template<>
	struct Factorial<8, 7> {
		using type = uint16_t;
		static constexpr type getValue() { return type(40320u); }
	};

	template<>
	struct Factorial<8, 8> {
		using type = uint16_t;
		static constexpr type getValue() { return type(40320u); }
	};

	template<>
	struct Factorial<9, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<9, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(9u); }
	};

	template<>
	struct Factorial<9, 2> {
		using type = uint8_t;
		static constexpr type getValue() { return type(72u); }
	};

	template<>
	struct Factorial<9, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(504u); }
	};

	template<>
	struct Factorial<9, 4> {
		using type = uint16_t;
		static constexpr type getValue() { return type(3024u); }
	};

	template<>
	struct Factorial<9, 5> {
		using type = uint16_t;
		static constexpr type getValue() { return type(15120u); }
	};

	template<>
	struct Factorial<9, 6> {
		using type = uint16_t;
		static constexpr type getValue() { return type(60480u); }
	};

	template<>
	struct Factorial<9, 7> {
		using type = uint32_t;
		static constexpr type getValue() { return type(181440u); }
	};

	template<>
	struct Factorial<9, 8> {
		using type = uint32_t;
		static constexpr type getValue() { return type(362880u); }
	};

	template<>
	struct Factorial<9, 9> {
		using type = uint32_t;
		static constexpr type getValue() { return type(362880u); }
	};

	template<>
	struct Factorial<10, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<10, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(10u); }
	};

	template<>
	struct Factorial<10, 2> {
		using type = uint8_t;
		static constexpr type getValue() { return type(90u); }
	};

	template<>
	struct Factorial<10, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(720u); }
	};

	template<>
	struct Factorial<10, 4> {
		using type = uint16_t;
		static constexpr type getValue() { return type(5040u); }
	};

	template<>
	struct Factorial<10, 5> {
		using type = uint16_t;
		static constexpr type getValue() { return type(30240u); }
	};

	template<>
	struct Factorial<10, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(151200u); }
	};

	template<>
	struct Factorial<10, 7> {
		using type = uint32_t;
		static constexpr type getValue() { return type(604800u); }
	};

	template<>
	struct Factorial<10, 8> {
		using type = uint32_t;
		static constexpr type getValue() { return type(1814400u); }
	};

	template<>
	struct Factorial<10, 9> {
		using type = uint32_t;
		static constexpr type getValue() { return type(3628800u); }
	};

	template<>
	struct Factorial<10, 10> {
		using type = uint32_t;
		static constexpr type getValue() { return type(3628800u); }
	};

	template<>
	struct Factorial<11, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<11, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(11u); }
	};

	template<>
	struct Factorial<11, 2> {
		using type = uint8_t;
		static constexpr type getValue() { return type(110u); }
	};

	template<>
	struct Factorial<11, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(990u); }
	};

	template<>
	struct Factorial<11, 4> {
		using type = uint16_t;
		static constexpr type getValue() { return type(7920u); }
	};

	template<>
	struct Factorial<11, 5> {
		using type = uint16_t;
		static constexpr type getValue() { return type(55440u); }
	};

	template<>
	struct Factorial<11, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(332640u); }
	};

	template<>
	struct Factorial<11, 7> {
		using type = uint32_t;
		static constexpr type getValue() { return type(1663200u); }
	};

	template<>
	struct Factorial<11, 8> {
		using type = uint32_t;
		static constexpr type getValue() { return type(6652800u); }
	};

	template<>
	struct Factorial<11, 9> {
		using type = uint32_t;
		static constexpr type getValue() { return type(19958400u); }
	};

	template<>
	struct Factorial<11, 10> {
		using type = uint32_t;
		static constexpr type getValue() { return type(39916800u); }
	};

	template<>
	struct Factorial<11, 11> {
		using type = uint32_t;
		static constexpr type getValue() { return type(39916800u); }
	};

	template<>
	struct Factorial<12, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<12, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(12u); }
	};

	template<>
	struct Factorial<12, 2> {
		using type = uint8_t;
		static constexpr type getValue() { return type(132u); }
	};

	template<>
	struct Factorial<12, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(1320u); }
	};

	template<>
	struct Factorial<12, 4> {
		using type = uint16_t;
		static constexpr type getValue() { return type(11880u); }
	};

	template<>
	struct Factorial<12, 5> {
		using type = uint32_t;
		static constexpr type getValue() { return type(95040u); }
	};

	template<>
	struct Factorial<12, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(665280u); }
	};

	template<>
	struct Factorial<12, 7> {
		using type = uint32_t;
		static constexpr type getValue() { return type(3991680u); }
	};

	template<>
	struct Factorial<12, 8> {
		using type = uint32_t;
		static constexpr type getValue() { return type(19958400u); }
	};

	template<>
	struct Factorial<12, 9> {
		using type = uint32_t;
		static constexpr type getValue() { return type(79833600u); }
	};

	template<>
	struct Factorial<12, 10> {
		using type = uint32_t;
		static constexpr type getValue() { return type(239500800u); }
	};

	template<>
	struct Factorial<12, 11> {
		using type = uint32_t;
		static constexpr type getValue() { return type(479001600u); }
	};

	template<>
	struct Factorial<12, 12> {
		using type = uint32_t;
		static constexpr type getValue() { return type(479001600u); }
	};

	template<>
	struct Factorial<13, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<13, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(13u); }
	};

	template<>
	struct Factorial<13, 2> {
		using type = uint8_t;
		static constexpr type getValue() { return type(156u); }
	};

	template<>
	struct Factorial<13, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(1716u); }
	};

	template<>
	struct Factorial<13, 4> {
		using type = uint16_t;
		static constexpr type getValue() { return type(17160u); }
	};

	template<>
	struct Factorial<13, 5> {
		using type = uint32_t;
		static constexpr type getValue() { return type(154440u); }
	};

	template<>
	struct Factorial<13, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(1235520u); }
	};

	template<>
	struct Factorial<13, 7> {
		using type = uint32_t;
		static constexpr type getValue() { return type(8648640u); }
	};

	template<>
	struct Factorial<13, 8> {
		using type = uint32_t;
		static constexpr type getValue() { return type(51891840u); }
	};

	template<>
	struct Factorial<13, 9> {
		using type = uint32_t;
		static constexpr type getValue() { return type(259459200u); }
	};

	template<>
	struct Factorial<13, 10> {
		using type = uint32_t;
		static constexpr type getValue() { return type(1037836800u); }
	};

	template<>
	struct Factorial<13, 11> {
		using type = uint32_t;
		static constexpr type getValue() { return type(3113510400u); }
	};

	template<>
	struct Factorial<13, 12> {
		using type = uint64_t;
		static constexpr type getValue() { return type(6227020800u); }
	};

	template<>
	struct Factorial<13, 13> {
		using type = uint64_t;
		static constexpr type getValue() { return type(6227020800u); }
	};

	template<>
	struct Factorial<14, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<14, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(14u); }
	};

	template<>
	struct Factorial<14, 2> {
		using type = uint8_t;
		static constexpr type getValue() { return type(182u); }
	};

	template<>
	struct Factorial<14, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(2184u); }
	};

	template<>
	struct Factorial<14, 4> {
		using type = uint16_t;
		static constexpr type getValue() { return type(24024u); }
	};

	template<>
	struct Factorial<14, 5> {
		using type = uint32_t;
		static constexpr type getValue() { return type(240240u); }
	};

	template<>
	struct Factorial<14, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(2162160u); }
	};

	template<>
	struct Factorial<14, 7> {
		using type = uint32_t;
		static constexpr type getValue() { return type(17297280u); }
	};

	template<>
	struct Factorial<14, 8> {
		using type = uint32_t;
		static constexpr type getValue() { return type(121080960u); }
	};

	template<>
	struct Factorial<14, 9> {
		using type = uint32_t;
		static constexpr type getValue() { return type(726485760u); }
	};

	template<>
	struct Factorial<14, 10> {
		using type = uint32_t;
		static constexpr type getValue() { return type(3632428800u); }
	};

	template<>
	struct Factorial<14, 11> {
		using type = uint64_t;
		static constexpr type getValue() { return type(14529715200u); }
	};

	template<>
	struct Factorial<14, 12> {
		using type = uint64_t;
		static constexpr type getValue() { return type(43589145600u); }
	};

	template<>
	struct Factorial<14, 13> {
		using type = uint64_t;
		static constexpr type getValue() { return type(87178291200u); }
	};

	template<>
	struct Factorial<14, 14> {
		using type = uint64_t;
		static constexpr type getValue() { return type(87178291200u); }
	};

	template<>
	struct Factorial<15, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<15, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(15u); }
	};

	template<>
	struct Factorial<15, 2> {
		using type = uint8_t;
		static constexpr type getValue() { return type(210u); }
	};

	template<>
	struct Factorial<15, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(2730u); }
	};

	template<>
	struct Factorial<15, 4> {
		using type = uint16_t;
		static constexpr type getValue() { return type(32760u); }
	};

	template<>
	struct Factorial<15, 5> {
		using type = uint32_t;
		static constexpr type getValue() { return type(360360u); }
	};

	template<>
	struct Factorial<15, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(3603600u); }
	};

	template<>
	struct Factorial<15, 7> {
		using type = uint32_t;
		static constexpr type getValue() { return type(32432400u); }
	};

	template<>
	struct Factorial<15, 8> {
		using type = uint32_t;
		static constexpr type getValue() { return type(259459200u); }
	};

	template<>
	struct Factorial<15, 9> {
		using type = uint32_t;
		static constexpr type getValue() { return type(1816214400u); }
	};

	template<>
	struct Factorial<15, 10> {
		using type = uint64_t;
		static constexpr type getValue() { return type(10897286400u); }
	};

	template<>
	struct Factorial<15, 11> {
		using type = uint64_t;
		static constexpr type getValue() { return type(54486432000u); }
	};

	template<>
	struct Factorial<15, 12> {
		using type = uint64_t;
		static constexpr type getValue() { return type(217945728000u); }
	};

	template<>
	struct Factorial<15, 13> {
		using type = uint64_t;
		static constexpr type getValue() { return type(653837184000u); }
	};

	template<>
	struct Factorial<15, 14> {
		using type = uint64_t;
		static constexpr type getValue() { return type(1307674368000u); }
	};

	template<>
	struct Factorial<15, 15> {
		using type = uint64_t;
		static constexpr type getValue() { return type(1307674368000u); }
	};

	template<>
	struct Factorial<16, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<16, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(16u); }
	};

	template<>
	struct Factorial<16, 2> {
		using type = uint8_t;
		static constexpr type getValue() { return type(240u); }
	};

	template<>
	struct Factorial<16, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(3360u); }
	};

	template<>
	struct Factorial<16, 4> {
		using type = uint16_t;
		static constexpr type getValue() { return type(43680u); }
	};

	template<>
	struct Factorial<16, 5> {
		using type = uint32_t;
		static constexpr type getValue() { return type(524160u); }
	};

	template<>
	struct Factorial<16, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(5765760u); }
	};

	template<>
	struct Factorial<16, 7> {
		using type = uint32_t;
		static constexpr type getValue() { return type(57657600u); }
	};

	template<>
	struct Factorial<16, 8> {
		using type = uint32_t;
		static constexpr type getValue() { return type(518918400u); }
	};

	template<>
	struct Factorial<16, 9> {
		using type = uint32_t;
		static constexpr type getValue() { return type(4151347200u); }
	};

	template<>
	struct Factorial<16, 10> {
		using type = uint64_t;
		static constexpr type getValue() { return type(29059430400u); }
	};

	template<>
	struct Factorial<16, 11> {
		using type = uint64_t;
		static constexpr type getValue() { return type(174356582400u); }
	};

	template<>
	struct Factorial<16, 12> {
		using type = uint64_t;
		static constexpr type getValue() { return type(871782912000u); }
	};

	template<>
	struct Factorial<16, 13> {
		using type = uint64_t;
		static constexpr type getValue() { return type(3487131648000u); }
	};

	template<>
	struct Factorial<16, 14> {
		using type = uint64_t;
		static constexpr type getValue() { return type(10461394944000u); }
	};

	template<>
	struct Factorial<16, 15> {
		using type = uint64_t;
		static constexpr type getValue() { return type(20922789888000u); }
	};

	template<>
	struct Factorial<16, 16> {
		using type = uint64_t;
		static constexpr type getValue() { return type(20922789888000u); }
	};

	template<>
	struct Factorial<17, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<17, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(17u); }
	};

	template<>
	struct Factorial<17, 2> {
		using type = uint16_t;
		static constexpr type getValue() { return type(272u); }
	};

	template<>
	struct Factorial<17, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(4080u); }
	};

	template<>
	struct Factorial<17, 4> {
		using type = uint16_t;
		static constexpr type getValue() { return type(57120u); }
	};

	template<>
	struct Factorial<17, 5> {
		using type = uint32_t;
		static constexpr type getValue() { return type(742560u); }
	};

	template<>
	struct Factorial<17, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(8910720u); }
	};

	template<>
	struct Factorial<17, 7> {
		using type = uint32_t;
		static constexpr type getValue() { return type(98017920u); }
	};

	template<>
	struct Factorial<17, 8> {
		using type = uint32_t;
		static constexpr type getValue() { return type(980179200u); }
	};

	template<>
	struct Factorial<17, 9> {
		using type = uint64_t;
		static constexpr type getValue() { return type(8821612800u); }
	};

	template<>
	struct Factorial<17, 10> {
		using type = uint64_t;
		static constexpr type getValue() { return type(70572902400u); }
	};

	template<>
	struct Factorial<17, 11> {
		using type = uint64_t;
		static constexpr type getValue() { return type(494010316800u); }
	};

	template<>
	struct Factorial<17, 12> {
		using type = uint64_t;
		static constexpr type getValue() { return type(2964061900800u); }
	};

	template<>
	struct Factorial<17, 13> {
		using type = uint64_t;
		static constexpr type getValue() { return type(14820309504000u); }
	};

	template<>
	struct Factorial<17, 14> {
		using type = uint64_t;
		static constexpr type getValue() { return type(59281238016000u); }
	};

	template<>
	struct Factorial<17, 15> {
		using type = uint64_t;
		static constexpr type getValue() { return type(177843714048000u); }
	};

	template<>
	struct Factorial<17, 16> {
		using type = uint64_t;
		static constexpr type getValue() { return type(355687428096000u); }
	};

	template<>
	struct Factorial<17, 17> {
		using type = uint64_t;
		static constexpr type getValue() { return type(355687428096000u); }
	};

	template<>
	struct Factorial<18, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<18, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(18u); }
	};

	template<>
	struct Factorial<18, 2> {
		using type = uint16_t;
		static constexpr type getValue() { return type(306u); }
	};

	template<>
	struct Factorial<18, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(4896u); }
	};

	template<>
	struct Factorial<18, 4> {
		using type = uint32_t;
		static constexpr type getValue() { return type(73440u); }
	};

	template<>
	struct Factorial<18, 5> {
		using type = uint32_t;
		static constexpr type getValue() { return type(1028160u); }
	};

	template<>
	struct Factorial<18, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(13366080u); }
	};

	template<>
	struct Factorial<18, 7> {
		using type = uint32_t;
		static constexpr type getValue() { return type(160392960u); }
	};

	template<>
	struct Factorial<18, 8> {
		using type = uint32_t;
		static constexpr type getValue() { return type(1764322560u); }
	};

	template<>
	struct Factorial<18, 9> {
		using type = uint64_t;
		static constexpr type getValue() { return type(17643225600u); }
	};

	template<>
	struct Factorial<18, 10> {
		using type = uint64_t;
		static constexpr type getValue() { return type(158789030400u); }
	};

	template<>
	struct Factorial<18, 11> {
		using type = uint64_t;
		static constexpr type getValue() { return type(1270312243200u); }
	};

	template<>
	struct Factorial<18, 12> {
		using type = uint64_t;
		static constexpr type getValue() { return type(8892185702400u); }
	};

	template<>
	struct Factorial<18, 13> {
		using type = uint64_t;
		static constexpr type getValue() { return type(53353114214400u); }
	};

	template<>
	struct Factorial<18, 14> {
		using type = uint64_t;
		static constexpr type getValue() { return type(266765571072000u); }
	};

	template<>
	struct Factorial<18, 15> {
		using type = uint64_t;
		static constexpr type getValue() { return type(1067062284288000u); }
	};

	template<>
	struct Factorial<18, 16> {
		using type = uint64_t;
		static constexpr type getValue() { return type(3201186852864000u); }
	};

	template<>
	struct Factorial<18, 17> {
		using type = uint64_t;
		static constexpr type getValue() { return type(6402373705728000u); }
	};

	template<>
	struct Factorial<18, 18> {
		using type = uint64_t;
		static constexpr type getValue() { return type(6402373705728000u); }
	};

	template<>
	struct Factorial<19, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<19, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(19u); }
	};

	template<>
	struct Factorial<19, 2> {
		using type = uint16_t;
		static constexpr type getValue() { return type(342u); }
	};

	template<>
	struct Factorial<19, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(5814u); }
	};

	template<>
	struct Factorial<19, 4> {
		using type = uint32_t;
		static constexpr type getValue() { return type(93024u); }
	};

	template<>
	struct Factorial<19, 5> {
		using type = uint32_t;
		static constexpr type getValue() { return type(1395360u); }
	};

	template<>
	struct Factorial<19, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(19535040u); }
	};

	template<>
	struct Factorial<19, 7> {
		using type = uint32_t;
		static constexpr type getValue() { return type(253955520u); }
	};

	template<>
	struct Factorial<19, 8> {
		using type = uint32_t;
		static constexpr type getValue() { return type(3047466240u); }
	};

	template<>
	struct Factorial<19, 9> {
		using type = uint64_t;
		static constexpr type getValue() { return type(33522128640u); }
	};

	template<>
	struct Factorial<19, 10> {
		using type = uint64_t;
		static constexpr type getValue() { return type(335221286400u); }
	};

	template<>
	struct Factorial<19, 11> {
		using type = uint64_t;
		static constexpr type getValue() { return type(3016991577600u); }
	};

	template<>
	struct Factorial<19, 12> {
		using type = uint64_t;
		static constexpr type getValue() { return type(24135932620800u); }
	};

	template<>
	struct Factorial<19, 13> {
		using type = uint64_t;
		static constexpr type getValue() { return type(168951528345600u); }
	};

	template<>
	struct Factorial<19, 14> {
		using type = uint64_t;
		static constexpr type getValue() { return type(1013709170073600u); }
	};

	template<>
	struct Factorial<19, 15> {
		using type = uint64_t;
		static constexpr type getValue() { return type(5068545850368000u); }
	};

	template<>
	struct Factorial<19, 16> {
		using type = uint64_t;
		static constexpr type getValue() { return type(20274183401472000u); }
	};

	template<>
	struct Factorial<19, 17> {
		using type = uint64_t;
		static constexpr type getValue() { return type(60822550204416000u); }
	};

	template<>
	struct Factorial<19, 18> {
		using type = uint64_t;
		static constexpr type getValue() { return type(121645100408832000u); }
	};

	template<>
	struct Factorial<19, 19> {
		using type = uint64_t;
		static constexpr type getValue() { return type(121645100408832000u); }
	};

	template<>
	struct Factorial<20, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<20, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(20u); }
	};

	template<>
	struct Factorial<20, 2> {
		using type = uint16_t;
		static constexpr type getValue() { return type(380u); }
	};

	template<>
	struct Factorial<20, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(6840u); }
	};

	template<>
	struct Factorial<20, 4> {
		using type = uint32_t;
		static constexpr type getValue() { return type(116280u); }
	};

	template<>
	struct Factorial<20, 5> {
		using type = uint32_t;
		static constexpr type getValue() { return type(1860480u); }
	};

	template<>
	struct Factorial<20, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(27907200u); }
	};

	template<>
	struct Factorial<20, 7> {
		using type = uint32_t;
		static constexpr type getValue() { return type(390700800u); }
	};

	template<>
	struct Factorial<20, 8> {
		using type = uint64_t;
		static constexpr type getValue() { return type(5079110400u); }
	};

	template<>
	struct Factorial<20, 9> {
		using type = uint64_t;
		static constexpr type getValue() { return type(60949324800u); }
	};

	template<>
	struct Factorial<20, 10> {
		using type = uint64_t;
		static constexpr type getValue() { return type(670442572800u); }
	};

	template<>
	struct Factorial<20, 11> {
		using type = uint64_t;
		static constexpr type getValue() { return type(6704425728000u); }
	};

	template<>
	struct Factorial<20, 12> {
		using type = uint64_t;
		static constexpr type getValue() { return type(60339831552000u); }
	};

	template<>
	struct Factorial<20, 13> {
		using type = uint64_t;
		static constexpr type getValue() { return type(482718652416000u); }
	};

	template<>
	struct Factorial<20, 14> {
		using type = uint64_t;
		static constexpr type getValue() { return type(3379030566912000u); }
	};

	template<>
	struct Factorial<20, 15> {
		using type = uint64_t;
		static constexpr type getValue() { return type(20274183401472000u); }
	};

	template<>
	struct Factorial<20, 16> {
		using type = uint64_t;
		static constexpr type getValue() { return type(101370917007360000u); }
	};

	template<>
	struct Factorial<20, 17> {
		using type = uint64_t;
		static constexpr type getValue() { return type(405483668029440000u); }
	};

	template<>
	struct Factorial<20, 18> {
		using type = uint64_t;
		static constexpr type getValue() { return type(1216451004088320000u); }
	};

	template<>
	struct Factorial<20, 19> {
		using type = uint64_t;
		static constexpr type getValue() { return type(2432902008176640000u); }
	};

	template<>
	struct Factorial<20, 20> {
		using type = uint64_t;
		static constexpr type getValue() { return type(2432902008176640000u); }
	};

	template<>
	struct Factorial<21, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<21, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(21u); }
	};

	template<>
	struct Factorial<21, 2> {
		using type = uint16_t;
		static constexpr type getValue() { return type(420u); }
	};

	template<>
	struct Factorial<21, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(7980u); }
	};

	template<>
	struct Factorial<21, 4> {
		using type = uint32_t;
		static constexpr type getValue() { return type(143640u); }
	};

	template<>
	struct Factorial<21, 5> {
		using type = uint32_t;
		static constexpr type getValue() { return type(2441880u); }
	};

	template<>
	struct Factorial<21, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(39070080u); }
	};

	template<>
	struct Factorial<21, 7> {
		using type = uint32_t;
		static constexpr type getValue() { return type(586051200u); }
	};

	template<>
	struct Factorial<21, 8> {
		using type = uint64_t;
		static constexpr type getValue() { return type(8204716800u); }
	};

	template<>
	struct Factorial<21, 9> {
		using type = uint64_t;
		static constexpr type getValue() { return type(106661318400u); }
	};

	template<>
	struct Factorial<21, 10> {
		using type = uint64_t;
		static constexpr type getValue() { return type(1279935820800u); }
	};

	template<>
	struct Factorial<21, 11> {
		using type = uint64_t;
		static constexpr type getValue() { return type(14079294028800u); }
	};

	template<>
	struct Factorial<21, 12> {
		using type = uint64_t;
		static constexpr type getValue() { return type(140792940288000u); }
	};

	template<>
	struct Factorial<21, 13> {
		using type = uint64_t;
		static constexpr type getValue() { return type(1267136462592000u); }
	};

	template<>
	struct Factorial<21, 14> {
		using type = uint64_t;
		static constexpr type getValue() { return type(10137091700736000u); }
	};

	template<>
	struct Factorial<21, 15> {
		using type = uint64_t;
		static constexpr type getValue() { return type(70959641905152000u); }
	};

	template<>
	struct Factorial<21, 16> {
		using type = uint64_t;
		static constexpr type getValue() { return type(425757851430912000u); }
	};

	template<>
	struct Factorial<21, 17> {
		using type = uint64_t;
		static constexpr type getValue() { return type(2128789257154560000u); }
	};

	template<>
	struct Factorial<21, 18> {
		using type = uint64_t;
		static constexpr type getValue() { return type(8515157028618240000u); }
	};

	template<>
	struct Factorial<21, 19> {
		using type = uint128_t;
		static type getValue() { return type("25545471085854720000"); }
	};

	template<>
	struct Factorial<21, 20> {
		using type = uint128_t;
		static type getValue() { return type("51090942171709440000"); }
	};

	template<>
	struct Factorial<21, 21> {
		using type = uint128_t;
		static type getValue() { return type("51090942171709440000"); }
	};

	template<>
	struct Factorial<22, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<22, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(22u); }
	};

	template<>
	struct Factorial<22, 2> {
		using type = uint16_t;
		static constexpr type getValue() { return type(462u); }
	};

	template<>
	struct Factorial<22, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(9240u); }
	};

	template<>
	struct Factorial<22, 4> {
		using type = uint32_t;
		static constexpr type getValue() { return type(175560u); }
	};

	template<>
	struct Factorial<22, 5> {
		using type = uint32_t;
		static constexpr type getValue() { return type(3160080u); }
	};

	template<>
	struct Factorial<22, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(53721360u); }
	};

	template<>
	struct Factorial<22, 7> {
		using type = uint32_t;
		static constexpr type getValue() { return type(859541760u); }
	};

	template<>
	struct Factorial<22, 8> {
		using type = uint64_t;
		static constexpr type getValue() { return type(12893126400u); }
	};

	template<>
	struct Factorial<22, 9> {
		using type = uint64_t;
		static constexpr type getValue() { return type(180503769600u); }
	};

	template<>
	struct Factorial<22, 10> {
		using type = uint64_t;
		static constexpr type getValue() { return type(2346549004800u); }
	};

	template<>
	struct Factorial<22, 11> {
		using type = uint64_t;
		static constexpr type getValue() { return type(28158588057600u); }
	};

	template<>
	struct Factorial<22, 12> {
		using type = uint64_t;
		static constexpr type getValue() { return type(309744468633600u); }
	};

	template<>
	struct Factorial<22, 13> {
		using type = uint64_t;
		static constexpr type getValue() { return type(3097444686336000u); }
	};

	template<>
	struct Factorial<22, 14> {
		using type = uint64_t;
		static constexpr type getValue() { return type(27877002177024000u); }
	};

	template<>
	struct Factorial<22, 15> {
		using type = uint64_t;
		static constexpr type getValue() { return type(223016017416192000u); }
	};

	template<>
	struct Factorial<22, 16> {
		using type = uint64_t;
		static constexpr type getValue() { return type(1561112121913344000u); }
	};

	template<>
	struct Factorial<22, 17> {
		using type = uint64_t;
		static constexpr type getValue() { return type(9366672731480064000u); }
	};

	template<>
	struct Factorial<22, 18> {
		using type = uint128_t;
		static type getValue() { return type("46833363657400320000"); }
	};

	template<>
	struct Factorial<22, 19> {
		using type = uint128_t;
		static type getValue() { return type("187333454629601280000"); }
	};

	template<>
	struct Factorial<22, 20> {
		using type = uint128_t;
		static type getValue() { return type("562000363888803840000"); }
	};

	template<>
	struct Factorial<22, 21> {
		using type = uint128_t;
		static type getValue() { return type("1124000727777607680000"); }
	};

	template<>
	struct Factorial<22, 22> {
		using type = uint128_t;
		static type getValue() { return type("1124000727777607680000"); }
	};

	template<>
	struct Factorial<23, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<23, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(23u); }
	};

	template<>
	struct Factorial<23, 2> {
		using type = uint16_t;
		static constexpr type getValue() { return type(506u); }
	};

	template<>
	struct Factorial<23, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(10626u); }
	};

	template<>
	struct Factorial<23, 4> {
		using type = uint32_t;
		static constexpr type getValue() { return type(212520u); }
	};

	template<>
	struct Factorial<23, 5> {
		using type = uint32_t;
		static constexpr type getValue() { return type(4037880u); }
	};

	template<>
	struct Factorial<23, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(72681840u); }
	};

	template<>
	struct Factorial<23, 7> {
		using type = uint32_t;
		static constexpr type getValue() { return type(1235591280u); }
	};

	template<>
	struct Factorial<23, 8> {
		using type = uint64_t;
		static constexpr type getValue() { return type(19769460480u); }
	};

	template<>
	struct Factorial<23, 9> {
		using type = uint64_t;
		static constexpr type getValue() { return type(296541907200u); }
	};

	template<>
	struct Factorial<23, 10> {
		using type = uint64_t;
		static constexpr type getValue() { return type(4151586700800u); }
	};

	template<>
	struct Factorial<23, 11> {
		using type = uint64_t;
		static constexpr type getValue() { return type(53970627110400u); }
	};

	template<>
	struct Factorial<23, 12> {
		using type = uint64_t;
		static constexpr type getValue() { return type(647647525324800u); }
	};

	template<>
	struct Factorial<23, 13> {
		using type = uint64_t;
		static constexpr type getValue() { return type(7124122778572800u); }
	};

	template<>
	struct Factorial<23, 14> {
		using type = uint64_t;
		static constexpr type getValue() { return type(71241227785728000u); }
	};

	template<>
	struct Factorial<23, 15> {
		using type = uint64_t;
		static constexpr type getValue() { return type(641171050071552000u); }
	};

	template<>
	struct Factorial<23, 16> {
		using type = uint64_t;
		static constexpr type getValue() { return type(5129368400572416000u); }
	};

	template<>
	struct Factorial<23, 17> {
		using type = uint128_t;
		static type getValue() { return type("35905578804006912000"); }
	};

	template<>
	struct Factorial<23, 18> {
		using type = uint128_t;
		static type getValue() { return type("215433472824041472000"); }
	};

	template<>
	struct Factorial<23, 19> {
		using type = uint128_t;
		static type getValue() { return type("1077167364120207360000"); }
	};

	template<>
	struct Factorial<23, 20> {
		using type = uint128_t;
		static type getValue() { return type("4308669456480829440000"); }
	};

	template<>
	struct Factorial<23, 21> {
		using type = uint128_t;
		static type getValue() { return type("12926008369442488320000"); }
	};

	template<>
	struct Factorial<23, 22> {
		using type = uint128_t;
		static type getValue() { return type("25852016738884976640000"); }
	};

	template<>
	struct Factorial<23, 23> {
		using type = uint128_t;
		static type getValue() { return type("25852016738884976640000"); }
	};

	template<>
	struct Factorial<24, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<24, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(24u); }
	};

	template<>
	struct Factorial<24, 2> {
		using type = uint16_t;
		static constexpr type getValue() { return type(552u); }
	};

	template<>
	struct Factorial<24, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(12144u); }
	};

	template<>
	struct Factorial<24, 4> {
		using type = uint32_t;
		static constexpr type getValue() { return type(255024u); }
	};

	template<>
	struct Factorial<24, 5> {
		using type = uint32_t;
		static constexpr type getValue() { return type(5100480u); }
	};

	template<>
	struct Factorial<24, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(96909120u); }
	};

	template<>
	struct Factorial<24, 7> {
		using type = uint32_t;
		static constexpr type getValue() { return type(1744364160u); }
	};

	template<>
	struct Factorial<24, 8> {
		using type = uint64_t;
		static constexpr type getValue() { return type(29654190720u); }
	};

	template<>
	struct Factorial<24, 9> {
		using type = uint64_t;
		static constexpr type getValue() { return type(474467051520u); }
	};

	template<>
	struct Factorial<24, 10> {
		using type = uint64_t;
		static constexpr type getValue() { return type(7117005772800u); }
	};

	template<>
	struct Factorial<24, 11> {
		using type = uint64_t;
		static constexpr type getValue() { return type(99638080819200u); }
	};

	template<>
	struct Factorial<24, 12> {
		using type = uint64_t;
		static constexpr type getValue() { return type(1295295050649600u); }
	};

	template<>
	struct Factorial<24, 13> {
		using type = uint64_t;
		static constexpr type getValue() { return type(15543540607795200u); }
	};

	template<>
	struct Factorial<24, 14> {
		using type = uint64_t;
		static constexpr type getValue() { return type(170978946685747200u); }
	};

	template<>
	struct Factorial<24, 15> {
		using type = uint64_t;
		static constexpr type getValue() { return type(1709789466857472000u); }
	};

	template<>
	struct Factorial<24, 16> {
		using type = uint64_t;
		static constexpr type getValue() { return type(15388105201717248000u); }
	};

	template<>
	struct Factorial<24, 17> {
		using type = uint128_t;
		static type getValue() { return type("123104841613737984000"); }
	};

	template<>
	struct Factorial<24, 18> {
		using type = uint128_t;
		static type getValue() { return type("861733891296165888000"); }
	};

	template<>
	struct Factorial<24, 19> {
		using type = uint128_t;
		static type getValue() { return type("5170403347776995328000"); }
	};

	template<>
	struct Factorial<24, 20> {
		using type = uint128_t;
		static type getValue() { return type("25852016738884976640000"); }
	};

	template<>
	struct Factorial<24, 21> {
		using type = uint128_t;
		static type getValue() { return type("103408066955539906560000"); }
	};

	template<>
	struct Factorial<24, 22> {
		using type = uint128_t;
		static type getValue() { return type("310224200866619719680000"); }
	};

	template<>
	struct Factorial<24, 23> {
		using type = uint128_t;
		static type getValue() { return type("620448401733239439360000"); }
	};

	template<>
	struct Factorial<24, 24> {
		using type = uint128_t;
		static type getValue() { return type("620448401733239439360000"); }
	};

	template<>
	struct Factorial<25, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<25, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(25u); }
	};

	template<>
	struct Factorial<25, 2> {
		using type = uint16_t;
		static constexpr type getValue() { return type(600u); }
	};

	template<>
	struct Factorial<25, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(13800u); }
	};

	template<>
	struct Factorial<25, 4> {
		using type = uint32_t;
		static constexpr type getValue() { return type(303600u); }
	};

	template<>
	struct Factorial<25, 5> {
		using type = uint32_t;
		static constexpr type getValue() { return type(6375600u); }
	};

	template<>
	struct Factorial<25, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(127512000u); }
	};

	template<>
	struct Factorial<25, 7> {
		using type = uint32_t;
		static constexpr type getValue() { return type(2422728000u); }
	};

	template<>
	struct Factorial<25, 8> {
		using type = uint64_t;
		static constexpr type getValue() { return type(43609104000u); }
	};

	template<>
	struct Factorial<25, 9> {
		using type = uint64_t;
		static constexpr type getValue() { return type(741354768000u); }
	};

	template<>
	struct Factorial<25, 10> {
		using type = uint64_t;
		static constexpr type getValue() { return type(11861676288000u); }
	};

	template<>
	struct Factorial<25, 11> {
		using type = uint64_t;
		static constexpr type getValue() { return type(177925144320000u); }
	};

	template<>
	struct Factorial<25, 12> {
		using type = uint64_t;
		static constexpr type getValue() { return type(2490952020480000u); }
	};

	template<>
	struct Factorial<25, 13> {
		using type = uint64_t;
		static constexpr type getValue() { return type(32382376266240000u); }
	};

	template<>
	struct Factorial<25, 14> {
		using type = uint64_t;
		static constexpr type getValue() { return type(388588515194880000u); }
	};

	template<>
	struct Factorial<25, 15> {
		using type = uint64_t;
		static constexpr type getValue() { return type(4274473667143680000u); }
	};

	template<>
	struct Factorial<25, 16> {
		using type = uint128_t;
		static type getValue() { return type("42744736671436800000"); }
	};

	template<>
	struct Factorial<25, 17> {
		using type = uint128_t;
		static type getValue() { return type("384702630042931200000"); }
	};

	template<>
	struct Factorial<25, 18> {
		using type = uint128_t;
		static type getValue() { return type("3077621040343449600000"); }
	};

	template<>
	struct Factorial<25, 19> {
		using type = uint128_t;
		static type getValue() { return type("21543347282404147200000"); }
	};

	template<>
	struct Factorial<25, 20> {
		using type = uint128_t;
		static type getValue() { return type("129260083694424883200000"); }
	};

	template<>
	struct Factorial<25, 21> {
		using type = uint128_t;
		static type getValue() { return type("646300418472124416000000"); }
	};

	template<>
	struct Factorial<25, 22> {
		using type = uint128_t;
		static type getValue() { return type("2585201673888497664000000"); }
	};

	template<>
	struct Factorial<25, 23> {
		using type = uint128_t;
		static type getValue() { return type("7755605021665492992000000"); }
	};

	template<>
	struct Factorial<25, 24> {
		using type = uint128_t;
		static type getValue() { return type("15511210043330985984000000"); }
	};

	template<>
	struct Factorial<25, 25> {
		using type = uint128_t;
		static type getValue() { return type("15511210043330985984000000"); }
	};

	template<>
	struct Factorial<26, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<26, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(26u); }
	};

	template<>
	struct Factorial<26, 2> {
		using type = uint16_t;
		static constexpr type getValue() { return type(650u); }
	};

	template<>
	struct Factorial<26, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(15600u); }
	};

	template<>
	struct Factorial<26, 4> {
		using type = uint32_t;
		static constexpr type getValue() { return type(358800u); }
	};

	template<>
	struct Factorial<26, 5> {
		using type = uint32_t;
		static constexpr type getValue() { return type(7893600u); }
	};

	template<>
	struct Factorial<26, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(165765600u); }
	};

	template<>
	struct Factorial<26, 7> {
		using type = uint32_t;
		static constexpr type getValue() { return type(3315312000u); }
	};

	template<>
	struct Factorial<26, 8> {
		using type = uint64_t;
		static constexpr type getValue() { return type(62990928000u); }
	};

	template<>
	struct Factorial<26, 9> {
		using type = uint64_t;
		static constexpr type getValue() { return type(1133836704000u); }
	};

	template<>
	struct Factorial<26, 10> {
		using type = uint64_t;
		static constexpr type getValue() { return type(19275223968000u); }
	};

	template<>
	struct Factorial<26, 11> {
		using type = uint64_t;
		static constexpr type getValue() { return type(308403583488000u); }
	};

	template<>
	struct Factorial<26, 12> {
		using type = uint64_t;
		static constexpr type getValue() { return type(4626053752320000u); }
	};

	template<>
	struct Factorial<26, 13> {
		using type = uint64_t;
		static constexpr type getValue() { return type(64764752532480000u); }
	};

	template<>
	struct Factorial<26, 14> {
		using type = uint64_t;
		static constexpr type getValue() { return type(841941782922240000u); }
	};

	template<>
	struct Factorial<26, 15> {
		using type = uint64_t;
		static constexpr type getValue() { return type(10103301395066880000u); }
	};

	template<>
	struct Factorial<26, 16> {
		using type = uint128_t;
		static type getValue() { return type("111136315345735680000"); }
	};

	template<>
	struct Factorial<26, 17> {
		using type = uint128_t;
		static type getValue() { return type("1111363153457356800000"); }
	};

	template<>
	struct Factorial<26, 18> {
		using type = uint128_t;
		static type getValue() { return type("10002268381116211200000"); }
	};

	template<>
	struct Factorial<26, 19> {
		using type = uint128_t;
		static type getValue() { return type("80018147048929689600000"); }
	};

	template<>
	struct Factorial<26, 20> {
		using type = uint128_t;
		static type getValue() { return type("560127029342507827200000"); }
	};

	template<>
	struct Factorial<26, 21> {
		using type = uint128_t;
		static type getValue() { return type("3360762176055046963200000"); }
	};

	template<>
	struct Factorial<26, 22> {
		using type = uint128_t;
		static type getValue() { return type("16803810880275234816000000"); }
	};

	template<>
	struct Factorial<26, 23> {
		using type = uint128_t;
		static type getValue() { return type("67215243521100939264000000"); }
	};

	template<>
	struct Factorial<26, 24> {
		using type = uint128_t;
		static type getValue() { return type("201645730563302817792000000"); }
	};

	template<>
	struct Factorial<26, 25> {
		using type = uint128_t;
		static type getValue() { return type("403291461126605635584000000"); }
	};

	template<>
	struct Factorial<26, 26> {
		using type = uint128_t;
		static type getValue() { return type("403291461126605635584000000"); }
	};

	template<>
	struct Factorial<27, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<27, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(27u); }
	};

	template<>
	struct Factorial<27, 2> {
		using type = uint16_t;
		static constexpr type getValue() { return type(702u); }
	};

	template<>
	struct Factorial<27, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(17550u); }
	};

	template<>
	struct Factorial<27, 4> {
		using type = uint32_t;
		static constexpr type getValue() { return type(421200u); }
	};

	template<>
	struct Factorial<27, 5> {
		using type = uint32_t;
		static constexpr type getValue() { return type(9687600u); }
	};

	template<>
	struct Factorial<27, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(213127200u); }
	};

	template<>
	struct Factorial<27, 7> {
		using type = uint64_t;
		static constexpr type getValue() { return type(4475671200u); }
	};

	template<>
	struct Factorial<27, 8> {
		using type = uint64_t;
		static constexpr type getValue() { return type(89513424000u); }
	};

	template<>
	struct Factorial<27, 9> {
		using type = uint64_t;
		static constexpr type getValue() { return type(1700755056000u); }
	};

	template<>
	struct Factorial<27, 10> {
		using type = uint64_t;
		static constexpr type getValue() { return type(30613591008000u); }
	};

	template<>
	struct Factorial<27, 11> {
		using type = uint64_t;
		static constexpr type getValue() { return type(520431047136000u); }
	};

	template<>
	struct Factorial<27, 12> {
		using type = uint64_t;
		static constexpr type getValue() { return type(8326896754176000u); }
	};

	template<>
	struct Factorial<27, 13> {
		using type = uint64_t;
		static constexpr type getValue() { return type(124903451312640000u); }
	};

	template<>
	struct Factorial<27, 14> {
		using type = uint64_t;
		static constexpr type getValue() { return type(1748648318376960000u); }
	};

	template<>
	struct Factorial<27, 15> {
		using type = uint128_t;
		static type getValue() { return type("22732428138900480000"); }
	};

	template<>
	struct Factorial<27, 16> {
		using type = uint128_t;
		static type getValue() { return type("272789137666805760000"); }
	};

	template<>
	struct Factorial<27, 17> {
		using type = uint128_t;
		static type getValue() { return type("3000680514334863360000"); }
	};

	template<>
	struct Factorial<27, 18> {
		using type = uint128_t;
		static type getValue() { return type("30006805143348633600000"); }
	};

	template<>
	struct Factorial<27, 19> {
		using type = uint128_t;
		static type getValue() { return type("270061246290137702400000"); }
	};

	template<>
	struct Factorial<27, 20> {
		using type = uint128_t;
		static type getValue() { return type("2160489970321101619200000"); }
	};

	template<>
	struct Factorial<27, 21> {
		using type = uint128_t;
		static type getValue() { return type("15123429792247711334400000"); }
	};

	template<>
	struct Factorial<27, 22> {
		using type = uint128_t;
		static type getValue() { return type("90740578753486268006400000"); }
	};

	template<>
	struct Factorial<27, 23> {
		using type = uint128_t;
		static type getValue() { return type("453702893767431340032000000"); }
	};

	template<>
	struct Factorial<27, 24> {
		using type = uint128_t;
		static type getValue() { return type("1814811575069725360128000000"); }
	};

	template<>
	struct Factorial<27, 25> {
		using type = uint128_t;
		static type getValue() { return type("5444434725209176080384000000"); }
	};

	template<>
	struct Factorial<27, 26> {
		using type = uint128_t;
		static type getValue() { return type("10888869450418352160768000000"); }
	};

	template<>
	struct Factorial<27, 27> {
		using type = uint128_t;
		static type getValue() { return type("10888869450418352160768000000"); }
	};

	template<>
	struct Factorial<28, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<28, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(28u); }
	};

	template<>
	struct Factorial<28, 2> {
		using type = uint16_t;
		static constexpr type getValue() { return type(756u); }
	};

	template<>
	struct Factorial<28, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(19656u); }
	};

	template<>
	struct Factorial<28, 4> {
		using type = uint32_t;
		static constexpr type getValue() { return type(491400u); }
	};

	template<>
	struct Factorial<28, 5> {
		using type = uint32_t;
		static constexpr type getValue() { return type(11793600u); }
	};

	template<>
	struct Factorial<28, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(271252800u); }
	};

	template<>
	struct Factorial<28, 7> {
		using type = uint64_t;
		static constexpr type getValue() { return type(5967561600u); }
	};

	template<>
	struct Factorial<28, 8> {
		using type = uint64_t;
		static constexpr type getValue() { return type(125318793600u); }
	};

	template<>
	struct Factorial<28, 9> {
		using type = uint64_t;
		static constexpr type getValue() { return type(2506375872000u); }
	};

	template<>
	struct Factorial<28, 10> {
		using type = uint64_t;
		static constexpr type getValue() { return type(47621141568000u); }
	};

	template<>
	struct Factorial<28, 11> {
		using type = uint64_t;
		static constexpr type getValue() { return type(857180548224000u); }
	};

	template<>
	struct Factorial<28, 12> {
		using type = uint64_t;
		static constexpr type getValue() { return type(14572069319808000u); }
	};

	template<>
	struct Factorial<28, 13> {
		using type = uint64_t;
		static constexpr type getValue() { return type(233153109116928000u); }
	};

	template<>
	struct Factorial<28, 14> {
		using type = uint64_t;
		static constexpr type getValue() { return type(3497296636753920000u); }
	};

	template<>
	struct Factorial<28, 15> {
		using type = uint128_t;
		static type getValue() { return type("48962152914554880000"); }
	};

	template<>
	struct Factorial<28, 16> {
		using type = uint128_t;
		static type getValue() { return type("636507987889213440000"); }
	};

	template<>
	struct Factorial<28, 17> {
		using type = uint128_t;
		static type getValue() { return type("7638095854670561280000"); }
	};

	template<>
	struct Factorial<28, 18> {
		using type = uint128_t;
		static type getValue() { return type("84019054401376174080000"); }
	};

	template<>
	struct Factorial<28, 19> {
		using type = uint128_t;
		static type getValue() { return type("840190544013761740800000"); }
	};

	template<>
	struct Factorial<28, 20> {
		using type = uint128_t;
		static type getValue() { return type("7561714896123855667200000"); }
	};

	template<>
	struct Factorial<28, 21> {
		using type = uint128_t;
		static type getValue() { return type("60493719168990845337600000"); }
	};

	template<>
	struct Factorial<28, 22> {
		using type = uint128_t;
		static type getValue() { return type("423456034182935917363200000"); }
	};

	template<>
	struct Factorial<28, 23> {
		using type = uint128_t;
		static type getValue() { return type("2540736205097615504179200000"); }
	};

	template<>
	struct Factorial<28, 24> {
		using type = uint128_t;
		static type getValue() { return type("12703681025488077520896000000"); }
	};

	template<>
	struct Factorial<28, 25> {
		using type = uint128_t;
		static type getValue() { return type("50814724101952310083584000000"); }
	};

	template<>
	struct Factorial<28, 26> {
		using type = uint128_t;
		static type getValue() { return type("152444172305856930250752000000"); }
	};

	template<>
	struct Factorial<28, 27> {
		using type = uint128_t;
		static type getValue() { return type("304888344611713860501504000000"); }
	};

	template<>
	struct Factorial<28, 28> {
		using type = uint128_t;
		static type getValue() { return type("304888344611713860501504000000"); }
	};

	template<>
	struct Factorial<29, 0> {
		using type = uint8_t;
		static constexpr type getValue() { return type(1u); }
	};

	template<>
	struct Factorial<29, 1> {
		using type = uint8_t;
		static constexpr type getValue() { return type(29u); }
	};

	template<>
	struct Factorial<29, 2> {
		using type = uint16_t;
		static constexpr type getValue() { return type(812u); }
	};

	template<>
	struct Factorial<29, 3> {
		using type = uint16_t;
		static constexpr type getValue() { return type(21924u); }
	};

	template<>
	struct Factorial<29, 4> {
		using type = uint32_t;
		static constexpr type getValue() { return type(570024u); }
	};

	template<>
	struct Factorial<29, 5> {
		using type = uint32_t;
		static constexpr type getValue() { return type(14250600u); }
	};

	template<>
	struct Factorial<29, 6> {
		using type = uint32_t;
		static constexpr type getValue() { return type(342014400u); }
	};

	template<>
	struct Factorial<29, 7> {
		using type = uint64_t;
		static constexpr type getValue() { return type(7866331200u); }
	};

	template<>
	struct Factorial<29, 8> {
		using type = uint64_t;
		static constexpr type getValue() { return type(173059286400u); }
	};

	template<>
	struct Factorial<29, 9> {
		using type = uint64_t;
		static constexpr type getValue() { return type(3634245014400u); }
	};

	template<>
	struct Factorial<29, 10> {
		using type = uint64_t;
		static constexpr type getValue() { return type(72684900288000u); }
	};

	template<>
	struct Factorial<29, 11> {
		using type = uint64_t;
		static constexpr type getValue() { return type(1381013105472000u); }
	};

	template<>
	struct Factorial<29, 12> {
		using type = uint64_t;
		static constexpr type getValue() { return type(24858235898496000u); }
	};

	template<>
	struct Factorial<29, 13> {
		using type = uint64_t;
		static constexpr type getValue() { return type(422590010274432000u); }
	};

	template<>
	struct Factorial<29, 14> {
		using type = uint64_t;
		static constexpr type getValue() { return type(6761440164390912000u); }
	};

	template<>
	struct Factorial<29, 15> {
		using type = uint128_t;
		static type getValue() { return type("101421602465863680000"); }
	};

	template<>
	struct Factorial<29, 16> {
		using type = uint128_t;
		static type getValue() { return type("1419902434522091520000"); }
	};

	template<>
	struct Factorial<29, 17> {
		using type = uint128_t;
		static type getValue() { return type("18458731648787189760000"); }
	};

	template<>
	struct Factorial<29, 18> {
		using type = uint128_t;
		static type getValue() { return type("221504779785446277120000"); }
	};

	template<>
	struct Factorial<29, 19> {
		using type = uint128_t;
		static type getValue() { return type("2436552577639909048320000"); }
	};

	template<>
	struct Factorial<29, 20> {
		using type = uint128_t;
		static type getValue() { return type("24365525776399090483200000"); }
	};

	template<>
	struct Factorial<29, 21> {
		using type = uint128_t;
		static type getValue() { return type("219289731987591814348800000"); }
	};

	template<>
	struct Factorial<29, 22> {
		using type = uint128_t;
		static type getValue() { return type("1754317855900734514790400000"); }
	};

	template<>
	struct Factorial<29, 23> {
		using type = uint128_t;
		static type getValue() { return type("12280224991305141603532800000"); }
	};

	template<>
	struct Factorial<29, 24> {
		using type = uint128_t;
		static type getValue() { return type("73681349947830849621196800000"); }
	};

	template<>
	struct Factorial<29, 25> {
		using type = uint128_t;
		static type getValue() { return type("368406749739154248105984000000"); }
	};

	template<>
	struct Factorial<29, 26> {
		using type = uint128_t;
		static type getValue() { return type("1473626998956616992423936000000"); }
	};

	template<>
	struct Factorial<29, 27> {
		using type = uint128_t;
		static type getValue() { return type("4420880996869850977271808000000"); }
	};

	template<>
	struct Factorial<29, 28> {
		using type = uint128_t;
		static type getValue() { return type("8841761993739701954543616000000"); }
	};

	template<>
	struct Factorial<29, 29> {
		using type = uint128_t;
		static type getValue() { return type("8841761993739701954543616000000"); }
	};


}}

