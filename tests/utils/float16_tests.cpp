#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "../../modules/utils/types/BFloat16.h"
#include <limits>
#include <cmath>



TEST_CASE("bfloat16_bfloat16_detail_builtin_isinf", "[float16]"){

	// Does not work with CPP11_CMATH, works

	//numeric Limits
	bfloat16::bfloat16 inf = std::numeric_limits<bfloat16::bfloat16>::infinity();	//0x7F80
	bfloat16::bfloat16 QNaN = std::numeric_limits<bfloat16::bfloat16>::quiet_NaN();	//0x7FFF
	bfloat16::bfloat16 min = std::numeric_limits<bfloat16::bfloat16>::min();	//0x0080
	bfloat16::bfloat16 low = std::numeric_limits<bfloat16::bfloat16>::lowest();	//0xFF7F
	bfloat16::bfloat16 max = std::numeric_limits<bfloat16::bfloat16>::max();	//0x7F7F
	bfloat16::bfloat16 eps = std::numeric_limits<bfloat16::bfloat16>::epsilon();	//0x0280
	bfloat16::bfloat16 rr = std::numeric_limits<bfloat16::bfloat16>::round_error();	//0x3F00
	bfloat16::bfloat16 SNaN = std::numeric_limits<bfloat16::bfloat16>::signaling_NaN(); //0x7FBF
	bfloat16::bfloat16 den = std::numeric_limits<bfloat16::bfloat16>::denorm_min();	//0x0001

	//numeric Limits tests
  	REQUIRE(true == bfloat16::detail::builtin_isinf(inf) );
	REQUIRE(false == bfloat16::detail::builtin_isinf(QNaN) );
	REQUIRE(false == bfloat16::detail::builtin_isinf(min) );
	REQUIRE(false == bfloat16::detail::builtin_isinf(low) );
	REQUIRE(false == bfloat16::detail::builtin_isinf(max) );
	REQUIRE(false == bfloat16::detail::builtin_isinf(eps) );
	REQUIRE(false == bfloat16::detail::builtin_isinf(rr) );
	REQUIRE(false == bfloat16::detail::builtin_isinf(SNaN) );
	REQUIRE(false == bfloat16::detail::builtin_isinf(den) );

	//positive
	REQUIRE(false == bfloat16::detail::builtin_isinf(bfloat16::bfloat16(0.10)) );
	REQUIRE(false == bfloat16::detail::builtin_isinf(bfloat16::bfloat16(0.5)) );
	REQUIRE(false == bfloat16::detail::builtin_isinf(bfloat16::bfloat16(5)) );
	REQUIRE(false == bfloat16::detail::builtin_isinf(bfloat16::bfloat16(10)) );
	REQUIRE(false == bfloat16::detail::builtin_isinf(bfloat16::bfloat16(10.1)) );
	REQUIRE(false == bfloat16::detail::builtin_isinf(bfloat16::bfloat16(10.5)) );

	//negative
	REQUIRE(false == bfloat16::detail::builtin_isinf(bfloat16::bfloat16(-0.10)) );
	REQUIRE(false == bfloat16::detail::builtin_isinf(bfloat16::bfloat16(-0.5)) );
	REQUIRE(false == bfloat16::detail::builtin_isinf(bfloat16::bfloat16(-5)) );
	REQUIRE(false == bfloat16::detail::builtin_isinf(bfloat16::bfloat16(-10)) );
	REQUIRE(false == bfloat16::detail::builtin_isinf(bfloat16::bfloat16(-10.1)) );
	REQUIRE(false == bfloat16::detail::builtin_isinf(bfloat16::bfloat16(-10.5)) );

}



TEST_CASE("bfloat16_bfloat16_detail_builtin_isnan", "[float16]"){

	// Does not work with CPP11_CMATH, works

	//numeric Limits
	bfloat16::bfloat16 inf = std::numeric_limits<bfloat16::bfloat16>::infinity();	//0x7F80
	bfloat16::bfloat16 QNaN = std::numeric_limits<bfloat16::bfloat16>::quiet_NaN();	//0x7FFF
	bfloat16::bfloat16 min = std::numeric_limits<bfloat16::bfloat16>::min();	//0x0080
	bfloat16::bfloat16 low = std::numeric_limits<bfloat16::bfloat16>::lowest();	//0xFF7F
	bfloat16::bfloat16 max = std::numeric_limits<bfloat16::bfloat16>::max();	//0x7F7F
	bfloat16::bfloat16 eps = std::numeric_limits<bfloat16::bfloat16>::epsilon();	//0x0280
	bfloat16::bfloat16 rr = std::numeric_limits<bfloat16::bfloat16>::round_error();	//0x3F00
	bfloat16::bfloat16 SNaN = std::numeric_limits<bfloat16::bfloat16>::signaling_NaN(); //0x7FBF
	bfloat16::bfloat16 den = std::numeric_limits<bfloat16::bfloat16>::denorm_min();	//0x0001
	
	//numeric Limits tests
  	REQUIRE(false == bfloat16::detail::builtin_isnan(inf) );
	REQUIRE(false == bfloat16::detail::builtin_isnan(min) );
	REQUIRE(false == bfloat16::detail::builtin_isnan(low) );
	REQUIRE(false == bfloat16::detail::builtin_isnan(max) );
	REQUIRE(false == bfloat16::detail::builtin_isnan(eps) );
	REQUIRE(false == bfloat16::detail::builtin_isnan(rr) );
	REQUIRE(false == bfloat16::detail::builtin_isnan(den) );
	REQUIRE(true == bfloat16::detail::builtin_isnan(SNaN) );
	REQUIRE(true == bfloat16::detail::builtin_isnan(QNaN) );

	//positive
	REQUIRE(false == bfloat16::detail::builtin_isnan(bfloat16::bfloat16(0.10)) );
	REQUIRE(false == bfloat16::detail::builtin_isnan(bfloat16::bfloat16(0.5)) );
	REQUIRE(false == bfloat16::detail::builtin_isnan(bfloat16::bfloat16(5)) );
	REQUIRE(false == bfloat16::detail::builtin_isnan(bfloat16::bfloat16(10)) );
	REQUIRE(false == bfloat16::detail::builtin_isnan(bfloat16::bfloat16(10.1)) );
	REQUIRE(false == bfloat16::detail::builtin_isnan(bfloat16::bfloat16(10.5)) );

	//negative
	REQUIRE(false == bfloat16::detail::builtin_isnan(bfloat16::bfloat16(-0.10)) );
	REQUIRE(false == bfloat16::detail::builtin_isnan(bfloat16::bfloat16(-0.5)) );
	REQUIRE(false == bfloat16::detail::builtin_isnan(bfloat16::bfloat16(-5)) );
	REQUIRE(false == bfloat16::detail::builtin_isnan(bfloat16::bfloat16(-10)) );
	REQUIRE(false == bfloat16::detail::builtin_isnan(bfloat16::bfloat16(-10.1)) );
	REQUIRE(false == bfloat16::detail::builtin_isnan(bfloat16::bfloat16(-10.5)) );

}


TEST_CASE("bfloat16_bfloat16_detail_builtin_signbit", "[float16]"){

	// Does not work with CPP11_CMATH, otherwise: works

	//numeric Limits
	bfloat16::bfloat16 inf = std::numeric_limits<bfloat16::bfloat16>::infinity();	//0x7F80
	bfloat16::bfloat16 QNaN = std::numeric_limits<bfloat16::bfloat16>::quiet_NaN();	//0x7FFF
	bfloat16::bfloat16 min = std::numeric_limits<bfloat16::bfloat16>::min();	//0x0080
	bfloat16::bfloat16 low = std::numeric_limits<bfloat16::bfloat16>::lowest();	//0xFF7F
	bfloat16::bfloat16 max = std::numeric_limits<bfloat16::bfloat16>::max();	//0x7F7F
	bfloat16::bfloat16 eps = std::numeric_limits<bfloat16::bfloat16>::epsilon();	//0x0280
	bfloat16::bfloat16 rr = std::numeric_limits<bfloat16::bfloat16>::round_error();	//0x3F00
	bfloat16::bfloat16 SNaN = std::numeric_limits<bfloat16::bfloat16>::signaling_NaN(); //0x7FBF
	bfloat16::bfloat16 den = std::numeric_limits<bfloat16::bfloat16>::denorm_min();	//0x0001

	//numeric Limits tests
	REQUIRE(false == bfloat16::detail::builtin_signbit(inf) );
	REQUIRE(false == bfloat16::detail::builtin_signbit(QNaN) );
	REQUIRE(false == bfloat16::detail::builtin_signbit(min) );
	REQUIRE(true == bfloat16::detail::builtin_signbit(low) );
	REQUIRE(false == bfloat16::detail::builtin_signbit(max) );
	REQUIRE(false == bfloat16::detail::builtin_signbit(eps) );
	REQUIRE(false == bfloat16::detail::builtin_signbit(rr) );
	REQUIRE(false == bfloat16::detail::builtin_signbit(SNaN) );
	REQUIRE(false == bfloat16::detail::builtin_signbit(den) );


	REQUIRE(false == bfloat16::detail::builtin_signbit(bfloat16::bfloat16(0.10)) );
	REQUIRE(false == bfloat16::detail::builtin_signbit(bfloat16::bfloat16(0.5)) );
	REQUIRE(false == bfloat16::detail::builtin_signbit(bfloat16::bfloat16(5)) );
	REQUIRE(false == bfloat16::detail::builtin_signbit(bfloat16::bfloat16(10)) );
	REQUIRE(false == bfloat16::detail::builtin_signbit(bfloat16::bfloat16(10.1)) );
	REQUIRE(false == bfloat16::detail::builtin_signbit(bfloat16::bfloat16(10.5)) );


	REQUIRE(true == bfloat16::detail::builtin_signbit(bfloat16::bfloat16(-0.10)) );
	REQUIRE(true == bfloat16::detail::builtin_signbit(bfloat16::bfloat16(-0.5)) );
	REQUIRE(true == bfloat16::detail::builtin_signbit(bfloat16::bfloat16(-5)) );
	REQUIRE(true == bfloat16::detail::builtin_signbit(bfloat16::bfloat16(-10)) );
	REQUIRE(true == bfloat16::detail::builtin_signbit(bfloat16::bfloat16(-10.1)) );
	REQUIRE(true == bfloat16::detail::builtin_signbit(bfloat16::bfloat16(-10.5)) );

}



TEST_CASE("bfloat16_bfloat16_detail_sign_mask", "[float16]"){

	// works
	
	//numeric Limits
	bfloat16::detail::uint32 inf = std::numeric_limits<bfloat16::bfloat16>::infinity();	//0x7F80
	bfloat16::detail::uint32 QNaN = std::numeric_limits<bfloat16::bfloat16>::quiet_NaN();	//0x7FFF
	bfloat16::detail::uint32 min = std::numeric_limits<bfloat16::bfloat16>::min();	//0x0080
	bfloat16::detail::uint32 low = std::numeric_limits<bfloat16::bfloat16>::lowest();	//0xFF7F
	bfloat16::detail::uint32 max = std::numeric_limits<bfloat16::bfloat16>::max();	//0x7F7F
	bfloat16::detail::uint32 eps = std::numeric_limits<bfloat16::bfloat16>::epsilon();	//0x0280
	bfloat16::detail::uint32 rr = std::numeric_limits<bfloat16::bfloat16>::round_error();	//0x3F00
	bfloat16::detail::uint32 SNaN = std::numeric_limits<bfloat16::bfloat16>::signaling_NaN(); //0x7FBF
	bfloat16::detail::uint32 den = std::numeric_limits<bfloat16::bfloat16>::denorm_min();	//0x0001

	// sign_mask returns 0 (pos) or -1 (neg), and false = 0
	//numeric Limits tests
	REQUIRE(false == bfloat16::detail::sign_mask(inf) );
	REQUIRE(false == bfloat16::detail::sign_mask(QNaN) );
	REQUIRE(false == bfloat16::detail::sign_mask(min) );
	REQUIRE(false == bfloat16::detail::sign_mask(low) );
	REQUIRE(false == bfloat16::detail::sign_mask(max) );
	REQUIRE(false == bfloat16::detail::sign_mask(eps) );
	REQUIRE(false == bfloat16::detail::sign_mask(rr) );
	REQUIRE(false == bfloat16::detail::sign_mask(SNaN) );
	REQUIRE(false == bfloat16::detail::sign_mask(den) );

	//positive
	REQUIRE(false == bfloat16::detail::sign_mask(bfloat16::detail::uint32(5)) );
	REQUIRE(false == bfloat16::detail::sign_mask(bfloat16::detail::uint32(10)) );

	//negative
	REQUIRE(-1 == bfloat16::detail::sign_mask(bfloat16::detail::uint32(-5)) );
	REQUIRE(-1 == bfloat16::detail::sign_mask(bfloat16::detail::uint32(-10)) );

}



TEST_CASE("bfloat16_bfloat16_detail_arithmetic_shift", "[float16]"){

	// works

	bfloat16::detail::uint32 max = 4294967295; //2^32 - 1

	//Arithmetic Shift right a given number of times
	REQUIRE(4294967295 == bfloat16::detail::arithmetic_shift(max, 0 ) );
	REQUIRE(2147483647 == bfloat16::detail::arithmetic_shift(max, 1 ) );
	REQUIRE(1073741823 == bfloat16::detail::arithmetic_shift(max, 2 ) );
	REQUIRE(536870911 ==  bfloat16::detail::arithmetic_shift(max, 3 ) );
	REQUIRE(268435455 ==  bfloat16::detail::arithmetic_shift(max, 4 ) );
	REQUIRE(134217727 ==  bfloat16::detail::arithmetic_shift(max, 5 ) );
	REQUIRE(67108863 ==   bfloat16::detail::arithmetic_shift(max, 6 ) );
	REQUIRE(33554431 ==   bfloat16::detail::arithmetic_shift(max, 7 ) );

	REQUIRE(16777215 ==   bfloat16::detail::arithmetic_shift(max, 8 ) );
	REQUIRE(8388607 ==    bfloat16::detail::arithmetic_shift(max, 9 ) );
	REQUIRE(4194303 ==    bfloat16::detail::arithmetic_shift(max, 10) );
	REQUIRE(2097151 ==    bfloat16::detail::arithmetic_shift(max, 11) );
	REQUIRE(1048575 ==	bfloat16::detail::arithmetic_shift(max, 12) );
	REQUIRE(524287 == 	bfloat16::detail::arithmetic_shift(max, 13) );
	REQUIRE(262143 == 	bfloat16::detail::arithmetic_shift(max, 14) );
	REQUIRE(131071 == 	bfloat16::detail::arithmetic_shift(max, 15) );

	REQUIRE(65535 == 	bfloat16::detail::arithmetic_shift(max, 16) );
	REQUIRE(32767 == 	bfloat16::detail::arithmetic_shift(max, 17) );
	REQUIRE(16383 == 	bfloat16::detail::arithmetic_shift(max, 18) );
	REQUIRE(8191 ==       bfloat16::detail::arithmetic_shift(max, 19) );
	REQUIRE(4095 == 	bfloat16::detail::arithmetic_shift(max, 20) );
	REQUIRE(2047 == 	bfloat16::detail::arithmetic_shift(max, 21) );
	REQUIRE(1023 == 	bfloat16::detail::arithmetic_shift(max, 22) );
	REQUIRE(511 == 	bfloat16::detail::arithmetic_shift(max, 23) );
	
	REQUIRE(255 == 	bfloat16::detail::arithmetic_shift(max, 24) );
	REQUIRE(127 == 	bfloat16::detail::arithmetic_shift(max, 25) );
	REQUIRE(63 == 	bfloat16::detail::arithmetic_shift(max, 26) );
	REQUIRE(31 == 	bfloat16::detail::arithmetic_shift(max, 27) );
	REQUIRE(15 == 	bfloat16::detail::arithmetic_shift(max, 28) );
	REQUIRE(7 == 		bfloat16::detail::arithmetic_shift(max, 29) );
	REQUIRE(3 == 		bfloat16::detail::arithmetic_shift(max, 30) );
	REQUIRE(1 == 		bfloat16::detail::arithmetic_shift(max, 31) );

}



TEST_CASE("bfloat16_bfloat16_detail_compsignal", "[float16]"){

	// Both variations work
	
	//numeric Limits
	bfloat16::bfloat16 inf = std::numeric_limits<bfloat16::bfloat16>::infinity();	//0x7F80
	bfloat16::bfloat16 QNaN = std::numeric_limits<bfloat16::bfloat16>::quiet_NaN();	//0x7FFF
	bfloat16::bfloat16 min = std::numeric_limits<bfloat16::bfloat16>::min();	//0x0080
	bfloat16::bfloat16 low = std::numeric_limits<bfloat16::bfloat16>::lowest();	//0xFF7F
	bfloat16::bfloat16 max = std::numeric_limits<bfloat16::bfloat16>::max();	//0x7F7F
	bfloat16::bfloat16 eps = std::numeric_limits<bfloat16::bfloat16>::epsilon();	//0x0280
	bfloat16::bfloat16 rr = std::numeric_limits<bfloat16::bfloat16>::round_error();	//0x3F00
	bfloat16::bfloat16 SNaN = std::numeric_limits<bfloat16::bfloat16>::signaling_NaN(); //0x7FBF
	bfloat16::bfloat16 den = std::numeric_limits<bfloat16::bfloat16>::denorm_min();	//0x0001

	//positive
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(0.10), bfloat16::bfloat16(0.32)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(0.32), bfloat16::bfloat16(0.10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(0.10), inf) );
	REQUIRE(false == bfloat16::detail::compsignal(inf, bfloat16::bfloat16(0.10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(0.10), min) );
	REQUIRE(false == bfloat16::detail::compsignal(min, bfloat16::bfloat16(0.10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(0.10), max) );
	REQUIRE(false == bfloat16::detail::compsignal(max, bfloat16::bfloat16(0.10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(0.10), eps) );
	REQUIRE(false == bfloat16::detail::compsignal(eps, bfloat16::bfloat16(0.10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(0.10), rr) );
	REQUIRE(false == bfloat16::detail::compsignal(rr, bfloat16::bfloat16(0.10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(0.10), den) );
	REQUIRE(false == bfloat16::detail::compsignal(den, bfloat16::bfloat16(0.10)) );
	REQUIRE(true == bfloat16::detail::compsignal(QNaN, bfloat16::bfloat16(0.10)) );
	REQUIRE(true == bfloat16::detail::compsignal(bfloat16::bfloat16(0.10), QNaN) );
	REQUIRE(true == bfloat16::detail::compsignal(SNaN, bfloat16::bfloat16(0.10)) );
	REQUIRE(true == bfloat16::detail::compsignal(bfloat16::bfloat16(0.10), SNaN) );

	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10), bfloat16::bfloat16(32)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(32), bfloat16::bfloat16(10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10), inf) );
	REQUIRE(false == bfloat16::detail::compsignal(inf, bfloat16::bfloat16(10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10), min) );
	REQUIRE(false == bfloat16::detail::compsignal(min, bfloat16::bfloat16(10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10), max) );
	REQUIRE(false == bfloat16::detail::compsignal(max, bfloat16::bfloat16(10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10), eps) );
	REQUIRE(false == bfloat16::detail::compsignal(eps, bfloat16::bfloat16(10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10), rr) );
	REQUIRE(false == bfloat16::detail::compsignal(rr, bfloat16::bfloat16(10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10), den) );
	REQUIRE(false == bfloat16::detail::compsignal(den, bfloat16::bfloat16(10)) );
	REQUIRE(true == bfloat16::detail::compsignal(QNaN, bfloat16::bfloat16(10)) );
	REQUIRE(true == bfloat16::detail::compsignal(bfloat16::bfloat16(10), QNaN) );
	REQUIRE(true == bfloat16::detail::compsignal(SNaN, bfloat16::bfloat16(10)) );
	REQUIRE(true == bfloat16::detail::compsignal(bfloat16::bfloat16(10), SNaN) );

	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10.1), bfloat16::bfloat16(32.1)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(32.1), bfloat16::bfloat16(10.1)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10.1), inf) );
	REQUIRE(false == bfloat16::detail::compsignal(inf, bfloat16::bfloat16(10.1)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10.1), min) );
	REQUIRE(false == bfloat16::detail::compsignal(min, bfloat16::bfloat16(10.1)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10.1), max) );
	REQUIRE(false == bfloat16::detail::compsignal(max, bfloat16::bfloat16(10.1)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10.1), eps) );
	REQUIRE(false == bfloat16::detail::compsignal(eps, bfloat16::bfloat16(10.1)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10.1), rr) );
	REQUIRE(false == bfloat16::detail::compsignal(rr, bfloat16::bfloat16(10.1)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10.1), den) );
	REQUIRE(false == bfloat16::detail::compsignal(den, bfloat16::bfloat16(10.1)) );
	REQUIRE(true == bfloat16::detail::compsignal(QNaN, bfloat16::bfloat16(10.1)) );
	REQUIRE(true == bfloat16::detail::compsignal(bfloat16::bfloat16(10.1), QNaN) );
	REQUIRE(true == bfloat16::detail::compsignal(SNaN, bfloat16::bfloat16(10.1)) );
	REQUIRE(true == bfloat16::detail::compsignal(bfloat16::bfloat16(10.1), SNaN) );

	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10.5), bfloat16::bfloat16(32.5)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(32.5), bfloat16::bfloat16(10.5)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10.5), inf) );
	REQUIRE(false == bfloat16::detail::compsignal(inf, bfloat16::bfloat16(10.5)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10.5), min) );
	REQUIRE(false == bfloat16::detail::compsignal(min, bfloat16::bfloat16(10.5)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10.5), max) );
	REQUIRE(false == bfloat16::detail::compsignal(max, bfloat16::bfloat16(10.5)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10.5), eps) );
	REQUIRE(false == bfloat16::detail::compsignal(eps, bfloat16::bfloat16(10.5)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10.5), rr) );
	REQUIRE(false == bfloat16::detail::compsignal(rr, bfloat16::bfloat16(10.5)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(10.5), den) );
	REQUIRE(false == bfloat16::detail::compsignal(den, bfloat16::bfloat16(10.5)) );
	REQUIRE(true == bfloat16::detail::compsignal(QNaN, bfloat16::bfloat16(10.5)) );
	REQUIRE(true == bfloat16::detail::compsignal(bfloat16::bfloat16(10.5), QNaN) );
	REQUIRE(true == bfloat16::detail::compsignal(SNaN, bfloat16::bfloat16(10.5)) );
	REQUIRE(true == bfloat16::detail::compsignal(bfloat16::bfloat16(10.5), SNaN) );


	//negative
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-0.10), bfloat16::bfloat16(-0.32)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-0.32), bfloat16::bfloat16(-0.10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-0.10), inf) );
	REQUIRE(false == bfloat16::detail::compsignal(inf, bfloat16::bfloat16(-0.10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-0.10), min) );
	REQUIRE(false == bfloat16::detail::compsignal(min, bfloat16::bfloat16(-0.10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-0.10), max) );
	REQUIRE(false == bfloat16::detail::compsignal(max, bfloat16::bfloat16(-0.10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-0.10), eps) );
	REQUIRE(false == bfloat16::detail::compsignal(eps, bfloat16::bfloat16(-0.10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-0.10), rr) );
	REQUIRE(false == bfloat16::detail::compsignal(rr, bfloat16::bfloat16(-0.10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-0.10), den) );
	REQUIRE(false == bfloat16::detail::compsignal(den, bfloat16::bfloat16(-0.10)) );
	REQUIRE(true == bfloat16::detail::compsignal(QNaN, bfloat16::bfloat16(-0.10)) );
	REQUIRE(true == bfloat16::detail::compsignal(bfloat16::bfloat16(-0.10), QNaN) );
	REQUIRE(true == bfloat16::detail::compsignal(SNaN, bfloat16::bfloat16(-0.10)) );
	REQUIRE(true == bfloat16::detail::compsignal(bfloat16::bfloat16(-0.10), SNaN) );

	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10), bfloat16::bfloat16(-32)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-32), bfloat16::bfloat16(-10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10), inf) );
	REQUIRE(false == bfloat16::detail::compsignal(inf, bfloat16::bfloat16(-10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10), min) );
	REQUIRE(false == bfloat16::detail::compsignal(min, bfloat16::bfloat16(-10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10), max) );
	REQUIRE(false == bfloat16::detail::compsignal(max, bfloat16::bfloat16(-10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10), eps) );
	REQUIRE(false == bfloat16::detail::compsignal(eps, bfloat16::bfloat16(-10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10), rr) );
	REQUIRE(false == bfloat16::detail::compsignal(rr, bfloat16::bfloat16(-10)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10), den) );
	REQUIRE(false == bfloat16::detail::compsignal(den, bfloat16::bfloat16(-10)) );
	REQUIRE(true == bfloat16::detail::compsignal(QNaN, bfloat16::bfloat16(-10)) );
	REQUIRE(true == bfloat16::detail::compsignal(bfloat16::bfloat16(-10), QNaN) );
	REQUIRE(true == bfloat16::detail::compsignal(SNaN, bfloat16::bfloat16(-10)) );
	REQUIRE(true == bfloat16::detail::compsignal(bfloat16::bfloat16(-10), SNaN) );

	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10.1), bfloat16::bfloat16(-32.1)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-32.1), bfloat16::bfloat16(-10.1)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10.1), inf) );
	REQUIRE(false == bfloat16::detail::compsignal(inf, bfloat16::bfloat16(-10.1)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10.1), min) );
	REQUIRE(false == bfloat16::detail::compsignal(min, bfloat16::bfloat16(-10.1)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10.1), max) );
	REQUIRE(false == bfloat16::detail::compsignal(max, bfloat16::bfloat16(-10.1)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10.1), eps) );
	REQUIRE(false == bfloat16::detail::compsignal(eps, bfloat16::bfloat16(-10.1)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10.1), rr) );
	REQUIRE(false == bfloat16::detail::compsignal(rr, bfloat16::bfloat16(-10.1)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10.1), den) );
	REQUIRE(false == bfloat16::detail::compsignal(den, bfloat16::bfloat16(-10.1)) );
	REQUIRE(true == bfloat16::detail::compsignal(QNaN, bfloat16::bfloat16(-10.1)) );
	REQUIRE(true == bfloat16::detail::compsignal(bfloat16::bfloat16(-10.1), QNaN) );
	REQUIRE(true == bfloat16::detail::compsignal(SNaN, bfloat16::bfloat16(-10.1)) );
	REQUIRE(true == bfloat16::detail::compsignal(bfloat16::bfloat16(-10.1), SNaN) );

	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10.5), bfloat16::bfloat16(-32.5)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-32.5), bfloat16::bfloat16(-10.5)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10.5), inf) );
	REQUIRE(false == bfloat16::detail::compsignal(inf, bfloat16::bfloat16(-10.5)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10.5), min) );
	REQUIRE(false == bfloat16::detail::compsignal(min, bfloat16::bfloat16(-10.5)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10.5), max) );
	REQUIRE(false == bfloat16::detail::compsignal(max, bfloat16::bfloat16(-10.5)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10.5), eps) );
	REQUIRE(false == bfloat16::detail::compsignal(eps, bfloat16::bfloat16(-10.5)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10.5), rr) );
	REQUIRE(false == bfloat16::detail::compsignal(rr, bfloat16::bfloat16(-10.5)) );
	REQUIRE(false == bfloat16::detail::compsignal(bfloat16::bfloat16(-10.5), den) );
	REQUIRE(false == bfloat16::detail::compsignal(den, bfloat16::bfloat16(-10.5)) );
	REQUIRE(true == bfloat16::detail::compsignal(QNaN, bfloat16::bfloat16(-10.5)) );
	REQUIRE(true == bfloat16::detail::compsignal(bfloat16::bfloat16(-10.5), QNaN) );
	REQUIRE(true == bfloat16::detail::compsignal(SNaN, bfloat16::bfloat16(-10.5)) );
	REQUIRE(true == bfloat16::detail::compsignal(bfloat16::bfloat16(-10.5), SNaN) );


	//numeric Limits tests
	REQUIRE(false == bfloat16::detail::compsignal(inf, min) );
	REQUIRE(false == bfloat16::detail::compsignal(inf, max) );
	REQUIRE(false == bfloat16::detail::compsignal(inf, eps) );
	REQUIRE(false == bfloat16::detail::compsignal(inf, rr) );
	REQUIRE(false == bfloat16::detail::compsignal(inf, den) );
	REQUIRE(false == bfloat16::detail::compsignal(min, max) );
	REQUIRE(false == bfloat16::detail::compsignal(min, eps) );
	REQUIRE(false == bfloat16::detail::compsignal(min, rr) );
	REQUIRE(false == bfloat16::detail::compsignal(min, den) );
	REQUIRE(false == bfloat16::detail::compsignal(max, eps) );
	REQUIRE(false == bfloat16::detail::compsignal(max, rr) );
	REQUIRE(false == bfloat16::detail::compsignal(max, den) );
	REQUIRE(false == bfloat16::detail::compsignal(rr, den) );
	REQUIRE(true == bfloat16::detail::compsignal(QNaN,SNaN) );
	REQUIRE(true == bfloat16::detail::compsignal(SNaN,QNaN) );

	//QNaN
	REQUIRE(true == bfloat16::detail::compsignal(QNaN, inf) );
	REQUIRE(true == bfloat16::detail::compsignal(QNaN, min) );
	REQUIRE(true == bfloat16::detail::compsignal(QNaN, max) );
	REQUIRE(true == bfloat16::detail::compsignal(QNaN, eps) );
	REQUIRE(true == bfloat16::detail::compsignal(QNaN, rr) );
	REQUIRE(true == bfloat16::detail::compsignal(QNaN, den) );
	REQUIRE(true == bfloat16::detail::compsignal(inf, QNaN) );
	REQUIRE(true == bfloat16::detail::compsignal(min, QNaN) );
	REQUIRE(true == bfloat16::detail::compsignal(max, QNaN) );
	REQUIRE(true == bfloat16::detail::compsignal(eps, QNaN) );
	REQUIRE(true == bfloat16::detail::compsignal(rr, QNaN) );
	REQUIRE(true == bfloat16::detail::compsignal(den, QNaN) );

	//SNaN
	REQUIRE(true == bfloat16::detail::compsignal(SNaN, inf) );
	REQUIRE(true == bfloat16::detail::compsignal(SNaN, min) );
	REQUIRE(true == bfloat16::detail::compsignal(SNaN, max) );
	REQUIRE(true == bfloat16::detail::compsignal(SNaN, eps) );
	REQUIRE(true == bfloat16::detail::compsignal(SNaN, rr) );
	REQUIRE(true == bfloat16::detail::compsignal(SNaN, den) );
	REQUIRE(true == bfloat16::detail::compsignal(inf, SNaN) );
	REQUIRE(true == bfloat16::detail::compsignal(min, SNaN) );
	REQUIRE(true == bfloat16::detail::compsignal(max, SNaN) );
	REQUIRE(true == bfloat16::detail::compsignal(eps, SNaN) );
	REQUIRE(true == bfloat16::detail::compsignal(rr, SNaN) );
	REQUIRE(true == bfloat16::detail::compsignal(den, SNaN) );

}



TEST_CASE("bfloat16_bfloat16_detail_signal1", "[float16]"){

	//works

	bfloat16::bfloat16 QNaN = std::numeric_limits<bfloat16::bfloat16>::quiet_NaN();	//0x7FFF
	bfloat16::bfloat16 SNaN = std::numeric_limits<bfloat16::bfloat16>::signaling_NaN(); //0x7FBF

	REQUIRE(0x7FFF == bfloat16::detail::signal(QNaN) );
	REQUIRE(0x7FFF == bfloat16::detail::signal(SNaN) );

}



TEST_CASE("bfloat16_bfloat16_detail_signal2", "[float16]"){

	//works

	bfloat16::bfloat16 QNaN = std::numeric_limits<bfloat16::bfloat16>::quiet_NaN();	//0x7FFF
	bfloat16::bfloat16 SNaN = std::numeric_limits<bfloat16::bfloat16>::signaling_NaN(); //0x7FBF

	REQUIRE(0x7FFF == bfloat16::detail::signal(bfloat16::bfloat16(10), QNaN) );
	REQUIRE(0x7FFF == bfloat16::detail::signal(bfloat16::bfloat16(10), SNaN) );
	REQUIRE(0x7FFF == bfloat16::detail::signal(QNaN, bfloat16::bfloat16(10)) );
	REQUIRE(0x7FFF == bfloat16::detail::signal(SNaN, bfloat16::bfloat16(10)) );

}



TEST_CASE("bfloat16_bfloat16_detail_signal3", "[float16]"){

	//works

	bfloat16::bfloat16 QNaN = std::numeric_limits<bfloat16::bfloat16>::quiet_NaN();	//0x7FFF
	bfloat16::bfloat16 SNaN = std::numeric_limits<bfloat16::bfloat16>::signaling_NaN(); //0x7FBF

	REQUIRE(0x7FFF == bfloat16::detail::signal(QNaN, bfloat16::bfloat16(10), bfloat16::bfloat16(10)) );
	REQUIRE(0x7FFF == bfloat16::detail::signal(SNaN, bfloat16::bfloat16(10), bfloat16::bfloat16(10)) );
	REQUIRE(0x7FFF == bfloat16::detail::signal(bfloat16::bfloat16(10), QNaN, bfloat16::bfloat16(10)) );
	REQUIRE(0x7FFF == bfloat16::detail::signal(bfloat16::bfloat16(10), SNaN, bfloat16::bfloat16(10)) );
	REQUIRE(0x7FFF == bfloat16::detail::signal(bfloat16::bfloat16(10), bfloat16::bfloat16(10), QNaN) );
	REQUIRE(0x7FFF == bfloat16::detail::signal(bfloat16::bfloat16(10), bfloat16::bfloat16(10), SNaN) );

}



TEST_CASE("bfloat16_bfloat16_detail_select", "[float16]"){

	//works

	bfloat16::bfloat16 SNaN = std::numeric_limits<bfloat16::bfloat16>::signaling_NaN(); //0x7FBF

	REQUIRE(0x4120 == bfloat16::detail::select(bfloat16::bfloat16(10), SNaN) ); //0x4120 = 10.0 bf16
	REQUIRE(0xC120 == bfloat16::detail::select(bfloat16::bfloat16(-10), SNaN) ); //0xC120 = -10.0 bf16

}



TEST_CASE("bfloat16_bfloat16_detail_invalid", "[float16]"){

	//works

	REQUIRE(0x7FFF == bfloat16::detail::invalid() ); // 0x7FF = QNaN

}



TEST_CASE("bfloat16_bfloat16_detail_pole", "[float16]"){

	// works

	REQUIRE(0xFF80 == bfloat16::detail::pole(0x8000) );	//0xFF80 - Neg Inf
	REQUIRE(0x7F80 == bfloat16::detail::pole(0x0) );	//0x7F80 = Pos Inf

}



TEST_CASE("bfloat16_bfloat16_detail_check_underflow", "[float16]"){

	// works

	//numeric Limits
	unsigned int inf = std::numeric_limits<bfloat16::bfloat16>::infinity();		//0x7F80
	unsigned int QNaN = std::numeric_limits<bfloat16::bfloat16>::quiet_NaN();	//0x7FFF
	unsigned int min = std::numeric_limits<bfloat16::bfloat16>::min();		//0x0080
	unsigned int low = std::numeric_limits<bfloat16::bfloat16>::lowest();		//0xFF7F
	unsigned int max = std::numeric_limits<bfloat16::bfloat16>::max();		//0x7F7F
	unsigned int eps = std::numeric_limits<bfloat16::bfloat16>::epsilon();		//0x280
	unsigned int rr = std::numeric_limits<bfloat16::bfloat16>::round_error();	//0x3F00
	unsigned int SNaN = std::numeric_limits<bfloat16::bfloat16>::signaling_NaN();	//0x7FBF
	unsigned int den = std::numeric_limits<bfloat16::bfloat16>::denorm_min();	//0x0001

	//numeric Limits
  	REQUIRE(inf == bfloat16::detail::check_underflow(inf) );
	REQUIRE(QNaN == bfloat16::detail::check_underflow(QNaN) );
	REQUIRE(min == bfloat16::detail::check_underflow(min) );
	REQUIRE(low == bfloat16::detail::check_underflow(low) );
	REQUIRE(max == bfloat16::detail::check_underflow(max) );
	REQUIRE(eps == bfloat16::detail::check_underflow(eps) );
	REQUIRE(rr == bfloat16::detail::check_underflow(rr) );
	REQUIRE(SNaN == bfloat16::detail::check_underflow(SNaN) );
	REQUIRE(den == bfloat16::detail::check_underflow(den) );

	//positive
	REQUIRE(0x3DCC == bfloat16::detail::check_underflow(bfloat16::bfloat16(0.10)) ); // 0x3DCC = 0.099609375 bf16
	REQUIRE(0x3F00 == bfloat16::detail::check_underflow(bfloat16::bfloat16(0.5)) ); // 0x3F00 = 0.5 bf16
	REQUIRE(0x40A0 == bfloat16::detail::check_underflow(bfloat16::bfloat16(5)) ); // 0x40A0 = 5.0 bf16
	REQUIRE(0x4120 == bfloat16::detail::check_underflow(bfloat16::bfloat16(10)) ); //0x4120 = 10.0 bf16
	REQUIRE(0x4121 == bfloat16::detail::check_underflow(bfloat16::bfloat16(10.1)) ); // 0x4121 = 10.0625 bf16
	REQUIRE(0x4128 == bfloat16::detail::check_underflow(bfloat16::bfloat16(10.5)) ); // 0x4128 = 10.5 bf16

	
	//negative
	REQUIRE(0xBDCC == bfloat16::detail::check_underflow(bfloat16::bfloat16(-0.10)) ); // 0xBDCC = -0.099609375 bf16
	REQUIRE(0xBF00 == bfloat16::detail::check_underflow(bfloat16::bfloat16(-0.5)) ); // 0xBF00 = -0.5 bf16
	REQUIRE(0xC0A0 == bfloat16::detail::check_underflow(bfloat16::bfloat16(-5)) ); // 0xC0A0 = -5.0 bf16
	REQUIRE(0xC120 == bfloat16::detail::check_underflow(bfloat16::bfloat16(-10)) ); //0xC120 = -10.0 bf16
	REQUIRE(0xC121 == bfloat16::detail::check_underflow(bfloat16::bfloat16(-10.1)) ); // 0xC121 = -10.0625 bf16
	REQUIRE(0xC128 == bfloat16::detail::check_underflow(bfloat16::bfloat16(-10.5)) ); // 0xC128 = -10.5 bf16

}



TEST_CASE("bfloat16_bfloat16_detail_overflow", "[float16]"){

	//works

	REQUIRE(0x7F80 == bfloat16::detail::overflow<std::round_toward_infinity>(0x0) ); // pos inf
	REQUIRE(0xFF7F == bfloat16::detail::overflow<std::round_toward_infinity>(0x8000) );// lowest

	REQUIRE(0x7F7F == bfloat16::detail::overflow<std::round_toward_neg_infinity>(0x0) );// max
	REQUIRE(0xFF80 == bfloat16::detail::overflow<std::round_toward_neg_infinity>(0x8000) ); // neg inf

	REQUIRE(0x7F7F == bfloat16::detail::overflow<std::round_toward_zero>(0x0) ); // max
	REQUIRE(0xFF7F == bfloat16::detail::overflow<std::round_toward_zero>(0x8000) ); // lowest

	REQUIRE(0x7F80 == bfloat16::detail::overflow<std::round_to_nearest>(0x0) ); // pos inf
	REQUIRE(0xFF80 == bfloat16::detail::overflow<std::round_to_nearest>(0x8000) ); // neg inf

}



TEST_CASE("bfloat16_bfloat16_detail_underflow", "[float16]"){

	//works

	REQUIRE(0x7F80 == bfloat16::detail::overflow<std::round_toward_infinity>(0x0) ); // pos inf
	REQUIRE(0xFF7F == bfloat16::detail::overflow<std::round_toward_infinity>(0x8000) ); // lowest

	REQUIRE(0x7F7F == bfloat16::detail::overflow<std::round_toward_neg_infinity>(0x0) ); // lowest
	REQUIRE(0xFF80 == bfloat16::detail::overflow<std::round_toward_neg_infinity>(0x8000) ); // neg inf

}



TEST_CASE("bfloat16_bfloat16_detail_rounded", "[float16]"){

	//works

	bfloat16::bfloat16 five = bfloat16::bfloat16(5);
	bfloat16::bfloat16 negfive = bfloat16::bfloat16(-5);
	bfloat16::bfloat16 ten = bfloat16::bfloat16(10);
	bfloat16::bfloat16 negten = bfloat16::bfloat16(-10);
	bfloat16::bfloat16 tenone = bfloat16::bfloat16(10.1);
	bfloat16::bfloat16 negtenone = bfloat16::bfloat16(-10.1);
	bfloat16::bfloat16 tenfive = bfloat16::bfloat16(10.5);
	bfloat16::bfloat16 negtenfive = bfloat16::bfloat16(-10.5);
	bfloat16::bfloat16 pointten = bfloat16::bfloat16(0.10);
	bfloat16::bfloat16 negpointten = bfloat16::bfloat16(-0.10);
	bfloat16::bfloat16 pointfive = bfloat16::bfloat16(0.5);
	bfloat16::bfloat16 negpointfive = bfloat16::bfloat16(-0.5);

	unsigned int val;

	//positive
	// Tests for bfloat16(0.10)
	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(pointten, 0,1);
	REQUIRE(0x3DCC == val ); // 0x3DCC = 0.099609375 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(pointten, 0,1);
	REQUIRE(0x3DCD == val ); // 0x3DCD = 0.10009765625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(pointten, 0,1);
	REQUIRE(0x3DCC == val ); // 0x3DCC = 0.099609375 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(pointten, 1,1);
	REQUIRE(0x3DCC == val ); // 0x3DCC = 0.099609375 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(pointten, 1,1);
	REQUIRE(0x3DCD == val ); // 0x3DCD = 0.10009765625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(pointten, 1,1);
	REQUIRE(0x3DCD == val ); // 0x3DCD = 0.10009765625 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(pointten, 0,1);
	REQUIRE(0x3DCC == val ); // 0x3DCC = 0.099609375 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(pointten, 0,1);
	REQUIRE(0x3DCD == val ); // 0x3DCD = 0.10009765625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(pointten, 0,1);
	REQUIRE(0x3DCC == val ); // 0x3DCC = 0.099609375 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(pointten, 1,1);
	REQUIRE(0x3DCC == val ); // 0x3DCC = 0.099609375 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(pointten, 1,1);
	REQUIRE(0x3DCD == val ); // 0x3DCD = 0.10009765625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(pointten, 1,1);
	REQUIRE(0x3DCD == val ); // 0x3DCD = 0.10009765625 bf16


	// Tests for bfloat16(0.5)
	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(pointfive, 0,1);
	REQUIRE(0x3F00 == val ); // 0x3F00 = 0.5 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(pointfive, 0,1);
	REQUIRE(0x3F01 == val ); // 0x3F01 = 0.50390625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(pointfive, 0,1);
	REQUIRE(0x3F00 == val ); // 0x3F00 = 0.5 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(pointfive, 1,1);
	REQUIRE(0x3F00 == val ); // 0x3F00 = 0.5 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(pointfive, 1,1);
	REQUIRE(0x3F01 == val ); // 0x3F01 = 0.50390625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(pointfive, 1,1);
	REQUIRE(0x3F01 == val ); // 0x3F01 = 0.50390625 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(pointfive, 0,1);
	REQUIRE(0x3F00 == val ); // 0x3F00 = 0.5 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(pointfive, 0,1);
	REQUIRE(0x3F01 == val ); // 0x3F01 = 0.50390625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(pointfive, 0,1);
	REQUIRE(0x3F00 == val ); // 0x3F00 = 0.5 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(pointfive, 1,1);
	REQUIRE(0x3F00 == val ); // 0x3F00 = 0.5 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(pointfive, 1,1);
	REQUIRE(0x3F01 == val ); // 0x3F01 = 0.50390625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(pointfive, 1,1);
	REQUIRE(0x3F01 == val ); // 0x3F01 = 0.50390625 bf16


	// Tests for bfloat16(5)
	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(five, 0,1);
	REQUIRE(0x40A0 == val ); // 0x40A0 = 5.0 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(five, 0,1);
	REQUIRE(0x40A1 == val ); // 0x40A1 = 5.03125 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(five, 0,1);
	REQUIRE(0x40A0 == val ); // 0x40A0 = 5.0 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(five, 1,1);
	REQUIRE(0x40A0 == val ); // 0x40A0 = 5.0 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(five, 1,1);
	REQUIRE(0x40A1 == val ); // 0x40A1 = 5.03125 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(five, 1,1);
	REQUIRE(0x40A1 == val ); // 0x40A1 = 5.03125 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(five, 0,1);
	REQUIRE(0x40A0 == val ); // 0x40A0 = 5.0 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(five, 0,1);
	REQUIRE(0x40A1 == val ); // 0x40A1 = 5.03125 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(five, 0,1);
	REQUIRE(0x40A0 == val ); // 0x40A0 = 5.0 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(five, 1,1);
	REQUIRE(0x40A0 == val ); // 0x40A0 = 5.0 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(five, 1,1);
	REQUIRE(0x40A1 == val ); // 0x40A1 = 5.03125 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(five, 1,1);
	REQUIRE(0x40A1 == val ); // 0x40A1 = 5.03125 bf16


	// Tests for bfloat16(10)
	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(ten, 0,1);
	REQUIRE(0x4120 == val ); // 0x4120 = 10.0 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(ten, 0,1);
	REQUIRE(0x4121 == val ); // 0x4121 = 10.0625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(ten, 0,1);
	REQUIRE(0x4120 == val ); // 0x4120 = 10.0 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(ten, 1,1);
	REQUIRE(0x4120 == val ); // 0x4120 = 10.0 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(ten, 1,1);
	REQUIRE(0x4121 == val ); // 0x4121 = 10.0625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(ten, 1,1);
	REQUIRE(0x4121 == val ); // 0x4121 = 10.0625 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(ten, 0,1);
	REQUIRE(0x4120 == val ); // 0x4120 = 10.0 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(ten, 0,1);
	REQUIRE(0x4121 == val ); // 0x4121 = 10.0625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(ten, 0,1);
	REQUIRE(0x4120 == val ); // 0x4120 = 10.0 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(ten, 1,1);
	REQUIRE(0x4120 == val ); // 0x4120 = 10.0 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(ten, 1,1);
	REQUIRE(0x4121 == val ); // 0x4121 = 10.0625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(ten, 1,1);
	REQUIRE(0x4121 == val ); // 0x4121 = 10.0625 bf16


	// Tests for bfloat16(10.1)
	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(tenone, 0,1);
	REQUIRE(0x4121 == val ); // 0x4121 = 10.0625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(tenone, 0,1);
	REQUIRE(0x4122 == val ); // 0x4122 = 10.125 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(tenone, 0,1);
	REQUIRE(0x4121 == val ); // 0x4121 = 10.0625 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(tenone, 1,1);
	REQUIRE(0x4121 == val ); // 0x4121 = 10.0625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(tenone, 1,1);
	REQUIRE(0x4122 == val ); // 0x4122 = 10.125 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(tenone, 1,1);
	REQUIRE(0x4122 == val ); // 0x4122 = 10.125 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(tenone, 0,1);
	REQUIRE(0x4121 == val ); // 0x4121 = 10.0625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(tenone, 0,1);
	REQUIRE(0x4122 == val ); // 0x4122 = 10.125 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(tenone, 0,1);
	REQUIRE(0x4121 == val ); // 0x4121 = 10.0625 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(tenone, 1,1);
	REQUIRE(0x4121 == val ); // 0x4121 = 10.0625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(tenone, 1,1);
	REQUIRE(0x4122 == val ); // 0x4122 = 10.125 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(tenone, 1,1);
	REQUIRE(0x4122 == val ); // 0x4122 = 10.125 bf16


	// Tests for bfloat16(10.5)
	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(tenfive, 0,1);
	REQUIRE(0x4128 == val ); // 0x4128 = 10.5 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(tenfive, 0,1);
	REQUIRE(0x4129 == val ); // 0x4129 = 10.5625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(tenfive, 0,1);
	REQUIRE(0x4128 == val ); // 0x4128 = 10.5 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(tenfive, 1,1);
	REQUIRE(0x4128 == val ); // 0x4128 = 10.5 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(tenfive, 1,1);
	REQUIRE(0x4129 == val ); // 0x4129 = 10.5625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(tenfive, 1,1);
	REQUIRE(0x4129 == val ); // 0x4129 = 10.5625 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(tenfive, 0,1);
	REQUIRE(0x4128 == val ); // 0x4128 = 10.5 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(tenfive, 0,1);
	REQUIRE(0x4129 == val ); // 0x4129 = 10.5625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(tenfive, 0,1);
	REQUIRE(0x4128 == val ); // 0x4128 = 10.5 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(tenfive, 1,1);
	REQUIRE(0x4128 == val ); // 0x4128 = 10.5 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(tenfive, 1,1);
	REQUIRE(0x4129 == val ); // 0x4129 = 10.5625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(tenfive, 1,1);
	REQUIRE(0x4129 == val ); // 0x4129 = 10.5625 bf16





	//negative
	// Tests for bfloat16(-0.10)
	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(negpointten, 0,1);
	REQUIRE(0xBDCD == val ); // 0xBDCD = -0.10009765625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(negpointten, 0,1);
	REQUIRE(0xBDCC == val ); // 0xBDCC = -0.099609375 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(negpointten, 0,1);
	REQUIRE(0xBDCC == val ); // 0xBDCC = -0.099609375 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(negpointten, 1,1);
	REQUIRE(0xBDCD == val ); // 0xBDCD = -0.10009765625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(negpointten, 1,1);
	REQUIRE(0xBDCC == val ); // 0xBDCC = -0.099609375 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(negpointten, 1,1);
	REQUIRE(0xBDCD == val ); // 0xBDCD = -0.10009765625 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(negpointten, 0,1);
	REQUIRE(0xBDCD == val ); // 0xBDCD = -0.10009765625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(negpointten, 0,1);
	REQUIRE(0xBDCC == val ); // 0xBDCC = -0.099609375 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(negpointten, 0,1);
	REQUIRE(0xBDCC == val ); // 0xBDCC = -0.099609375 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(negpointten, 1,1);
	REQUIRE(0xBDCD == val ); // 0xBDCD = -0.10009765625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(negpointten, 1,1);
	REQUIRE(0xBDCC == val ); // 0xBDCC = -0.099609375 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(negpointten, 1,1);
	REQUIRE(0xBDCD == val ); // 0xBDCD = -0.10009765625 bf16


	// Tests for bfloat16(-0.5)
	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(negpointfive, 0,1);
	REQUIRE(0xBF01 == val ); // 0xBF01 = -0.50390625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(negpointfive, 0,1);
	REQUIRE(0xBF00 == val ); // 0xBF00 = -0.5 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(negpointfive, 0,1);
	REQUIRE(0xBF00 == val ); // 0xBF00 = -0.5 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(negpointfive, 1,1);
	REQUIRE(0xBF01 == val ); // 0xBF01 = -0.50390625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(negpointfive, 1,1);
	REQUIRE(0xBF00 == val ); // 0xBF00 = -0.5 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(negpointfive, 1,1);
	REQUIRE(0xBF01 == val ); // 0xBF01 = -0.50390625 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(negpointfive, 0,1);
	REQUIRE(0xBF01 == val ); // 0xBF01 = -0.50390625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(negpointfive, 0,1);
	REQUIRE(0xBF00 == val ); // 0xBF00 = -0.5 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(negpointfive, 0,1);
	REQUIRE(0xBF00 == val ); // 0xBF00 = -0.5 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(negpointfive, 1,1);
	REQUIRE(0xBF01 == val ); // 0xBF01 = -0.50390625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(negpointfive, 1,1);
	REQUIRE(0xBF00 == val ); // 0xBF00 = -0.5 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(negpointfive, 1,1);
	REQUIRE(0xBF01 == val ); // 0xBF01 = -0.50390625 bf16


	// Tests for bfloat16(-5)
	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(negfive, 0,1);
	REQUIRE(0xC0A1 == val ); // 0xC0A1 = -5.03125 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(negfive, 0,1);
	REQUIRE(0xC0A0 == val ); // 0xC0A0 = -5.0 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(negfive, 0,1);
	REQUIRE(0xC0A0 == val ); // 0xC0A0 = -5.0 bf16
 
	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(negfive, 1,1);
	REQUIRE(0xC0A1 == val ); // 0xC0A1 = -5.03125 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(negfive, 1,1);
	REQUIRE(0xC0A0 == val ); // 0xC0A0 = -5.0 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(negfive, 1,1);
	REQUIRE(0xC0A1 == val ); // 0xC0A1 = -5.03125 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(negfive, 0,1);
	REQUIRE(0xC0A1 == val ); // 0xC0A1 = -5.03125 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(negfive, 0,1);
	REQUIRE(0xC0A0 == val ); // 0xC0A0 = -5.0 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(negfive, 0,1);
	REQUIRE(0xC0A0 == val ); // 0xC0A0 = -5.0 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(negfive, 1,1);
	REQUIRE(0xC0A1 == val ); // 0xC0A1 = -5.03125 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(negfive, 1,1);
	REQUIRE(0xC0A0 == val ); // 0xC0A0 = -5.0 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(negfive, 1,1);
	REQUIRE(0xC0A1 == val ); // 0xC0A1 = -5.03125 bf16


	// Tests for bfloat16(-10)
	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(negten, 0,1);
	REQUIRE(0xC121 == val ); // 0xC121 = -10.0625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(negten, 0,1);
	REQUIRE(0xC120 == val ); // 0xC120 = -10.0 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(negten, 0,1);
	REQUIRE(0xC120 == val ); // 0xC120 = -10.0 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(negten, 1,1);
	REQUIRE(0xC121 == val ); // 0xC121 = -10.0625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(negten, 1,1);
	REQUIRE(0xC120 == val ); // 0xC120 = -10.0 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(negten, 1,1);
	REQUIRE(0xC121 == val ); // 0xC121 = -10.0625 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(negten, 0,1);
	REQUIRE(0xC121 == val ); // 0xC121 = -10.0625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(negten, 0,1);
	REQUIRE(0xC120 == val ); // 0xC120 = -10.0 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(negten, 0,1);
	REQUIRE(0xC120 == val ); // 0xC120 = -10.0 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(negten, 1,1);
	REQUIRE(0xC121 == val ); // 0xC121 = -10.0625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(negten, 1,1);
	REQUIRE(0xC120 == val ); // 0xC120 = -10.0 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(negten, 1,1);
	REQUIRE(0xC121 == val ); // 0xC121 = -10.0625 bf16


	// Tests for bfloat16(-10.1)
	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(negtenone, 0,1);
	REQUIRE(0xC122 == val ); // 0xC122 = -10.125 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(negtenone, 0,1);
	REQUIRE(0xC121 == val ); // 0xC121 = -10.0625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(negtenone, 0,1);
	REQUIRE(0xC121 == val ); // 0xC121 = -10.0625 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(negtenone, 1,1);
	REQUIRE(0xC122 == val ); // 0xC122 = -10.125 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(negtenone, 1,1);
	REQUIRE(0xC121 == val ); // 0xC121 = -10.0625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(negtenone, 1,1);
	REQUIRE(0xC122 == val ); // 0xC122 = -10.125 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(negtenone, 0,1);
	REQUIRE(0xC122 == val ); // 0xC122 = -10.125 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(negtenone, 0,1);
	REQUIRE(0xC121 == val ); // 0xC121 = -10.0625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(negtenone, 0,1);
	REQUIRE(0xC121 == val ); // 0xC121 = -10.0625 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(negtenone, 1,1);
	REQUIRE(0xC122 == val ); // 0xC122 = -10.125 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(negtenone, 1,1);
	REQUIRE(0xC121 == val ); // 0xC121 = -10.0625 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(negtenone, 1,1);
	REQUIRE(0xC122 == val ); // 0xC122 = -10.125 bf16


	// Tests for bfloat16(-10.5)
	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(negtenfive, 0,1);
	REQUIRE(0xC129 == val ); // 0xC129 = -10.5625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(negtenfive, 0,1);
	REQUIRE(0xC128 == val ); // 0xC128 = -10.5 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(negtenfive, 0,1);
	REQUIRE(0xC128 == val ); // 0xC128 = -10.5 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, false>(negtenfive, 1,1);
	REQUIRE(0xC129 == val ); // 0xC129 = -10.5625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, false>(negtenfive, 1,1);
	REQUIRE(0xC128 == val ); // 0xC128 = -10.5 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, false>(negtenfive, 1,1);
	REQUIRE(0xC129 == val ); // 0xC129 = -10.5625 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(negtenfive, 0,1);
	REQUIRE(0xC129 == val ); // 0xC129 = -10.5625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(negtenfive, 0,1);
	REQUIRE(0xC128 == val ); // 0xC128 = -10.5 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(negtenfive, 0,1);
	REQUIRE(0xC128 == val ); // 0xC128 = -10.5 bf16

	val = bfloat16::detail::rounded<std::round_toward_neg_infinity, true>(negtenfive, 1,1);
	REQUIRE(0xC129 == val ); // 0xC129 = -10.5625 bf16
	val = bfloat16::detail::rounded<std::round_toward_infinity, true>(negtenfive, 1,1);
	REQUIRE(0xC128 == val ); // 0xC128 = -10.5 bf16
	val = bfloat16::detail::rounded<std::round_to_nearest, true>(negtenfive, 1,1);
	REQUIRE(0xC129 == val ); // 0xC129 = -10.5625 bf16

}



TEST_CASE("bfloat16_bfloat16_detail_integral", "[float16]"){

	//works

	unsigned int val;

	//positive
	//for bfloat16(0.3)
	val =bfloat16::detail::integral<std::round_to_nearest , false, false>(bfloat16::bfloat16(0.3));
	REQUIRE(0x0 == val ); // 0x0 = 0.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , false, false>(bfloat16::bfloat16(0.3));
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , false, false>(bfloat16::bfloat16(0.3));
	REQUIRE(0x0 == val ); // 0x0 = 0.0 bf16

	val =bfloat16::detail::integral<std::round_to_nearest , true, false>(bfloat16::bfloat16(0.3));
	REQUIRE(0x0 == val ); // 0x0 = 0.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , true, false>(bfloat16::bfloat16(0.3));
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , true, false>(bfloat16::bfloat16(0.3));
	REQUIRE(0x0 == val ); // 0x0 = 0.0 bf16


	//for bfloat16(0.5)
	val =bfloat16::detail::integral<std::round_to_nearest , false, false>(bfloat16::bfloat16(0.5));
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , false, false>(bfloat16::bfloat16(0.5));
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , false, false>(bfloat16::bfloat16(0.5));
	REQUIRE(0x0 == val ); // 0x0 = 0.0 bf16

	val =bfloat16::detail::integral<std::round_to_nearest , true, false>(bfloat16::bfloat16(0.5));
	REQUIRE(0x0 == val ); // 0x0 = 0.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , true, false>(bfloat16::bfloat16(0.5));
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , true, false>(bfloat16::bfloat16(0.5));
	REQUIRE(0x0 == val ); // 0x0 = 0.0 bf16


	//for bfloat16(0.8)
	val =bfloat16::detail::integral<std::round_to_nearest , false, false>(bfloat16::bfloat16(0.8));
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , false, false>(bfloat16::bfloat16(0.8));
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , false, false>(bfloat16::bfloat16(0.8));
	REQUIRE(0x0 == val ); // 0x0 = 0.0 bf16

	val =bfloat16::detail::integral<std::round_to_nearest , true, false>(bfloat16::bfloat16(0.8));
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , true, false>(bfloat16::bfloat16(0.8));
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , true, false>(bfloat16::bfloat16(0.8));
	REQUIRE(0x0 == val ); // 0x0 = 0.0 bf16



	//for bfloat16(6.3)
	val =bfloat16::detail::integral<std::round_to_nearest , false, false>(bfloat16::bfloat16(6.3));
	REQUIRE(0x40C0 == val ); // 0x40C0 = 6.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , false, false>(bfloat16::bfloat16(6.3));
	REQUIRE(0x40E0 == val ); // 0x40E0 = 7.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , false, false>(bfloat16::bfloat16(6.3));
	REQUIRE(0x40C0 == val ); // 0x40C0 = 6.0 bf16

	val =bfloat16::detail::integral<std::round_to_nearest , true, false>(bfloat16::bfloat16(6.3));
	REQUIRE(0x40C0 == val ); // 0x40C0 = 6.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , true, false>(bfloat16::bfloat16(6.3));
	REQUIRE(0x40E0 == val ); // 0x40E0 = 7.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , true, false>(bfloat16::bfloat16(6.3));
	REQUIRE(0x40C0 == val ); // 0x40C0 = 6.0 bf16


	//for bfloat16(6.5)
	val =bfloat16::detail::integral<std::round_to_nearest , false, false>(bfloat16::bfloat16(6.5));
	REQUIRE(0x40C0 == val ); // 0x40C0 = 6.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , false, false>(bfloat16::bfloat16(6.5));
	REQUIRE(0x40E0 == val ); // 0x40E0 = 7.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , false, false>(bfloat16::bfloat16(6.5));
	REQUIRE(0x40C0 == val ); // 0x40C0 = 6.0 bf16

	val =bfloat16::detail::integral<std::round_to_nearest , true, false>(bfloat16::bfloat16(6.5));
	REQUIRE(0x40E0 == val ); // 0x40E0 = 7.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , true, false>(bfloat16::bfloat16(6.5));
	REQUIRE(0x40E0 == val ); // 0x40E0 = 7.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , true, false>(bfloat16::bfloat16(6.5));
	REQUIRE(0x40C0 == val ); // 0x40C0 = 6.0 bf16


	//for bfloat16(6.8)
	val =bfloat16::detail::integral<std::round_to_nearest , false, false>(bfloat16::bfloat16(6.8));
	REQUIRE(0x40E0 == val ); // 0x40E0 = 7.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , false, false>(bfloat16::bfloat16(6.8));
	REQUIRE(0x40E0 == val ); // 0x40E0 = 7.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , false, false>(bfloat16::bfloat16(6.8));
	REQUIRE(0x40C0 == val ); // 0x40C0 = 6.0 bf16

	val =bfloat16::detail::integral<std::round_to_nearest , true, false>(bfloat16::bfloat16(6.8));
	REQUIRE(0x40E0 == val ); // 0x40E0 = 7.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , true, false>(bfloat16::bfloat16(6.8));
	REQUIRE(0x40E0 == val ); // 0x40E0 = 7.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , true, false>(bfloat16::bfloat16(6.8));
	REQUIRE(0x40C0 == val ); // 0x40C0 = 6.0 bf16




	//negative
	//for bfloat16(-0.3)
	val =bfloat16::detail::integral<std::round_to_nearest , false, false>(bfloat16::bfloat16(-0.3));
	REQUIRE(0x8000 == val ); // 0x8000 = -0.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , false, false>(bfloat16::bfloat16(-0.3));
	REQUIRE(0x8000 == val ); // 0x8000 = -0.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , false, false>(bfloat16::bfloat16(-0.3));
	REQUIRE(0xBF80 == val ); // 0xBF80 = -1.0 bf16

	val =bfloat16::detail::integral<std::round_to_nearest , true, false>(bfloat16::bfloat16(-0.3));
	REQUIRE(0x8000 == val ); // 0x8000 = -0.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , true, false>(bfloat16::bfloat16(-0.3));
	REQUIRE(0x8000 == val ); // 0x8000 = -0.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , true, false>(bfloat16::bfloat16(-0.3));
	REQUIRE(0xBF80 == val ); // 0xBF80 = -1.0 bf16


	//for bfloat16(-0.5)
	val =bfloat16::detail::integral<std::round_to_nearest , false, false>(bfloat16::bfloat16(-0.5));
	REQUIRE(0xBF80 == val ); // 0xBF80 = -1.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , false, false>(bfloat16::bfloat16(-0.5));
	REQUIRE(0x8000 == val ); // 0x8000 = -0.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , false, false>(bfloat16::bfloat16(-0.5));
	REQUIRE(0xBF80 == val ); // 0xBF80 = -1.0 bf16

	val =bfloat16::detail::integral<std::round_to_nearest , true, false>(bfloat16::bfloat16(-0.5));
	REQUIRE(0x8000 == val ); // 0x8000 = -0.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , true, false>(bfloat16::bfloat16(-0.5));
	REQUIRE(0x8000 == val ); // 0x8000 = -0.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , true, false>(bfloat16::bfloat16(-0.5));
	REQUIRE(0xBF80 == val ); // 0xBF80 = -1.0 bf16


	//for bfloat16(-0.8)
	val =bfloat16::detail::integral<std::round_to_nearest , false, false>(bfloat16::bfloat16(-0.8));
	REQUIRE(0xBF80 == val ); // 0xBF80 = -1.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , false, false>(bfloat16::bfloat16(-0.8));
	REQUIRE(0x8000 == val ); // 0x8000 = -0.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , false, false>(bfloat16::bfloat16(-0.8));
	REQUIRE(0xBF80 == val ); // 0xBF80 = -1.0 bf16

	val =bfloat16::detail::integral<std::round_to_nearest , true, false>(bfloat16::bfloat16(-0.8));
	REQUIRE(0xBF80 == val ); // 0xBF80 = -1.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , true, false>(bfloat16::bfloat16(-0.8));
	REQUIRE(0x8000 == val ); // 0x8000 = -0.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , true, false>(bfloat16::bfloat16(-0.8));
	REQUIRE(0xBF80 == val ); // 0xBF80 = -1.0 bf16


	//for bfloat16(-6.3)
	val =bfloat16::detail::integral<std::round_to_nearest , false, false>(bfloat16::bfloat16(-6.3));
	REQUIRE(0xC0C0 == val ); // 0xC0C0 = -6.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , false, false>(bfloat16::bfloat16(-6.3));
	REQUIRE(0xC0C0 == val ); // 0xC0C0 = -6.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , false, false>(bfloat16::bfloat16(-6.3));
	REQUIRE(0xC0E0 == val ); // 0xC0E0 = -7.0 bf16

	val =bfloat16::detail::integral<std::round_to_nearest , true, false>(bfloat16::bfloat16(-6.3));
	REQUIRE(0xC0C0 == val ); // 0xC0C0 = -6.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , true, false>(bfloat16::bfloat16(-6.3));
	REQUIRE(0xC0C0 == val ); // 0xC0C0 = -6.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , true, false>(bfloat16::bfloat16(-6.3));
	REQUIRE(0xC0E0 == val ); // 0xC0E0 = -7.0 bf16


	//for bfloat16(-6.5)
	val =bfloat16::detail::integral<std::round_to_nearest , false, false>(bfloat16::bfloat16(-6.5));
	REQUIRE(0xC0C0 == val ); // 0xC0C0 = -6.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , false, false>(bfloat16::bfloat16(-6.5));
	REQUIRE(0xC0C0 == val ); // 0xC0C0 = -6.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , false, false>(bfloat16::bfloat16(-6.5));
	REQUIRE(0xC0E0 == val ); // 0xC0E0 = -7.0 bf16

	val =bfloat16::detail::integral<std::round_to_nearest , true, false>(bfloat16::bfloat16(-6.5));
	REQUIRE(0xC0E0 == val ); // 0xC0E0 = -7.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , true, false>(bfloat16::bfloat16(-6.5));
	REQUIRE(0xC0C0 == val ); // 0xC0C0 = -6.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , true, false>(bfloat16::bfloat16(-6.5));
	REQUIRE(0xC0E0 == val ); // 0xC0E0 = -7.0 bf16


	//for bfloat16(-6.8)
	val =bfloat16::detail::integral<std::round_to_nearest , false, false>(bfloat16::bfloat16(-6.8));
	REQUIRE(0xC0E0 == val ); // 0xC0E0 = -7.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , false, false>(bfloat16::bfloat16(-6.8));
	REQUIRE(0xC0C0 == val ); // 0xC0C0 = -6.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , false, false>(bfloat16::bfloat16(-6.8));
	REQUIRE(0xC0E0 == val ); // 0xC0E0 = -7.0 bf16

	val =bfloat16::detail::integral<std::round_to_nearest , true, false>(bfloat16::bfloat16(-6.8));
	REQUIRE(0xC0E0 == val ); // 0xC0E0 = -7.0 bf16
	val =bfloat16::detail::integral<std::round_toward_infinity , true, false>(bfloat16::bfloat16(-6.8));
	REQUIRE(0xC0C0 == val ); // 0xC0C0 = -6.0 bf16
	val =bfloat16::detail::integral<std::round_toward_neg_infinity , true, false>(bfloat16::bfloat16(-6.8));
	REQUIRE(0xC0E0 == val ); // 0xC0E0 = -7.0 bf16

}



TEST_CASE("bfloat16_bfloat16_detail_fixed2bfloat16", "[float16]"){

	//works

	//creating fixed point values for conversion
	bfloat16::detail::f31 pointten(bfloat16::bfloat16(0.10));
	bfloat16::detail::f31 pointfive(bfloat16::bfloat16(0.5));
	bfloat16::detail::f31 one(bfloat16::bfloat16(1.0));
	bfloat16::detail::f31 onepointsix(bfloat16::bfloat16(1.6));
	bfloat16::detail::f31 twopointfour(bfloat16::bfloat16(2.4));
	bfloat16::detail::f31 tenpointeight(bfloat16::bfloat16(10.8));
	bfloat16::detail::f31 fiftypointfive(bfloat16::bfloat16(50.5));
	bfloat16::detail::f31 fivehundred(bfloat16::bfloat16(586.3));

	bfloat16::detail::f31 negpointten(bfloat16::bfloat16(-0.10));
	bfloat16::detail::f31 negpointfive(bfloat16::bfloat16(-0.5));
	bfloat16::detail::f31 negone(bfloat16::bfloat16(-1.0));
	bfloat16::detail::f31 negonepointsix(bfloat16::bfloat16(-1.6));
	bfloat16::detail::f31 negtwopointfour(bfloat16::bfloat16(-2.4));
	bfloat16::detail::f31 negtenpointeight(bfloat16::bfloat16(-10.8));
	bfloat16::detail::f31 negfiftypointfive(bfloat16::bfloat16(-50.5));
	bfloat16::detail::f31 negfivehundred(bfloat16::bfloat16(-586.3));

	unsigned int possign = (0x4120 & 0x8000); // sign from 10 = 0x0
	unsigned int negsign = (0xC120 & 0x8000); // sign from -10 = 0x8000

	unsigned int val;

	//positive fixed point test
	//tests for (0.10)
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(pointten.m, pointten.exp+14, possign);
	REQUIRE(0x3DCC == val ); // 0x3DCC = 0.099609375 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_infinity,31,false, false, true>(pointten.m, pointten.exp+14, possign);
	REQUIRE(0x3DCC == val ); // 0x3DCC = 0.099609375 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_neg_infinity,31,false, false, true>(pointten.m, pointten.exp+14, possign);
	REQUIRE(0x3DCC == val ); // 0x3DCC = 0.099609375 bf16

	//tests for (0.5)
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(pointfive.m, pointfive.exp+14, possign);
	REQUIRE(0x3F00 == val ); // 0x3F00 = 0.5 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_infinity,31,false, false, true>(pointfive.m, pointfive.exp+14, possign);
	REQUIRE(0x3F00 == val ); // 0x3F00 = 0.5 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_neg_infinity,31,false, false, true>(pointfive.m, pointfive.exp+14, possign);
	REQUIRE(0x3F00 == val ); // 0x3F00 = 0.5 bf16

	//tests for (1.0)
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(one.m, one.exp+14, possign);
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_infinity,31,false, false, true>(one.m, one.exp+14, possign);
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_neg_infinity,31,false, false, true>(one.m, one.exp+14, possign);
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16

	//tests for (1.6)
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(onepointsix.m, onepointsix.exp+14, possign);
	REQUIRE(0x3FCC == val ); // 0x3FCC = 1.59375 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_infinity,31,false, false, true>(onepointsix.m, onepointsix.exp+14, possign);
	REQUIRE(0x3FCC == val ); // 0x3FCC = 1.59375 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_neg_infinity,31,false, false, true>(onepointsix.m, onepointsix.exp+14, possign);
	REQUIRE(0x3FCC == val ); // 0x3FCC = 1.59375 bf16

	//tests for (2.4)
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(twopointfour.m, twopointfour.exp+14, possign);
	REQUIRE(0x4019 == val ); // 0x4019 = 2.390625 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_infinity,31,false, false, true>(twopointfour.m, twopointfour.exp+14, possign);
	REQUIRE(0x4019 == val ); // 0x4019 = 2.390625 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_neg_infinity,31,false, false, true>(twopointfour.m, twopointfour.exp+14, possign);
	REQUIRE(0x4019 == val ); // 0x4019 = 2.390625 bf16

	//tests for (10.8)
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(tenpointeight.m, tenpointeight.exp+14, possign);
	REQUIRE(0x412C == val ); // 0x412C = 10.75 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_infinity,31,false, false, true>(tenpointeight.m, tenpointeight.exp+14, possign);
	REQUIRE(0x412C == val ); // 0x412C = 10.75 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_neg_infinity,31,false, false, true>(tenpointeight.m, tenpointeight.exp+14, possign);
	REQUIRE(0x412C == val ); // 0x412C = 10.75 bf16

	//tests for (50.5)
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(fiftypointfive.m, fiftypointfive.exp+14, possign);
	REQUIRE(0x424A == val ); // 0x424A = 50.5 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_infinity,31,false, false, true>(fiftypointfive.m, fiftypointfive.exp+14, possign);
	REQUIRE(0x424A == val ); // 0x424A = 50.5 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_neg_infinity,31,false, false, true>(fiftypointfive.m, fiftypointfive.exp+14, possign);
	REQUIRE(0x424A == val ); // 0x424A = 50.5 bf16

	//tests for (586.3)
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(fivehundred.m, fivehundred.exp+14, possign);
	REQUIRE(0x4412 == val ); // 0x4412 = 584.0 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_infinity,31,false, false, true>(fivehundred.m, fivehundred.exp+14, possign);
	REQUIRE(0x4412 == val ); // 0x4412 = 584.0 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_neg_infinity,31,false, false, true>(fivehundred.m, fivehundred.exp+14, possign);
	REQUIRE(0x4412 == val ); // 0x4412 = 584.0 bf16


	//negative fixed Point Test
	//tests for (-0.10)
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,true, false, true>(negpointten.m, negpointten.exp+14, negsign);
	REQUIRE(0xBDCC == val ); // 0xBDCC = -0.099609375 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_infinity,31,true, false, true>(negpointten.m, negpointten.exp+14, negsign);
	REQUIRE(0xBDCC == val ); // 0xBDCC = -0.099609375 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_neg_infinity,31,true, false, true>(negpointten.m, negpointten.exp+14, negsign);
	REQUIRE(0xBDCC == val ); // 0xBDCC = -0.099609375 bf16

	//tests for (-0.5)
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,true, false, true>(negpointfive.m, negpointfive.exp+14, negsign);
	REQUIRE(0xBF00 == val ); // 0xBF00 = -0.5 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_infinity,31,true, false, true>(negpointfive.m, negpointfive.exp+14, negsign);
	REQUIRE(0xBF00 == val ); // 0xBF00 = -0.5 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_neg_infinity,31,true, false, true>(negpointfive.m, negpointfive.exp+14, negsign);
	REQUIRE(0xBF00 == val ); // 0xBF00 = -0.5 bf16

	//tests for (-1.0)
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,true, false, true>(negone.m, negone.exp+14, negsign);
	REQUIRE(0xBF80 == val ); // 0xBF80 = -1.0 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_infinity,31,true, false, true>(negone.m, negone.exp+14, negsign);
	REQUIRE(0xBF80 == val ); // 0xBF80 = -1.0 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_neg_infinity,31,true, false, true>(negone.m, negone.exp+14, negsign);
	REQUIRE(0xBF80 == val ); // 0xBF80 = -1.0 bf16

	//tests for (-1.6)
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,true, false, true>(negonepointsix.m, negonepointsix.exp+14, negsign);
	REQUIRE(0xBFCC == val ); // 0xBFCC = -1.59375 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_infinity,31,true, false, true>(negonepointsix.m, negonepointsix.exp+14, negsign);
	REQUIRE(0xBFCC == val ); // 0xBFCC = -1.59375 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_neg_infinity,31,true, false, true>(negonepointsix.m, negonepointsix.exp+14, negsign);
	REQUIRE(0xBFCC == val ); // 0xBFCC = -1.59375 bf16

	//tests for (-2.4)
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,true, false, true>(negtwopointfour.m, negtwopointfour.exp+14, negsign);
	REQUIRE(0xC019 == val ); // 0xC019 = -2.390625 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_infinity,31,true, false, true>(negtwopointfour.m, negtwopointfour.exp+14, negsign);
	REQUIRE(0xC019 == val ); // 0xC019 = -2.390625 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_neg_infinity,31,true, false, true>(negtwopointfour.m, negtwopointfour.exp+14, negsign);
	REQUIRE(0xC019 == val ); // 0xC019 = -2.390625 bf16

	//tests for (-10.8)
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,true, false, true>(negtenpointeight.m, negtenpointeight.exp+14, negsign);
	REQUIRE(0xC12C == val ); // 0xC12C = -10.75 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_infinity,31,true, false, true>(negtenpointeight.m, negtenpointeight.exp+14, negsign);
	REQUIRE(0xC12C == val ); // 0xC12C = -10.75 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_neg_infinity,31,true, false, true>(negtenpointeight.m, negtenpointeight.exp+14, negsign);
	REQUIRE(0xC12C == val ); // 0xC12C = -10.75 bf16

	//tests for (-50.5)
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,true, false, true>(negfiftypointfive.m, negfiftypointfive.exp+14, negsign);
	REQUIRE(0xC24A == val ); // 0xC24A = -50.5 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_infinity,31,true, false, true>(negfiftypointfive.m, negfiftypointfive.exp+14, negsign);
	REQUIRE(0xC24A == val ); // 0xC24A = -50.5 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_neg_infinity,31,true, false, true>(negfiftypointfive.m, negfiftypointfive.exp+14, negsign);
	REQUIRE(0xC24A == val ); // 0xC24A = -50.5 bf16

	//tests for (-586.3)
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,true, false, true>(negfivehundred.m, negfivehundred.exp+14, negsign);
	REQUIRE(0xC412 == val ); // 0xC412 = -584.0 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_infinity,31,true, false, true>(negfivehundred.m, negfivehundred.exp+14, negsign);
	REQUIRE(0xC412 == val ); // 0xC412 = -584.0 bf16
	val = bfloat16::detail::fixed2bfloat16<std::round_toward_neg_infinity,31,true, false, true>(negfivehundred.m, negfivehundred.exp+14, negsign);
	REQUIRE(0xC412 == val ); // 0xC412 = -584.0 bf16

}



//TODO, conversion from double -> bfloat
TEST_CASE("bfloat16_bfloat16_detail_float2bfloat16_float", "[float16]"){

	//works

	//Constructor
	//positive Tests
	REQUIRE(0x3DCC == bfloat16::bfloat16(0.1)); // 0x3DCC = 0.099609375 bf16
	REQUIRE(0x3F00 == bfloat16::bfloat16(0.5)); // 0x3F00 = 0.5 bf16
	REQUIRE(0x3F80 == bfloat16::bfloat16(1.0)); // 0x3F80 = 1.0 bf16
	REQUIRE(0x3FCC == bfloat16::bfloat16(1.6)); // 0x3FCC = 1.59375 bf16
	REQUIRE(0x4019 == bfloat16::bfloat16(2.4)); // 0x4019 = 2.390625 bf16
	REQUIRE(0x412C == bfloat16::bfloat16(10.8)); // 0x412C = 10.75 bf16
	REQUIRE(0x424A == bfloat16::bfloat16(50.5)); // 0x424A = 50.5 bf16
	REQUIRE(0x4412 == bfloat16::bfloat16(586.3)); // 0x4412 = 584.0 bf16

	//negative Tests
	REQUIRE(0xBDCC == bfloat16::bfloat16(-0.1)); // 0xBDCC = -0.099609375 bf16
	REQUIRE(0xBF00 == bfloat16::bfloat16(-0.5)); // 0xBF00 = -0.5 bf16
	REQUIRE(0xBF80 == bfloat16::bfloat16(-1.0)); // 0xBF80 = -1.0 bf16
	REQUIRE(0xBFCC == bfloat16::bfloat16(-1.6)); // 0xBFCC = -1.59375 bf16
	REQUIRE(0xC019 == bfloat16::bfloat16(-2.4)); // 0xC019 = -2.390625 bf16
	REQUIRE(0xC12C == bfloat16::bfloat16(-10.8)); // 0xC12C = -10.75 bf16
	REQUIRE(0xC24A == bfloat16::bfloat16(-50.5)); // 0xC24A = -50.5 bf16
	REQUIRE(0xC412 == bfloat16::bfloat16(-586.3)); // 0xC412 = -584.0 bf16


	
	bfloat16::detail::uint16 val;

	//convert function
	//positive tests
	val = bfloat16::detail::float2bfloat16<std::round_to_nearest, float>(0.1);
	REQUIRE(0x3DCC == val ); // 0x3DCC = 0.099609375 bf16
	val = bfloat16::detail::float2bfloat16<std::round_to_nearest, float>(0.5);
	REQUIRE(0x3F00 == val ); // 0x3F00 = 0.5 bf16
	val = bfloat16::detail::float2bfloat16<std::round_to_nearest, float>(1.0);
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	val = bfloat16::detail::float2bfloat16<std::round_to_nearest, float>(1.6);
	REQUIRE(0x3FCC == val ); // 0x3FCC = 1.59375 bf16
	val = bfloat16::detail::float2bfloat16<std::round_to_nearest, float>(2.4);
	REQUIRE(0x4019 == val ); // 0x4019 = 2.390625 bf16
	val = bfloat16::detail::float2bfloat16<std::round_to_nearest, float>(10.8);
	REQUIRE(0x412C == val ); // 0x412C = 10.75 bf16
	val = bfloat16::detail::float2bfloat16<std::round_to_nearest, float>(50.5);
	REQUIRE(0x424A == val ); // 0x424A = 50.5 bf16
	val = bfloat16::detail::float2bfloat16<std::round_to_nearest, float>(586.3);
	REQUIRE(0x4412 == val ); // 0x4412 = 584.0 bf16

	//negative Tests
	val = bfloat16::detail::float2bfloat16<std::round_to_nearest, float>(-0.1);
	REQUIRE(0xBDCC == val ); // 0xBDCC = -0.099609375 bf16
	val = bfloat16::detail::float2bfloat16<std::round_to_nearest, float>(-0.5);
	REQUIRE(0xBF00 == val ); // 0xBF00 = -0.5 bf16
	val = bfloat16::detail::float2bfloat16<std::round_to_nearest, float>(-1.0);
	REQUIRE(0xBF80 == val ); // 0xBF80 = -1.0 bf16
	val = bfloat16::detail::float2bfloat16<std::round_to_nearest, float>(-1.6);
	REQUIRE(0xBFCC == val ); // 0xBFCC = -1.59375 bf16
	val = bfloat16::detail::float2bfloat16<std::round_to_nearest, float>(-2.4);
	REQUIRE(0xC019 == val ); // 0xC019 = -2.390625 bf16
	val = bfloat16::detail::float2bfloat16<std::round_to_nearest, float>(-10.8);
	REQUIRE(0xC12C == val ); // 0xC12C = -10.75 bf16
	val = bfloat16::detail::float2bfloat16<std::round_to_nearest, float>(-50.5);
	REQUIRE(0xC24A == val ); // 0xC24A = -50.5 bf16
	val = bfloat16::detail::float2bfloat16<std::round_to_nearest, float>(-586.3);
	REQUIRE(0xC412 == val ); // 0xC412 = -584.0 bf16



	//from float->bfloat->float
	//positive
	REQUIRE(0.099609375 == bfloat16::detail::bfloat162float_temp(bfloat16::bfloat16(0.1)));
	REQUIRE(0.5 == bfloat16::detail::bfloat162float_temp(bfloat16::bfloat16(0.5)) );
	REQUIRE(1.0 == bfloat16::detail::bfloat162float_temp(bfloat16::bfloat16(1.0)) );
	REQUIRE(1.59375 == bfloat16::detail::bfloat162float_temp(bfloat16::bfloat16(1.6)) );
	REQUIRE(2.390625 == bfloat16::detail::bfloat162float_temp(bfloat16::bfloat16(2.4)) );
	REQUIRE(10.75 == bfloat16::detail::bfloat162float_temp(bfloat16::bfloat16(10.8)) );
	REQUIRE(50.5 == bfloat16::detail::bfloat162float_temp(bfloat16::bfloat16(50.5)) );
	REQUIRE(584 == bfloat16::detail::bfloat162float_temp(bfloat16::bfloat16(586.3)) );

	//negative
	REQUIRE(-0.099609375 == bfloat16::detail::bfloat162float_temp(bfloat16::bfloat16(-0.1)) );
	REQUIRE(-0.5 == bfloat16::detail::bfloat162float_temp(bfloat16::bfloat16(-0.5)) );
	REQUIRE(-1.0 == bfloat16::detail::bfloat162float_temp(bfloat16::bfloat16(-1.0)) );
	REQUIRE(-1.59375 == bfloat16::detail::bfloat162float_temp(bfloat16::bfloat16(-1.6)) );
	REQUIRE(-2.390625 == bfloat16::detail::bfloat162float_temp(bfloat16::bfloat16(-2.4)) );
	REQUIRE(-10.75 == bfloat16::detail::bfloat162float_temp(bfloat16::bfloat16(-10.8)) );
	REQUIRE(-50.5 == bfloat16::detail::bfloat162float_temp(bfloat16::bfloat16(-50.5)) );
	REQUIRE(-584 == bfloat16::detail::bfloat162float_temp(bfloat16::bfloat16(-586.3)) );

}



TEST_CASE("bfloat16_bfloat16_detail_int2bfloat16_bfloat162int", "[floa16]"){

	//works

	unsigned int val;

	//positive
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(0);
	REQUIRE(0x0 == val ); // 0x0 = 0.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(1);
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(2);
	REQUIRE(0x4000 == val ); // 0x4000 = 2.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(3);
	REQUIRE(0x4040 == val ); // 0x4040 = 3.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(4);
	REQUIRE(0x4080 == val ); // 0x4080 = 4.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(5);
	REQUIRE(0x40A0 == val ); // 0x40A0 = 5.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(6);
	REQUIRE(0x40C0 == val ); // 0x40c0 = 6.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(7);
	REQUIRE(0x40E0 == val ); // 0x40E0 = 7.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(8);
	REQUIRE(0x4100 == val ); // 0x4100 = 8.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(9);
	REQUIRE(0x4110 == val ); // 0x4110 = 9.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(10);
	REQUIRE(0x4120 == val ); // 0x4120 = 10.0 bf16

	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(20);
	REQUIRE(0x41A0 == val ); // 0x41A0 = 20.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(30);
	REQUIRE(0x41F0 == val ); // 0x41F0 = 30.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(40);
	REQUIRE(0x4220 == val ); // 0x4220 = 40.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(50);
	REQUIRE(0x4248 == val ); // 0x4220 = 40.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(60);
	REQUIRE(0x4270 == val ); // 0x4270 = 60.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(70);
	REQUIRE(0x428C == val ); // 0x428C = 70.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(80);
	REQUIRE(0x42A0 == val ); // 0x42A0 = 80.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(90);
	REQUIRE(0x42B4 == val ); // 0x42B4 = 90.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(100);
	REQUIRE(0x42C8 == val ); // 0x42C8 = 100.0 bf16

	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(200);
	REQUIRE(0x4348 == val ); // 0x4348 = 200.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(300);
	REQUIRE(0x4396 == val ); // 0x4396 = 300.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(400);
	REQUIRE(0x43C8 == val ); // 0x43C8 = 400.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(500);
	REQUIRE(0x43FA == val ); // 0x43FA = 500.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(600);
	REQUIRE(0x4416 == val ); // 0x4416 = 600.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(700);
	REQUIRE(0x442F == val ); // 0x442F = 700.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(800);
	REQUIRE(0x4448 == val ); // 0x4448 = 800.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(900);
	REQUIRE(0x4461 == val ); // 0x4461 = 900.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(1000);
	REQUIRE(0x447A == val ); // 0x447A = 1000.0 bf16

	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(2000);
	REQUIRE(0x44FA == val ); // 0x44FA = 2000.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(3000);
	REQUIRE(0x453C == val ); // 0x453C = 3008.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(4000);
	REQUIRE(0x457A == val ); // 0x457A = 4000.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(5000);
	REQUIRE(0x459C == val ); // 0x459C = 4992.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(6000);
	REQUIRE(0x45BC == val ); // 0x45BC = 6016.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(7000);
	REQUIRE(0x45DB == val ); // 0x45DB = 7008.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(8000);
	REQUIRE(0x45FA == val ); // 0x45FA = 8000.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(9000);
	REQUIRE(0x460D == val ); // 0x460D = 9024.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(10000);
	REQUIRE(0x461C == val ); // 0x461C = 9984.0 bf16



	//negative
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-0);
	REQUIRE(0x0 == val ); // 0x0 = 0.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-1);
	REQUIRE(0xBF80 == val ); // 0xBF80 = -1.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-2);
	REQUIRE(0xC000 == val ); // 0xC000 = -2.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-3);
	REQUIRE(0xC040 == val ); // 0xC040 = -3.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-4);
	REQUIRE(0xC080 == val ); // 0xC080 = -4.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-5);
	REQUIRE(0xC0A0 == val ); // 0xC0A0 = -5.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-6);
	REQUIRE(0xC0C0 == val ); // 0xC0c0 = -6.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-7);
	REQUIRE(0xC0E0 == val ); // 0xC0E0 = -7.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-8);
	REQUIRE(0xC100 == val ); // 0xC100 = -8.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-9);
	REQUIRE(0xC110 == val ); // 0xC110 = -9.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-10);
	REQUIRE(0xC120 == val ); // 0xC120 = -10.0 bf16

	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-20);
	REQUIRE(0xC1A0 == val ); // 0xC1A0 = -20.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-30);
	REQUIRE(0xC1F0 == val ); // 0xC1F0 = -30.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-40);
	REQUIRE(0xC220 == val ); // 0xC220 = -40.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-50);
	REQUIRE(0xC248 == val ); // 0xC248 = -50.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-60);
	REQUIRE(0xC270 == val ); // 0xC270 = -60.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-70);
	REQUIRE(0xC28C == val ); // 0xC28C = -70.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-80);
	REQUIRE(0xC2A0 == val ); // 0xC2A0 = -80.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-90);
	REQUIRE(0xC2B4 == val ); // 0xC2B4 = -90.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-100);
	REQUIRE(0xC2C8 == val ); // 0xC2C8 = -100.0 bf16

	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-200);
	REQUIRE(0xC348 == val ); // 0xC348 = -200.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-300);
	REQUIRE(0xC396 == val ); // 0xC396 = -300.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-400);
	REQUIRE(0xC3C8 == val ); // 0xC3C8 = -400.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-500);
	REQUIRE(0xC3FA == val ); // 0xC3FA = -500.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-600);
	REQUIRE(0xC416 == val ); // 0xC416 = -600.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-700);
	REQUIRE(0xC42F == val ); // 0xC42F = -700.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-800);
	REQUIRE(0xC448 == val ); // 0xC448 = -800.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-900);
	REQUIRE(0xC461 == val ); // 0xC461 = -900.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-1000);
	REQUIRE(0xC47A == val ); // 0xC47A = -1000.0 bf16

	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-2000);
	REQUIRE(0xC4FA == val ); // 0xC4FA = -2000.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-3000);
	REQUIRE(0xC53C == val ); // 0xC53C = -3008.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-4000);
	REQUIRE(0xC57A == val ); // 0xC57A = -4000.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-5000);
	REQUIRE(0xC59C == val ); // 0xC59C = -4992.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-6000);
	REQUIRE(0xC5BC == val ); // 0xC5BC = -6016.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-7000);
	REQUIRE(0xC5DB == val ); // 0xC5DB = -7008.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-8000);
	REQUIRE(0xC5FA == val ); // 0xC5FA = -8000.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-9000);
	REQUIRE(0xC60D == val ); // 0xC60D = -9024.0 bf16
	val = bfloat16::detail::int2bfloat16<std::round_to_nearest, int>(-10000);
	REQUIRE(0xC61C == val ); // 0xC61C = -9984.0 bf16


	//bfloat162int
	int result;
	
	result = bfloat16::detail::bfloat162int<std::round_to_nearest, true, false, int>(bfloat16::bfloat16(0.1));
	REQUIRE(0 == result );
	result = bfloat16::detail::bfloat162int<std::round_to_nearest, false, false, int>(bfloat16::bfloat16(0.1));
	REQUIRE(0 == result );

	result = bfloat16::detail::bfloat162int<std::round_to_nearest, true, false, int>(bfloat16::bfloat16(0.5));
	REQUIRE(0 == result );
	result = bfloat16::detail::bfloat162int<std::round_to_nearest, false, false, int>(bfloat16::bfloat16(0.5));
	REQUIRE(1 == result );

	result = bfloat16::detail::bfloat162int<std::round_to_nearest, true, false, int>(bfloat16::bfloat16(1.0));
	REQUIRE(1 == result );
	result = bfloat16::detail::bfloat162int<std::round_to_nearest, false, false, int>(bfloat16::bfloat16(1.0));
	REQUIRE(1 == result );

	result = bfloat16::detail::bfloat162int<std::round_to_nearest, true, false, int>(bfloat16::bfloat16(1.4));
	REQUIRE(1 == result );
	result = bfloat16::detail::bfloat162int<std::round_to_nearest, false, false, int>(bfloat16::bfloat16(1.4));
	REQUIRE(1 == result );

	result = bfloat16::detail::bfloat162int<std::round_to_nearest, true, false, int>(bfloat16::bfloat16(2.1));
	REQUIRE(2 == result );
	result = bfloat16::detail::bfloat162int<std::round_to_nearest, true, false, int>(bfloat16::bfloat16(2.1));
	REQUIRE(2 == result );

	result = bfloat16::detail::bfloat162int<std::round_to_nearest, true, false, int>(bfloat16::bfloat16(10.8));
	REQUIRE(11 == result );
	result = bfloat16::detail::bfloat162int<std::round_to_nearest, true, false, int>(bfloat16::bfloat16(10.8));
	REQUIRE(11 == result );

}



TEST_CASE("bfloat16_bfloat16_detail_f31", "[float16]"){

	//works

	bfloat16::detail::f31 pointone(bfloat16::bfloat16(0.1));
	bfloat16::detail::f31 pointfive(bfloat16::bfloat16(0.5));
	bfloat16::detail::f31 five(bfloat16::bfloat16(5));
	bfloat16::detail::f31 ten(bfloat16::bfloat16(10.0));
	bfloat16::detail::f31 tenone(bfloat16::bfloat16(10.1));
	bfloat16::detail::f31 tenfive(bfloat16::bfloat16(10.5));
	bfloat16::detail::f31 twentyfive(bfloat16::bfloat16(20.5));

	
	bfloat16::detail::f31 result(24, 7);

	unsigned int possign = (0x4120 & 0x8000);
	unsigned int negsign = (0xC120 & 0x8000);

	unsigned int val;

	
	//add
	//0.1
	result = pointone + pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x3E4C == val ); // 0x3e4c = 0.19921875 bf16
	result = pointone + pointfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x3F1A == val ); // 0x3f1a = 0.6015625 bf16
	result = pointone + five;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x40A3 == val ); // 0x40a3 = 5.09375 bf16
	result = pointone + ten;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x4122 == val ); // 0x4122 = 10.125 bf16
	result = pointone + tenone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x4123 == val ); // 0x4123 = 10.1875 bf16
	result = pointone + tenfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x412A == val ); // 0x412A = 10.625 bf16
	result = pointone + twentyfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x41A5 == val ); // 0x41A5 = 20.625 bf16
	
	//0.5
	result = pointfive + pointfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	result = pointfive + five;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x40B0 == val ); // 0x40B8 = 5.75 bf16
	result = pointfive + ten;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x4128 == val ); // 0x4128 = 10.5 bf16
	result = pointfive + tenone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x4129 == val ); // 0x4129 = 10.5625 bf16
	result = pointfive + tenfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x4130 == val ); // 0x4130 = 11.0 bf16
	result = pointfive + twentyfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x41A8 == val ); // 0x41A8 = 21.0 bf16

	//5
	result = five + five;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x4120 == val ); // 0x4120 = 10.0 bf16
	result = five + ten;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x4170 == val ); // 0x4170 = 15.0 bf16
	result = five + tenone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x4171 == val ); // 0x4171 = 15.0625 bf16
	result = five + tenfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x4178 == val ); // 0x4178 = 15.5 bf16
	result = five + twentyfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x41CC == val ); // 0x41CC = 25.5 bf16

	//10
	result = ten + ten;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x41A0 == val ); // 0x41A0 = 20.0 bf16
	result = ten + tenone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x41A0 == val ); // 0x41A0 = 20.0 bf16
	result = ten + tenfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x41A4 == val ); // 0x41A4 = 20.5 bf16
	result = ten + twentyfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x41F4 == val ); // 0x41F4 = 30.5 bf16

	//10.1
	result = tenone + tenone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x41A1 == val ); // 0x41A1 = 20.125 bf16
	result = tenone + tenfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x41A4 == val ); // 0x41A4 = 20.5 bf16
	result = tenone + twentyfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x41F4 == val ); // 0x41F4 = 30.5 bf16

	//10.5
	result = tenfive + tenfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x41A8 == val ); // 0x41A8 = 21.0 bf16
	result = tenfive + twentyfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x41F8 == val ); // 0x41F8 = 31.0 bf16
 
	//20.5
	result = twentyfive + twentyfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x4224 == val ); // 0x4224 = 41.0 bf16



	//sub
	//0.1
	result = pointone - pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x0 == val ); // 0x0 = 0.0 bf16
	result = pointfive - pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x3ECD == val ); // 0x3ECD = 0.400390625 bf16
	result = five - pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x409D == val ); // 0x409D = 4.90625 bf16
	result = ten - pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x411E == val ); // 0x411E = 9.875 bf16
	result = tenone - pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x411F == val ); // 0x411F = 9.9375 bf16
	result = tenfive - pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x4126 == val ); // 0x4126 = 10.375 bf16
	result = twentyfive - pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x41A3 == val ); // 0x41A3 = 20.375 bf16
	
	//0.5
	result = pointfive - pointfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x0 == val ); // 0x0 = 0.0 bf16
	result = five - pointfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x4090 == val ); // 0x4090 = 4.5 bf16
	result = ten - pointfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x4118 == val ); // 0x4118 = 9.5 bf16
	result = tenone - pointfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x4119 == val ); // 0x4119 = 9.5625 bf16
	result = tenfive - pointfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x4120 == val ); // 0x4120 = 10.0 bf16
	result = twentyfive - pointfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x41A0 == val ); // 0x41A0 = 20.0 bf16

	//5
	result = five - five;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x0 == val ); // 0x0 = 0.0 bf16
	result = ten - five;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x40A0 == val ); // 0x40A0 = 5.0 bf16
	result = tenone - five;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x40A2 == val ); // 0x40A2 = 5.0625 bf16
	result = tenfive - five;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x40B0 == val ); // 0x40B0 = 5.5 bf16
	result = twentyfive - five;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x4178 == val ); // 0x4178 = 15.5 bf16

	//10
	result = ten - ten;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x0 == val ); // 0x0 = 0.0 bf16
	result = tenone - ten;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x3D80 == val ); // 0x3D80 = 0.0625 bf16
	result = tenfive - ten;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x3F00 == val ); // 0x3F00 = 0.5 bf16
	result = twentyfive - ten;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x4128 == val ); // 0x4128 = 10.5 bf16

	//10.1
	result = tenone - tenone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x0 == val ); // 0x0 = 0.0 bf16
	result = tenfive - tenone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x3EE0 == val ); // 0x3EE0 = 0.4375 bf16
	result = twentyfive - tenone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x4127 == val ); // 0x4127 = 10.4375 bf16

	//10.5
	result = tenfive - tenfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x0 == val ); // 0x0 = 0.0 bf16
	result = twentyfive - tenfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x4120 == val ); // 0x4120 = 10.0 bf16

	//20.5
	result = twentyfive - twentyfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+14, possign);
	REQUIRE(0x0 == val ); // 0x0 = 0.0 bf16
	


	//mult
	//0.1
	result = pointone * pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x3C23 == val ); // 0x3C23 = 0.00994873046875 bf16
	result = pointfive * pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x3D4C == val ); // 0x3D4C = 0.0498046875 bf16
	result = five * pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x3EFF == val ); // 0x3EFF = 0.498046875 bf16
	result = ten * pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x3F7F == val ); // 0x3F7F = 0.99609375 bf16
	result = tenone * pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	result = tenfive * pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x3F86 == val ); // 0x3F86 = 1.046875 bf16
	result = twentyfive * pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x4003 == val ); // 0x4003 = 2.046875 bf16

	//0.5
	result = pointfive * pointfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x3E80 == val ); // 0x3E80 = 0.25 bf16
	result = five * pointfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x4020 == val ); // 0x4020 = 2.5 bf16
	result = ten * pointfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x40A0 == val ); // 0x40A0 = 5.0 bf16
	result = tenone * pointfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x40A1 == val ); // 0x40A1 = 5.03125 bf16
	result = tenfive * pointfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x40A8 == val ); // 0x40A8 = 5.25 bf16
	result = twentyfive * pointfive; 
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x4124 == val ); // 0x4124 = 10.25 bf16

	//5
	result = five * five;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x41C8 == val ); // 0x41C8 = 25.0 bf16
	result = ten * five;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x4248 == val ); // 0x4248 = 50.0 bf16
	result = tenone * five;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x4249 == val ); // 0x4249 = 50.25 bf16
	result = tenfive * five;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x4252 == val ); // 0x4252 = 52.5 bf16
	result = twentyfive * five;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x42CD == val ); // 0x42CD = 102.5 bf16

	//10
	result = ten * ten;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x42C8 == val ); // 0x42C8 = 100.0 bf16
	result = tenone * ten;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x42C9 == val ); // 0x42C9 = 100.5 bf16
	result = tenfive * ten;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x42D2 == val ); // 0x42D2 = 105.0 bf16
	result = twentyfive * ten;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x434D == val ); // 0x434D = 205.0 bf16

	//10.1
	result = tenone * tenone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x42CB == val ); // 0x42CB = 101.5 bf16
	result = tenfive * tenone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x42D3 == val ); // 0x42D3 = 105.5 bf16
	result = twentyfive * tenone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x434E == val ); // 0x434E = 206.0 bf16

	//10.5
	result = tenfive * tenfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x42DC == val ); // 0x42DC = 110.0 bf16
	result = twentyfive * tenfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x4357 == val ); // 0x4357 = 215.0 bf16

	//20.5
	result = twentyfive * twentyfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp-98, possign);
	REQUIRE(0x43D2 == val ); // 0x43D2 = 420.0 bf16



	//Div
	//0.1
	result = pointone / pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	result = pointfive / pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x40A1 == val ); // 0x40A1 = 5.03125 bf16
	result = five / pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x4249 == val ); // 0x4249 = 50.25 bf16
	result = ten / pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x42C9 == val ); // 0x42C9 = 100.5 bf16
	result = tenone / pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x42CA == val ); // 0x42CA = 101.0 bf16
	result = tenfive / pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x42D3 == val ); // 0x42D3 = 105.5 bf16
	result = twentyfive / pointone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x434E == val ); // 0x434E = 206.0 bf16

	//0.5
	result = pointfive / pointfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	result = five / pointfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x4120 == val ); // 0x4120 = 10.0 bf16
	result = ten / pointfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x41A0 == val ); // 0x41A0 = 20.0 bf16
	result = tenone / pointfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x41A1 == val ); // 0x41A1 = 20.125 bf16
	result = tenfive / pointfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x41A8 == val ); // 0x41A8 = 21.0 bf16
	result = twentyfive / pointfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x4224 == val ); // 0x4224 = 41.0 bf16

	//5
	result = five / five;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	result = ten / five;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x4000 == val ); // 0x4000 = 2.0 bf16
	result = tenone / five;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x4001 == val ); // 0x4001 = 2.015625 bf16
	result = tenfive / five;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x4006 == val ); // 0x4006 = 2.09375 bf16
	result = twentyfive / five;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x4083 == val ); // 0x4083 = 4.09375 bf16

	//10
	result = ten / ten;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	result = tenone / ten;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x3F81 == val ); // 0x3F81 = 1.0078125 bf16
	result = tenfive / ten;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x3F86 == val ); // 0x3F86 = 1.046875 bf16
	result = twentyfive / ten;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x4003 == val ); // 0x4003 = 2.046875 bf16

	//10.1
	result = tenone / tenone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	result = tenfive / tenone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x3F86 == val ); // 0x3F86 = 1.046875 bf16
	result = twentyfive / tenone;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x4002 == val ); // 0x4002 = 2.03125 bf16

	//10.5
	result = tenfive / tenfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16
	result = twentyfive / tenfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x3FFA == val ); // 0x3FFA = 1.953125 bf16

	//20.5
	result = twentyfive / twentyfive;
	val = bfloat16::detail::fixed2bfloat16<std::round_to_nearest,31,false, false, true>(result.m, result.exp+126, possign);
	REQUIRE(0x3F80 == val ); // 0x3F80 = 1.0 bf16

}



TEST_CASE("bfloat16_bfloat16_detail_mod", "[float16]"){

	//works

	int *quo;
	unsigned int test = bfloat16::detail::mod<false, false>(bfloat16::bfloat16(1000), bfloat16::bfloat16(6), quo);
	REQUIRE(0x4080 == test ); // 0x4080 = 4.0 bf16
	test = bfloat16::detail::mod<false, false>(bfloat16::bfloat16(1000), bfloat16::bfloat16(2), quo);
	REQUIRE(0x0 == test ); // 0x0 = 0.0 bf16
	test = bfloat16::detail::mod<false, false>(bfloat16::bfloat16(1000), bfloat16::bfloat16(1), quo);
	REQUIRE(0x0 == test ); // 0x0 = 0.0 bf16
	test = bfloat16::detail::mod<false, false>(bfloat16::bfloat16(1000), bfloat16::bfloat16(150), quo);
	REQUIRE(0x42C8 == test ); // 0x42C8 = 100.0 bf16
	test = bfloat16::detail::mod<false, false>(bfloat16::bfloat16(10), bfloat16::bfloat16(3), quo);
	REQUIRE(0x3F80 == test ); // 0x3F80 = 1.0 bf16

}



TEST_CASE("bfloat16_operator_equal", "[float16]"){

	//works

	//numeric Limits
	REQUIRE(0x7F80 == std::numeric_limits<bfloat16::bfloat16>::infinity());
	REQUIRE(0x7FFF == std::numeric_limits<bfloat16::bfloat16>::quiet_NaN());
	REQUIRE(0x0080 == std::numeric_limits<bfloat16::bfloat16>::min());
	REQUIRE(0xFF7F == std::numeric_limits<bfloat16::bfloat16>::lowest());
	REQUIRE(0x7F7F == std::numeric_limits<bfloat16::bfloat16>::max());
	REQUIRE(0x0280 == std::numeric_limits<bfloat16::bfloat16>::epsilon());
	REQUIRE(0x3F00 == std::numeric_limits<bfloat16::bfloat16>::round_error());
	REQUIRE(0x7FBF == std::numeric_limits<bfloat16::bfloat16>::signaling_NaN());
	REQUIRE(0x0001 == std::numeric_limits<bfloat16::bfloat16>::denorm_min());


	//positive
	REQUIRE(bfloat16::bfloat16(0.5) == bfloat16::bfloat16(0.5) );
	REQUIRE_FALSE(bfloat16::bfloat16(0.1) == bfloat16::bfloat16(0.5) );
	REQUIRE_FALSE(bfloat16::bfloat16(0.5) == bfloat16::bfloat16(0.1) );

	REQUIRE(bfloat16::bfloat16(10) == bfloat16::bfloat16(10) );
	REQUIRE_FALSE(bfloat16::bfloat16(20) == bfloat16::bfloat16(10) );
	REQUIRE_FALSE(bfloat16::bfloat16(10) == bfloat16::bfloat16(20) );

	REQUIRE(bfloat16::bfloat16(10.1) == bfloat16::bfloat16(10.1) );
	REQUIRE_FALSE(bfloat16::bfloat16(20.1) == bfloat16::bfloat16(10.1) );
	REQUIRE_FALSE(bfloat16::bfloat16(10.1) == bfloat16::bfloat16(20.1) );

	REQUIRE(bfloat16::bfloat16(10.5) == bfloat16::bfloat16(10.5) );
	REQUIRE_FALSE(bfloat16::bfloat16(20.5) == bfloat16::bfloat16(10.5) );
	REQUIRE_FALSE(bfloat16::bfloat16(10.5) == bfloat16::bfloat16(20.5) );

	
	//negative
	REQUIRE(bfloat16::bfloat16(-0.5) == bfloat16::bfloat16(-0.5) );
	REQUIRE_FALSE(bfloat16::bfloat16(-0.1) == bfloat16::bfloat16(-0.5) );
	REQUIRE_FALSE(bfloat16::bfloat16(-0.5) == bfloat16::bfloat16(-0.1) );

	REQUIRE(bfloat16::bfloat16(-10) == bfloat16::bfloat16(-10) );
	REQUIRE_FALSE(bfloat16::bfloat16(-20) == bfloat16::bfloat16(-10) );
	REQUIRE_FALSE(bfloat16::bfloat16(-10) == bfloat16::bfloat16(-20) );

	REQUIRE(bfloat16::bfloat16(-10.1) == bfloat16::bfloat16(-10.1) );
	REQUIRE_FALSE(bfloat16::bfloat16(-20.1) == bfloat16::bfloat16(-10.1) );
	REQUIRE_FALSE(bfloat16::bfloat16(-10.1) == bfloat16::bfloat16(-20.1) );

	REQUIRE(bfloat16::bfloat16(-10.5) == bfloat16::bfloat16(-10.5) );
	REQUIRE_FALSE(bfloat16::bfloat16(-20.5) == bfloat16::bfloat16(-10.5) );
	REQUIRE_FALSE(bfloat16::bfloat16(-10.5) == bfloat16::bfloat16(-20.5) );

}



TEST_CASE("bfloat16_operator_notequal", "[float16]"){

	//works

	//numeric Limits
	REQUIRE(0x7F80 == std::numeric_limits<bfloat16::bfloat16>::infinity());
	REQUIRE(0x7FFF == std::numeric_limits<bfloat16::bfloat16>::quiet_NaN());
	REQUIRE(0x0080 == std::numeric_limits<bfloat16::bfloat16>::min());
	REQUIRE(0xFF7F == std::numeric_limits<bfloat16::bfloat16>::lowest());
	REQUIRE(0x7F7F == std::numeric_limits<bfloat16::bfloat16>::max());
	REQUIRE(0x0280 == std::numeric_limits<bfloat16::bfloat16>::epsilon());
	REQUIRE(0x3F00 == std::numeric_limits<bfloat16::bfloat16>::round_error());
	REQUIRE(0x7FBF == std::numeric_limits<bfloat16::bfloat16>::signaling_NaN());
	REQUIRE(0x0001 == std::numeric_limits<bfloat16::bfloat16>::denorm_min());


	//positive
	REQUIRE(bfloat16::bfloat16(0.5) == bfloat16::bfloat16(0.5) );
	REQUIRE_FALSE(bfloat16::bfloat16(0.1) == bfloat16::bfloat16(0.5) );
	REQUIRE_FALSE(bfloat16::bfloat16(0.5) == bfloat16::bfloat16(0.1) );

	REQUIRE(bfloat16::bfloat16(10) == bfloat16::bfloat16(10) );
	REQUIRE_FALSE(bfloat16::bfloat16(20) == bfloat16::bfloat16(10) );
	REQUIRE_FALSE(bfloat16::bfloat16(10) == bfloat16::bfloat16(20) );

	REQUIRE(bfloat16::bfloat16(10.1) == bfloat16::bfloat16(10.1) );
	REQUIRE_FALSE(bfloat16::bfloat16(20.1) == bfloat16::bfloat16(10.1) );
	REQUIRE_FALSE(bfloat16::bfloat16(10.1) == bfloat16::bfloat16(20.1) );

	REQUIRE(bfloat16::bfloat16(10.5) == bfloat16::bfloat16(10.5) );
	REQUIRE_FALSE(bfloat16::bfloat16(20.5) == bfloat16::bfloat16(10.5) );
	REQUIRE_FALSE(bfloat16::bfloat16(10.5) == bfloat16::bfloat16(20.5) );


	//negative
	REQUIRE(bfloat16::bfloat16(-0.5) == bfloat16::bfloat16(-0.5) );
	REQUIRE_FALSE(bfloat16::bfloat16(-0.1) == bfloat16::bfloat16(-0.5) );
	REQUIRE_FALSE(bfloat16::bfloat16(-0.5) == bfloat16::bfloat16(-0.1) );

	REQUIRE(bfloat16::bfloat16(-10) == bfloat16::bfloat16(-10) );
	REQUIRE_FALSE(bfloat16::bfloat16(20) == bfloat16::bfloat16(10) );
	REQUIRE_FALSE(bfloat16::bfloat16(10) == bfloat16::bfloat16(20) );

	REQUIRE(bfloat16::bfloat16(-10.1) == bfloat16::bfloat16(-10.1) );
	REQUIRE_FALSE(bfloat16::bfloat16(-20.1) == bfloat16::bfloat16(-10.1) );
	REQUIRE_FALSE(bfloat16::bfloat16(-10.1) == bfloat16::bfloat16(-20.1) );

	REQUIRE(bfloat16::bfloat16(-10.5) == bfloat16::bfloat16(-10.5) );
	REQUIRE_FALSE(bfloat16::bfloat16(-20.5) == bfloat16::bfloat16(-10.5) );
	REQUIRE_FALSE(bfloat16::bfloat16(-10.5) == bfloat16::bfloat16(-20.5) );

}



TEST_CASE("bfloat16_operator_less", "[float16]"){

	//works

	//numeric Limits
	REQUIRE(false == 0x7F80 < std::numeric_limits<bfloat16::bfloat16>::infinity());
	REQUIRE(false == 0x7FFF < std::numeric_limits<bfloat16::bfloat16>::quiet_NaN());
	REQUIRE(false == 0x0080 < std::numeric_limits<bfloat16::bfloat16>::min());
	REQUIRE(false == 0xFF7F < std::numeric_limits<bfloat16::bfloat16>::lowest());
	REQUIRE(false == 0x7F7F < std::numeric_limits<bfloat16::bfloat16>::max());
	REQUIRE(false == 0x0280 < std::numeric_limits<bfloat16::bfloat16>::epsilon());
	REQUIRE(false == 0x3F00 < std::numeric_limits<bfloat16::bfloat16>::round_error());
	REQUIRE(false == 0x7FBF < std::numeric_limits<bfloat16::bfloat16>::signaling_NaN());
	REQUIRE(false == 0x0001 < std::numeric_limits<bfloat16::bfloat16>::denorm_min());


	//positive
	REQUIRE(false == bfloat16::bfloat16(0.5) < bfloat16::bfloat16(0.5) );
	REQUIRE(true == bfloat16::bfloat16(0.1) < bfloat16::bfloat16(0.5) );
	REQUIRE(false == bfloat16::bfloat16(0.5) < bfloat16::bfloat16(0.1) );

	REQUIRE(false == bfloat16::bfloat16(10) < bfloat16::bfloat16(10) );
	REQUIRE(false == bfloat16::bfloat16(20) < bfloat16::bfloat16(10) );
	REQUIRE(true == bfloat16::bfloat16(10) < bfloat16::bfloat16(20) );

	REQUIRE(false == bfloat16::bfloat16(10.1) < bfloat16::bfloat16(10.1) );
	REQUIRE(false == bfloat16::bfloat16(20.1) < bfloat16::bfloat16(10.1) );
	REQUIRE(true == bfloat16::bfloat16(10.1) < bfloat16::bfloat16(20.1) );

	REQUIRE(false == bfloat16::bfloat16(10.5) < bfloat16::bfloat16(10.5) );
	REQUIRE(false == bfloat16::bfloat16(20.5) < bfloat16::bfloat16(10.5) );
	REQUIRE(true == bfloat16::bfloat16(10.5) < bfloat16::bfloat16(20.5) );


	//negative
	REQUIRE(false == bfloat16::bfloat16(-0.5) < bfloat16::bfloat16(-0.5) );
	REQUIRE(false == bfloat16::bfloat16(-0.1) < bfloat16::bfloat16(-0.5) );
	REQUIRE(true == bfloat16::bfloat16(-0.5) < bfloat16::bfloat16(-0.1) );

	REQUIRE(false == bfloat16::bfloat16(-10) < bfloat16::bfloat16(-10) );
	REQUIRE(true == bfloat16::bfloat16(-20) < bfloat16::bfloat16(-10) );
	REQUIRE(false == bfloat16::bfloat16(-10) < bfloat16::bfloat16(-20) );

	REQUIRE(false == bfloat16::bfloat16(-10.1) < bfloat16::bfloat16(-10.1) );
	REQUIRE(true == bfloat16::bfloat16(-20.1) < bfloat16::bfloat16(-10.1) );
	REQUIRE(false == bfloat16::bfloat16(-10.1) < bfloat16::bfloat16(-20.1) );

	REQUIRE(false == bfloat16::bfloat16(-10.5) < bfloat16::bfloat16(-10.5) );
	REQUIRE(true == bfloat16::bfloat16(-20.5) < bfloat16::bfloat16(-10.5) );
	REQUIRE(false == bfloat16::bfloat16(-10.5) < bfloat16::bfloat16(-20.5) );

}



TEST_CASE("bfloat16_operator_greater", "[float16]"){

	//works

	//numeric Limits
	REQUIRE(false == 0x7F80 > std::numeric_limits<bfloat16::bfloat16>::infinity());
	REQUIRE(false == 0x7FFF > std::numeric_limits<bfloat16::bfloat16>::quiet_NaN());
	REQUIRE(false == 0x0080 > std::numeric_limits<bfloat16::bfloat16>::min());
	REQUIRE(false == 0xFF7F > std::numeric_limits<bfloat16::bfloat16>::lowest());
	REQUIRE(false == 0x7F7F > std::numeric_limits<bfloat16::bfloat16>::max());
	REQUIRE(false == 0x0280 > std::numeric_limits<bfloat16::bfloat16>::epsilon());
	REQUIRE(false == 0x3F00 > std::numeric_limits<bfloat16::bfloat16>::round_error());
	REQUIRE(false == 0x7FBF > std::numeric_limits<bfloat16::bfloat16>::signaling_NaN());
	REQUIRE(false == 0x0001 > std::numeric_limits<bfloat16::bfloat16>::denorm_min());


	//positive
	REQUIRE(false == bfloat16::bfloat16(0.5) > bfloat16::bfloat16(0.5) );
	REQUIRE(false == bfloat16::bfloat16(0.1) > bfloat16::bfloat16(0.5) );
	REQUIRE(true == bfloat16::bfloat16(0.5) > bfloat16::bfloat16(0.1) );

	REQUIRE(false == bfloat16::bfloat16(10) > bfloat16::bfloat16(10) );
	REQUIRE(true == bfloat16::bfloat16(20) > bfloat16::bfloat16(10) );
	REQUIRE(false == bfloat16::bfloat16(10) > bfloat16::bfloat16(20) );

	REQUIRE(false == bfloat16::bfloat16(10.1) > bfloat16::bfloat16(10.1) );
	REQUIRE(true == bfloat16::bfloat16(20.1) > bfloat16::bfloat16(10.1) );
	REQUIRE(false == bfloat16::bfloat16(10.1) > bfloat16::bfloat16(20.1) );

	REQUIRE(false == bfloat16::bfloat16(10.5) > bfloat16::bfloat16(10.5) );
	REQUIRE(true == bfloat16::bfloat16(20.5) > bfloat16::bfloat16(10.5) );
	REQUIRE(false == bfloat16::bfloat16(10.5) > bfloat16::bfloat16(20.5) );


	//negative
	REQUIRE(false == bfloat16::bfloat16(-0.5) > bfloat16::bfloat16(-0.5) );
	REQUIRE(true == bfloat16::bfloat16(-0.1) > bfloat16::bfloat16(-0.5) );
	REQUIRE(false == bfloat16::bfloat16(-0.5) > bfloat16::bfloat16(-0.1) );

	REQUIRE(false == bfloat16::bfloat16(-10) > bfloat16::bfloat16(-10) );
	REQUIRE(false == bfloat16::bfloat16(-20) > bfloat16::bfloat16(-10) );
	REQUIRE(true == bfloat16::bfloat16(-10) > bfloat16::bfloat16(-20) );

	REQUIRE(false == bfloat16::bfloat16(-10.1) > bfloat16::bfloat16(-10.1) );
	REQUIRE(false == bfloat16::bfloat16(-20.1) > bfloat16::bfloat16(-10.1) );
	REQUIRE(true == bfloat16::bfloat16(-10.1) > bfloat16::bfloat16(-20.1) );

	REQUIRE(false == bfloat16::bfloat16(-10.5) > bfloat16::bfloat16(-10.5) );
	REQUIRE(false == bfloat16::bfloat16(-20.5) > bfloat16::bfloat16(-10.5) );
	REQUIRE(true == bfloat16::bfloat16(-10.5) > bfloat16::bfloat16(-20.5) );

}



TEST_CASE("bfloat16_operator_lessequal", "[float16]"){

	//works

	//numeric Limits
	REQUIRE(true == 0x7F80 <= std::numeric_limits<bfloat16::bfloat16>::infinity());
	REQUIRE(true == 0x7FFF <= std::numeric_limits<bfloat16::bfloat16>::quiet_NaN());
	REQUIRE(true == 0x0080 <= std::numeric_limits<bfloat16::bfloat16>::min());
	REQUIRE(true == 0xFF7F <= std::numeric_limits<bfloat16::bfloat16>::lowest());
	REQUIRE(true == 0x7F7F <= std::numeric_limits<bfloat16::bfloat16>::max());
	REQUIRE(true == 0x0280 <= std::numeric_limits<bfloat16::bfloat16>::epsilon());
	REQUIRE(true == 0x3F00 <= std::numeric_limits<bfloat16::bfloat16>::round_error());
	REQUIRE(true == 0x7FBF <= std::numeric_limits<bfloat16::bfloat16>::signaling_NaN());
	REQUIRE(true == 0x0001 <= std::numeric_limits<bfloat16::bfloat16>::denorm_min());


	//positive
	REQUIRE(true == bfloat16::bfloat16(0.5) <= bfloat16::bfloat16(0.5) );
	REQUIRE(true == bfloat16::bfloat16(0.1) <= bfloat16::bfloat16(0.5) );
	REQUIRE(false == bfloat16::bfloat16(0.5) <= bfloat16::bfloat16(0.1) );

	REQUIRE(true == bfloat16::bfloat16(10) <= bfloat16::bfloat16(10) );
	REQUIRE(false == bfloat16::bfloat16(20) <= bfloat16::bfloat16(10) );
	REQUIRE(true == bfloat16::bfloat16(10) <= bfloat16::bfloat16(20) );

	REQUIRE(true == bfloat16::bfloat16(10.1) <= bfloat16::bfloat16(10.1) );
	REQUIRE(false == bfloat16::bfloat16(20.1) <= bfloat16::bfloat16(10.1) );
	REQUIRE(true == bfloat16::bfloat16(10.1) <= bfloat16::bfloat16(20.1) );

	REQUIRE(true == bfloat16::bfloat16(10.5) <= bfloat16::bfloat16(10.5) );
	REQUIRE(false == bfloat16::bfloat16(20.5) <= bfloat16::bfloat16(10.5) );
	REQUIRE(true == bfloat16::bfloat16(10.5) <= bfloat16::bfloat16(20.5) );


	//negative
	REQUIRE(true == bfloat16::bfloat16(-0.5) <= bfloat16::bfloat16(-0.5) );
	REQUIRE(false == bfloat16::bfloat16(-0.1) <= bfloat16::bfloat16(-0.5) );
	REQUIRE(true == bfloat16::bfloat16(-0.5) <= bfloat16::bfloat16(-0.1) );

	REQUIRE(true == bfloat16::bfloat16(-10) <= bfloat16::bfloat16(-10) );
	REQUIRE(true == bfloat16::bfloat16(-20) <= bfloat16::bfloat16(-10) );
	REQUIRE(false == bfloat16::bfloat16(-10) <= bfloat16::bfloat16(-20) );

	REQUIRE(true == bfloat16::bfloat16(-10.1) <= bfloat16::bfloat16(-10.1) );
	REQUIRE(true == bfloat16::bfloat16(-20.1) <= bfloat16::bfloat16(-10.1) );
	REQUIRE(false == bfloat16::bfloat16(-10.1) <= bfloat16::bfloat16(-20.1) );

	REQUIRE(true == bfloat16::bfloat16(-10.5) <= bfloat16::bfloat16(-10.5) );
	REQUIRE(true == bfloat16::bfloat16(-20.5) <= bfloat16::bfloat16(-10.5) );
	REQUIRE(false == bfloat16::bfloat16(-10.5) <= bfloat16::bfloat16(-20.5) );

}



TEST_CASE("bfloat16_operator_greaterequal", "[float16]"){

	//works

	//numeric Limits
	REQUIRE(true == 0x7F80 >= std::numeric_limits<bfloat16::bfloat16>::infinity());
	REQUIRE(true == 0x7FFF >= std::numeric_limits<bfloat16::bfloat16>::quiet_NaN());
	REQUIRE(true == 0x0080 >= std::numeric_limits<bfloat16::bfloat16>::min());
	REQUIRE(true == 0xFF7F >= std::numeric_limits<bfloat16::bfloat16>::lowest());
	REQUIRE(true == 0x7F7F >= std::numeric_limits<bfloat16::bfloat16>::max());
	REQUIRE(true == 0x0280 >= std::numeric_limits<bfloat16::bfloat16>::epsilon());
	REQUIRE(true == 0x3F00 >= std::numeric_limits<bfloat16::bfloat16>::round_error());
	REQUIRE(true == 0x7FBF >= std::numeric_limits<bfloat16::bfloat16>::signaling_NaN());
	REQUIRE(true == 0x0001 >= std::numeric_limits<bfloat16::bfloat16>::denorm_min());


	//positive
	REQUIRE(true == bfloat16::bfloat16(0.5) >= bfloat16::bfloat16(0.5) );
	REQUIRE(false == bfloat16::bfloat16(0.1) >= bfloat16::bfloat16(0.5) );
	REQUIRE(true == bfloat16::bfloat16(0.5) >= bfloat16::bfloat16(0.1) );

	REQUIRE(true == bfloat16::bfloat16(10) >= bfloat16::bfloat16(10) );
	REQUIRE(true == bfloat16::bfloat16(20) >= bfloat16::bfloat16(10) );
	REQUIRE(false == bfloat16::bfloat16(10) >= bfloat16::bfloat16(20) );

	REQUIRE(true == bfloat16::bfloat16(10.1) >= bfloat16::bfloat16(10.1) );
	REQUIRE(true == bfloat16::bfloat16(20.1) >= bfloat16::bfloat16(10.1) );
	REQUIRE(false == bfloat16::bfloat16(10.1) >= bfloat16::bfloat16(20.1) );

	REQUIRE(true == bfloat16::bfloat16(10.5) >= bfloat16::bfloat16(10.5) );
	REQUIRE(true == bfloat16::bfloat16(20.5) >= bfloat16::bfloat16(10.5) );
	REQUIRE(false == bfloat16::bfloat16(10.5) >= bfloat16::bfloat16(20.5) );

	
	//negative
	REQUIRE(true == bfloat16::bfloat16(-0.5) >= bfloat16::bfloat16(-0.5) );
	REQUIRE(true == bfloat16::bfloat16(-0.1) >= bfloat16::bfloat16(-0.5) );
	REQUIRE(false == bfloat16::bfloat16(-0.5) >= bfloat16::bfloat16(-0.1) );

	REQUIRE(true == bfloat16::bfloat16(-10) >= bfloat16::bfloat16(-10) );
	REQUIRE(false == bfloat16::bfloat16(-20) >= bfloat16::bfloat16(-10) );
	REQUIRE(true == bfloat16::bfloat16(-10) >= bfloat16::bfloat16(-20) );

	REQUIRE(true == bfloat16::bfloat16(-10.1) >= bfloat16::bfloat16(-10.1) );
	REQUIRE(false == bfloat16::bfloat16(-20.1) >= bfloat16::bfloat16(-10.1) );
	REQUIRE(true == bfloat16::bfloat16(-10.1) >= bfloat16::bfloat16(-20.1) );

	REQUIRE(true == bfloat16::bfloat16(-10.5) >= bfloat16::bfloat16(-10.5) );
	REQUIRE(false == bfloat16::bfloat16(-20.5) >= bfloat16::bfloat16(-10.5) );
	REQUIRE(true == bfloat16::bfloat16(-10.5) >= bfloat16::bfloat16(-20.5) );

}



TEST_CASE("bfloat16_operator_positive", "[float16]"){

	//works

	//numeric Limits
	REQUIRE(0x7F80 == + std::numeric_limits<bfloat16::bfloat16>::infinity());
	REQUIRE(0x7FFF == + std::numeric_limits<bfloat16::bfloat16>::quiet_NaN());
	REQUIRE(0x0080 == + std::numeric_limits<bfloat16::bfloat16>::min());
	REQUIRE(0xFF7F == + std::numeric_limits<bfloat16::bfloat16>::lowest());
	REQUIRE(0x7F7F == + std::numeric_limits<bfloat16::bfloat16>::max());
	REQUIRE(0x0280 == + std::numeric_limits<bfloat16::bfloat16>::epsilon());
	REQUIRE(0x3F00 == + std::numeric_limits<bfloat16::bfloat16>::round_error());
	REQUIRE(0x7FBF == + std::numeric_limits<bfloat16::bfloat16>::signaling_NaN());
	REQUIRE(0x0001 == + std::numeric_limits<bfloat16::bfloat16>::denorm_min());


	//positive
	REQUIRE(0x3DCC == + bfloat16::bfloat16(0.1) ); // 0x3DCC = 0.099609375 bf16
	REQUIRE(0x3F00 == + bfloat16::bfloat16(0.5) ); // 0x3F00 = 0.5 bf16
	REQUIRE(0x4120 == + bfloat16::bfloat16(10) ); // 0x4120 = 10.0 bf16
	REQUIRE(0x4121 == + bfloat16::bfloat16(10.1) ); // 0x4121 = 10.0625 bf16
	REQUIRE(0x4128 == + bfloat16::bfloat16(10.5) ); // 0x4128 = 10.5 bf16


	//negative
	REQUIRE(0xBDCC == + bfloat16::bfloat16(-0.1) ); // 0xBDCC = -0.099609375 bf16
	REQUIRE(0xBF00 == + bfloat16::bfloat16(-0.5) ); // 0xBF00 = -0.5 bf16
	REQUIRE(0xC120 == + bfloat16::bfloat16(-10) ); // 0xC120 = -10.0 bf16
	REQUIRE(0xC121 == + bfloat16::bfloat16(-10.1) ); // 0xC121 = -10.0625 bf16
	REQUIRE(0xC128 == + bfloat16::bfloat16(-10.5) ); // 0xC128 = -10.5 bf16

}



TEST_CASE("bfloat16_operator_negative", "[float16]"){

	//works

	//numeric Limits
	REQUIRE(0xFF80 == - std::numeric_limits<bfloat16::bfloat16>::infinity());
	REQUIRE(0xfFFF == - std::numeric_limits<bfloat16::bfloat16>::quiet_NaN());
	REQUIRE(0x8080 == - std::numeric_limits<bfloat16::bfloat16>::min());
	REQUIRE(0x7F7F == - std::numeric_limits<bfloat16::bfloat16>::lowest());
	REQUIRE(0xFF7F == - std::numeric_limits<bfloat16::bfloat16>::max());
	REQUIRE(0x8280 == - std::numeric_limits<bfloat16::bfloat16>::epsilon());
	REQUIRE(0xBF00 == - std::numeric_limits<bfloat16::bfloat16>::round_error());
	REQUIRE(0xFFBF == - std::numeric_limits<bfloat16::bfloat16>::signaling_NaN());
	REQUIRE(0x8001 == - std::numeric_limits<bfloat16::bfloat16>::denorm_min());


	//positive
	REQUIRE(0xBDCC == - bfloat16::bfloat16(0.1) ); // 0xBDCC = -0.099609375 bf16
	REQUIRE(0xBF00 == - bfloat16::bfloat16(0.5) ); // 0xBF00 = -0.5 bf16
	REQUIRE(0xC120 == - bfloat16::bfloat16(10) ); // 0xC120 = -10.0 bf16
	REQUIRE(0xC121 == - bfloat16::bfloat16(10.1) ); // 0xC121 = -10.0625 bf16
	REQUIRE(0xC128 == - bfloat16::bfloat16(10.5) ); // 0xC128 = -10.5 bf16


	//negative
	REQUIRE(0x3DCC == - bfloat16::bfloat16(-0.1) ); // 0x3DCC = 0.099609375 bf16
	REQUIRE(0x3F00 == - bfloat16::bfloat16(-0.5) ); // 0x3F00 = 0.5 bf16
	REQUIRE(0x4120 == - bfloat16::bfloat16(-10) ); // 0x4120 = 10.0 bf16
	REQUIRE(0x4121 == - bfloat16::bfloat16(-10.1) ); // 0x4121 = 10.0625 bf16
	REQUIRE(0x4128 == - bfloat16::bfloat16(-10.5) ); // 0x4128 = 10.5 bf16

}



TEST_CASE("Temp_BF16_ADD", "[float16]"){

	//calculated with the temporary solution


	REQUIRE(0x3E4C == bfloat16::bfloat16(0.1) + bfloat16::bfloat16(0.1) ); // 0x3e4c = 0.19921875 bf16
	REQUIRE(0x0 == bfloat16::bfloat16(-0.1) + bfloat16::bfloat16(0.1) ); // 0x0 = 0.0 bf16
	REQUIRE(0x0 == bfloat16::bfloat16(0.1) + bfloat16::bfloat16(-0.1) ); // 0x0 = 0.0 bf16
	REQUIRE(0xBE4C == bfloat16::bfloat16(-0.1) + bfloat16::bfloat16(-0.1) ); // 0xBE4C h = -0.19921875 bf16


	REQUIRE(0x3F80 == bfloat16::bfloat16(0.5) + bfloat16::bfloat16(0.5) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x0 == bfloat16::bfloat16(-0.5) + bfloat16::bfloat16(0.5) ); // 0x0 = 0.0 bf16
	REQUIRE(0x0 == bfloat16::bfloat16(0.5) + bfloat16::bfloat16(-0.5) ); // 0x0 = 0.0 bf16
	REQUIRE(0xBF80 == bfloat16::bfloat16(-0.5) + bfloat16::bfloat16(-0.5) ); // 0xBF80 = -1.0 bf16


	REQUIRE(0x40A0 == bfloat16::bfloat16(10) + bfloat16::bfloat16(-5) ); // 0x40A0 = 5.0 bf16
	REQUIRE(0x4080 == bfloat16::bfloat16(10) + bfloat16::bfloat16(-6) ); // 0x4080 = 4.0 bf16
	REQUIRE(0x4040 == bfloat16::bfloat16(10) + bfloat16::bfloat16(-7) ); // 0x4040 = 3.0 bf16
	REQUIRE(0x4000 == bfloat16::bfloat16(10) + bfloat16::bfloat16(-8) ); // 0x4000 = 2.0 bf16


	REQUIRE(0x41A0 == bfloat16::bfloat16(10) + bfloat16::bfloat16(10) ); // 0x41A0 = 20.0 bf16
	REQUIRE(0x0 == bfloat16::bfloat16(-10) + bfloat16::bfloat16(10) ); // 0x0 = 0.0 bf16
	REQUIRE(0x0 == bfloat16::bfloat16(10) + bfloat16::bfloat16(-10) ); // 0x0 = 0.0 bf16
	REQUIRE(0xC1A0 == bfloat16::bfloat16(-10) + bfloat16::bfloat16(-10) ); // 0xC1A0 = -20.0 bf16


	REQUIRE(0x41A1 == bfloat16::bfloat16(10.1) + bfloat16::bfloat16(10.1) ); // 0x41A1 = 20.125 bf16
	REQUIRE(0x0 == bfloat16::bfloat16(-10.1) + bfloat16::bfloat16(10.1) ); // 0x0 = 0.0 bf16
	REQUIRE(0x0 == bfloat16::bfloat16(10.1) + bfloat16::bfloat16(-10.1) ); // 0x0 = 0.0 bf16
	REQUIRE(0xC1A1 == bfloat16::bfloat16(-10.1) + bfloat16::bfloat16(-10.1) ); // 0xC1A1 h = -20.125 bf16


	REQUIRE(0x41A8 == bfloat16::bfloat16(10.5) + bfloat16::bfloat16(10.5) ); // 0x41A8 = 21.0 bf16
	REQUIRE(0x0 == bfloat16::bfloat16(-10.5) + bfloat16::bfloat16(10.5) ); // 0x0 = 0.0 bf16
	REQUIRE(0x0 == bfloat16::bfloat16(10.5) + bfloat16::bfloat16(-10.5) ); // 0x0 = 0.0 bf16
	REQUIRE(0xC1A8 == bfloat16::bfloat16(-10.5) + bfloat16::bfloat16(-10.5) ); // 0xC1A8 h = -21.0 bf16

}



//TODO 
TEST_CASE("bfloat16_operator_add", "[floa16]"){

	//Arithmetic type is not enabled for me
	//works ,or not


	//REQUIRE(0x40A0 == bfloat16::bfloat16(10) + bfloat16::bfloat16(-5));
	//REQUIRE(0x4080 == bfloat16::bfloat16(10) + bfloat16::bfloat16(-6));
	//REQUIRE(0x4040 == bfloat16::bfloat16(10) + bfloat16::bfloat16(-7));
	//REQUIRE(0x4000 == bfloat16::bfloat16(10) + bfloat16::bfloat16(-8));

	
	//positive
	REQUIRE(0x3E4C == bfloat16::bfloat16(0.1) + bfloat16::bfloat16(0.1));
	REQUIRE(0x0 == bfloat16::bfloat16(-0.1) + bfloat16::bfloat16(0.1));
	REQUIRE(0x3F80 == bfloat16::bfloat16(0.5) + bfloat16::bfloat16(0.5));
	REQUIRE(0x0 == bfloat16::bfloat16(-0.5) + bfloat16::bfloat16(0.5));

	REQUIRE(0x41A0 == bfloat16::bfloat16(10) + bfloat16::bfloat16(10));
	REQUIRE(0x0 == bfloat16::bfloat16(-10) + bfloat16::bfloat16(10));
	REQUIRE(0x41A1 == bfloat16::bfloat16(10.1) + bfloat16::bfloat16(10.1));
	REQUIRE(0x0 == bfloat16::bfloat16(-10.1) + bfloat16::bfloat16(10.1));

	REQUIRE(0x41A8 == bfloat16::bfloat16(10.5) + bfloat16::bfloat16(10.5));
	REQUIRE(0x0 == bfloat16::bfloat16(-10.5) + bfloat16::bfloat16(10.5));


	//negative
	REQUIRE(0x0 == bfloat16::bfloat16(0.1) + bfloat16::bfloat16(-0.1));
	REQUIRE(0xBE4C == bfloat16::bfloat16(-0.1) + bfloat16::bfloat16(-0.1));
	REQUIRE(0x0 == bfloat16::bfloat16(0.5) + bfloat16::bfloat16(-0.5));
	REQUIRE(0xBF80 == bfloat16::bfloat16(-0.5) + bfloat16::bfloat16(-0.5));

	REQUIRE(0x0 == bfloat16::bfloat16(10) + bfloat16::bfloat16(-10));
	REQUIRE(0xC1A0 == bfloat16::bfloat16(-10) + bfloat16::bfloat16(-10));
	REQUIRE(0x0 == bfloat16::bfloat16(10.1) + bfloat16::bfloat16(-10.1));
	REQUIRE(0xC1A1 == bfloat16::bfloat16(-10.1) + bfloat16::bfloat16(-10.1));

	REQUIRE(0x0 == bfloat16::bfloat16(10.5) + bfloat16::bfloat16(-10.5));
	REQUIRE(0xC1A8 == bfloat16::bfloat16(-10.5) + bfloat16::bfloat16(-10.5));

}



TEST_CASE("bfloat16_operator_sub", "[float16]"){

	//works, depends on add above, at the moment with the temp solution

	//REQUIRE(0x40A0 == bfloat16::bfloat16(10) - bfloat16::bfloat16(5));


	REQUIRE(0x0 == bfloat16::bfloat16(0.1) - bfloat16::bfloat16(0.1) ); // 0x0 = 0.0 bf16
	REQUIRE(0xBE4C == bfloat16::bfloat16(-0.1) - bfloat16::bfloat16(0.1) ); // 0xBE4C = -0.19921875 bf16
	REQUIRE(0x3E4C == bfloat16::bfloat16(0.1) - bfloat16::bfloat16(-0.1) ); // 0x3E4C = 0.19921875 bf16
	REQUIRE(0x0 == bfloat16::bfloat16(-0.1) - bfloat16::bfloat16(-0.1) ); // 0x0 = 0.0 bf16


	REQUIRE(0x0 == bfloat16::bfloat16(0.5) - bfloat16::bfloat16(0.5) ); // 0x0 = 0.0 bf16
	REQUIRE(0xBF80 == bfloat16::bfloat16(-0.5) - bfloat16::bfloat16(0.5) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0x3F80 == bfloat16::bfloat16(0.5) - bfloat16::bfloat16(-0.5) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x0 == bfloat16::bfloat16(-0.5) - bfloat16::bfloat16(-0.5) ); // 0x0 = 0.0 bf16


	REQUIRE(0x0 == bfloat16::bfloat16(10) - bfloat16::bfloat16(10) ); // 0x0 = 0.0 bf16
	REQUIRE(0xC1A0 == bfloat16::bfloat16(-10) - bfloat16::bfloat16(10) ); // 0xC1A0 = -20.0 bf16
	REQUIRE(0x41A0 == bfloat16::bfloat16(10) - bfloat16::bfloat16(-10) ); // 0x41A0 = 10.0 bf16
	REQUIRE(0x0 == bfloat16::bfloat16(-10) - bfloat16::bfloat16(-10) ); // 0x0 = 0.0 bf16

	REQUIRE(0x0 == bfloat16::bfloat16(10.1) - bfloat16::bfloat16(10.1) ); // 0x0 = 0.0 bf16
	REQUIRE(0xC1A1 == bfloat16::bfloat16(-10.1) - bfloat16::bfloat16(10.1) ); // 0xC1A1 = -20.125 bf16
	REQUIRE(0x41A1 == bfloat16::bfloat16(10.1) - bfloat16::bfloat16(-10.1) ); // 0x41A1 = 20.125 bf16
	REQUIRE(0x0 == bfloat16::bfloat16(-10.1) - bfloat16::bfloat16(-10.1) ); // 0x0 = 0.0 bf16


	REQUIRE(0x0 == bfloat16::bfloat16(10.5) - bfloat16::bfloat16(10.5) ); // 0x0 = 0.0 bf16
	REQUIRE(0xC1A8 == bfloat16::bfloat16(-10.5) - bfloat16::bfloat16(10.5) ); // 0xC1A8 = -21.0 bf16
	REQUIRE(0x41A8 == bfloat16::bfloat16(10.5) - bfloat16::bfloat16(-10.5) ); // 0x41A8 = 21.0 bf16
	REQUIRE(0x0 == bfloat16::bfloat16(-10.5) - bfloat16::bfloat16(-10.5) ); // 0x0 = 0.0 bf16

}



TEST_CASE("bfloat16_operator_mult", "[float16]"){

	//calculated with the temporary solution


	REQUIRE(0x0 == bfloat16::bfloat16(0) * bfloat16::bfloat16(0) ); // 0x0 = 0.0 bf16

	REQUIRE(0x3C22 == bfloat16::bfloat16(0.1) * bfloat16::bfloat16(0.1) ); // 0x3C22 = 0.0098876953125 bf16
	REQUIRE(0xBC22 == bfloat16::bfloat16(-0.1) * bfloat16::bfloat16(0.1) ); // 0xBC22 = -0.0098876953125 bf16
	REQUIRE(0xBC22 == bfloat16::bfloat16(0.1) * bfloat16::bfloat16(-0.1) ); // 0xBC22 = -0.0098876953125 bf16
	REQUIRE(0x3C22 == bfloat16::bfloat16(-0.1) * bfloat16::bfloat16(-0.1) ); // 0x3C22 = 0.0098876953125 bf16


	REQUIRE(0x3E80 == bfloat16::bfloat16(0.5) * bfloat16::bfloat16(0.5) ); // 0x3E80 = 0.25 bf16
	REQUIRE(0xBE80 == bfloat16::bfloat16(-0.5) * bfloat16::bfloat16(0.5) ); // 0xBE80 = -0.25 bf16
	REQUIRE(0xBE80 == bfloat16::bfloat16(0.5) * bfloat16::bfloat16(-0.5) ); // 0xBE80 = -0.25 bf16
	REQUIRE(0x3E80 == bfloat16::bfloat16(-0.5) * bfloat16::bfloat16(-0.5) ); // 0x3E80 = 0.25 bf16


	REQUIRE(0x42C8 == bfloat16::bfloat16(10) * bfloat16::bfloat16(10) ); // 0x42C8 = 100.0 bf16
	REQUIRE(0xC2C8 == bfloat16::bfloat16(-10) * bfloat16::bfloat16(10) ); // 0xC2C8 = -100.0 bf16
	REQUIRE(0xC2C8 == bfloat16::bfloat16(10) * bfloat16::bfloat16(-10) ); // 0xC2C8 = -100.0 bf16
	REQUIRE(0x42C8 == bfloat16::bfloat16(-10) * bfloat16::bfloat16(-10) ); // 0x42C8 = 100.0 bf16


	REQUIRE(0x42CA == bfloat16::bfloat16(10.1) * bfloat16::bfloat16(10.1) ); // 0x42CA = 101.0 bf16
	REQUIRE(0xC2CA == bfloat16::bfloat16(-10.1) * bfloat16::bfloat16(10.1) ); // 0xC2CA = -101.0 bf16
	REQUIRE(0xC2CA == bfloat16::bfloat16(10.1) * bfloat16::bfloat16(-10.1) ); // 0xC2CA = -101.0 bf16
	REQUIRE(0x42CA == bfloat16::bfloat16(-10.1) * bfloat16::bfloat16(-10.1) ); // 0x42CA = 101.0 bf16


	REQUIRE(0x42DC == bfloat16::bfloat16(10.5) * bfloat16::bfloat16(10.5) ); // 0x42DC = 110.0 bf16
	REQUIRE(0xC2DC == bfloat16::bfloat16(-10.5) * bfloat16::bfloat16(10.5) ); // 0xC2DC = -110.0 bf16
	REQUIRE(0xC2DC == bfloat16::bfloat16(10.5) * bfloat16::bfloat16(-10.5) ); // 0xC2DC = -110.0 bf16
	REQUIRE(0x42DC == bfloat16::bfloat16(-10.5) * bfloat16::bfloat16(-10.5) ); // 0x42DC = 110.0 bf16

}



TEST_CASE("bfloat16_operator_div", "[float16]"){

	//calculated with the temporary solution


	REQUIRE(0x3F80 == bfloat16::bfloat16(0.1) / bfloat16::bfloat16(0.1) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0xBF80 == bfloat16::bfloat16(-0.1) / bfloat16::bfloat16(0.1) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0xBF80 == bfloat16::bfloat16(0.1) / bfloat16::bfloat16(-0.1) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0x3F80 == bfloat16::bfloat16(-0.1) / bfloat16::bfloat16(-0.1) ); // 0x3F80 = 1.0 bf16


	REQUIRE(0x3F80 == bfloat16::bfloat16(0.5) / bfloat16::bfloat16(0.5) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0xBF80 == bfloat16::bfloat16(-0.5) / bfloat16::bfloat16(0.5) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0xBF80 == bfloat16::bfloat16(0.5) / bfloat16::bfloat16(-0.5) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0x3F80 == bfloat16::bfloat16(-0.5) / bfloat16::bfloat16(-0.5) ); // 0x3F80 = 1.0 bf16


	REQUIRE(0x3F80 == bfloat16::bfloat16(10) / bfloat16::bfloat16(10) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0xBF80 == bfloat16::bfloat16(-10) / bfloat16::bfloat16(10) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0xBF80 == bfloat16::bfloat16(10) / bfloat16::bfloat16(-10) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0x3F80 == bfloat16::bfloat16(-10) / bfloat16::bfloat16(-10) ); // 0x3F80 = 1.0 bf16


	REQUIRE(0x3F80 == bfloat16::bfloat16(10.1) / bfloat16::bfloat16(10.1) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0xBF80 == bfloat16::bfloat16(-10.1) / bfloat16::bfloat16(10.1) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0xBF80 == bfloat16::bfloat16(10.1) / bfloat16::bfloat16(-10.1) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0x3F80 == bfloat16::bfloat16(-10.1) / bfloat16::bfloat16(-10.1) ); // 0x3F80 = 1.0 bf16


	REQUIRE(0x3F80 == bfloat16::bfloat16(10.5) / bfloat16::bfloat16(10.5) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0xBF80 == bfloat16::bfloat16(-10.5) / bfloat16::bfloat16(10.5) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0xBF80 == bfloat16::bfloat16(10.5) / bfloat16::bfloat16(-10.5) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0x3F80 == bfloat16::bfloat16(-10.5) / bfloat16::bfloat16(-10.5) ); // 0x3F80 = 1.0 bf16

}



TEST_CASE("bfloat16_operator_input_output", "[floa16]"){
	//TODO input
	//works

	//positive
	std::cout << bfloat16::bfloat16(0.1) << std::endl;
	std::cout << bfloat16::bfloat16(0.5) << std::endl;

	//negative
	std::cout << bfloat16::bfloat16(-0.1) << std::endl;
	std::cout << bfloat16::bfloat16(-0.5) << std::endl;

}



TEST_CASE("bfloat16_fabs", "[float16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(0x7F80 == bfloat16::fabs(std::numeric_limits<bfloat16::bfloat16>::infinity()) ); // input +inf
	REQUIRE(0x7F80 == bfloat16::fabs(-std::numeric_limits<bfloat16::bfloat16>::infinity()) ); // input -inf
	REQUIRE(0x7FFF == bfloat16::fabs(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	REQUIRE(0x0080 == bfloat16::fabs(std::numeric_limits<bfloat16::bfloat16>::min()) );
	REQUIRE(0x7F7F == bfloat16::fabs(std::numeric_limits<bfloat16::bfloat16>::lowest()) );
	REQUIRE(0x7F7F == bfloat16::fabs(std::numeric_limits<bfloat16::bfloat16>::max()) );
	REQUIRE(0x0280 == bfloat16::fabs(std::numeric_limits<bfloat16::bfloat16>::epsilon()) );
	REQUIRE(0x3F00 == bfloat16::fabs(std::numeric_limits<bfloat16::bfloat16>::round_error()) );
	REQUIRE(0x7FBF == bfloat16::fabs(std::numeric_limits<bfloat16::bfloat16>::signaling_NaN()) );
	REQUIRE(0x0001 == bfloat16::fabs(std::numeric_limits<bfloat16::bfloat16>::denorm_min()) );

	
	//positive
	REQUIRE(0x0 == bfloat16::fabs(bfloat16::bfloat16(0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3DCC == bfloat16::fabs(bfloat16::bfloat16(0.1)) ); // 0x3DCC = 0.099609375 bf16
	REQUIRE(0x3F00 == bfloat16::fabs(bfloat16::bfloat16(0.5)) ); // 0x3F00 = 0.5 bf16
	REQUIRE(0x4120 == bfloat16::fabs(bfloat16::bfloat16(10)) ); // 0x4120 = 10.0 bf16
	REQUIRE(0x4121 == bfloat16::fabs(bfloat16::bfloat16(10.1)) ); // 0x4121 = 10.0625 bf16
	REQUIRE(0x4128 == bfloat16::fabs(bfloat16::bfloat16(10.5)) ); // 0x4128 = 10.5 bf16


	//negative
	REQUIRE(0x0 == bfloat16::fabs(bfloat16::bfloat16(-0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3DCC == bfloat16::fabs(bfloat16::bfloat16(-0.1)) ); // 0x3DCC = 0.099609375 bf16
	REQUIRE(0x3F00 == bfloat16::fabs(bfloat16::bfloat16(-0.5)) ); // 0x3F00 = 0.5 bf16
	REQUIRE(0x4120 == bfloat16::fabs(bfloat16::bfloat16(-10)) ); // 0x4120 = 10.0 bf16
	REQUIRE(0x4121 == bfloat16::fabs(bfloat16::bfloat16(-10.1)) ); // 0x4121 = 10.0625 bf16
	REQUIRE(0x4128 == bfloat16::fabs(bfloat16::bfloat16(-10.5)) ); // 0x4128 = 10.5 bf16

}



TEST_CASE("bfloat16_abs", "[float16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(0x7F80 == bfloat16::abs(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x7FFF == bfloat16::abs(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	REQUIRE(0x0080 == bfloat16::abs(std::numeric_limits<bfloat16::bfloat16>::min()) );
	REQUIRE(0x7F7F == bfloat16::abs(std::numeric_limits<bfloat16::bfloat16>::lowest()) );
	REQUIRE(0x7F7F == bfloat16::abs(std::numeric_limits<bfloat16::bfloat16>::max()) );
	REQUIRE(0x0280 == bfloat16::abs(std::numeric_limits<bfloat16::bfloat16>::epsilon()) );
	REQUIRE(0x3F00 == bfloat16::abs(std::numeric_limits<bfloat16::bfloat16>::round_error()) );
	REQUIRE(0x7FBF == bfloat16::abs(std::numeric_limits<bfloat16::bfloat16>::signaling_NaN()) );
	REQUIRE(0x0001 == bfloat16::abs(std::numeric_limits<bfloat16::bfloat16>::denorm_min()) );


	//positive
	REQUIRE(0x3DCC == bfloat16::abs(bfloat16::bfloat16(0.1)) ); // 0x3DCC = 0.099609375 bf16
	REQUIRE(0x3F00 == bfloat16::abs(bfloat16::bfloat16(0.5)) ); // 0x3F00 = 0.5 bf16
	REQUIRE(0x4120 == bfloat16::abs(bfloat16::bfloat16(10)) ); // 0x4120 = 10.0 bf16
	REQUIRE(0x4121 == bfloat16::abs(bfloat16::bfloat16(10.1)) ); // 0x4121 = 10.0625 bf16
	REQUIRE(0x4128 == bfloat16::abs(bfloat16::bfloat16(10.5)) ); // 0x4128 = 10.5 bf16


	//negative
	REQUIRE(0x3DCC == bfloat16::abs(bfloat16::bfloat16(-0.1)) ); // 0x3DCC = 0.099609375 bf16
	REQUIRE(0x3F00 == bfloat16::abs(bfloat16::bfloat16(-0.5)) ); // 0x3F00 = 0.5 bf16
	REQUIRE(0x4120 == bfloat16::abs(bfloat16::bfloat16(-10)) ); // 0x4120 = 10.0 bf16
	REQUIRE(0x4121 == bfloat16::abs(bfloat16::bfloat16(-10.1)) ); // 0x4121 = 10.0625 bf16
	REQUIRE(0x4128 == bfloat16::abs(bfloat16::bfloat16(-10.5)) ); // 0x4128 = 10.5 bf16

}



TEST_CASE("bfloat16_fmod", "[float16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(0x0 == bfloat16::fmod(bfloat16::bfloat16(0), bfloat16::bfloat16(2)) );
	REQUIRE(0x0 == bfloat16::fmod(bfloat16::bfloat16(-0), bfloat16::bfloat16(2)) ); // should be -0

	REQUIRE(NAN == bfloat16::fmod(std::numeric_limits<bfloat16::bfloat16>::infinity(), bfloat16::bfloat16(2)) );
	REQUIRE(NAN == bfloat16::fmod(-std::numeric_limits<bfloat16::bfloat16>::infinity(), bfloat16::bfloat16(2)) );

	REQUIRE(NAN == bfloat16::fmod(bfloat16::bfloat16(2), bfloat16::bfloat16(0)) );
	REQUIRE(NAN == bfloat16::fmod(bfloat16::bfloat16(2), bfloat16::bfloat16(-2)) );

	REQUIRE(0x4000 == bfloat16::fmod(bfloat16::bfloat16(2), std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x4000 == bfloat16::fmod(bfloat16::bfloat16(2), -std::numeric_limits<bfloat16::bfloat16>::infinity()) );

	REQUIRE(NAN == bfloat16::fmod(bfloat16::bfloat16(2), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	REQUIRE(NAN == bfloat16::fmod(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(),bfloat16::bfloat16(2)) );



	//positive
	REQUIRE(0x0000 == bfloat16::fmod(bfloat16::bfloat16(10), bfloat16::bfloat16(2)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3F80 == bfloat16::fmod(bfloat16::bfloat16(10), bfloat16::bfloat16(3)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x4000 == bfloat16::fmod(bfloat16::bfloat16(10), bfloat16::bfloat16(4)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x0000 == bfloat16::fmod(bfloat16::bfloat16(10), bfloat16::bfloat16(5)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x4080 == bfloat16::fmod(bfloat16::bfloat16(10), bfloat16::bfloat16(6)) ); // 0x4080 = 4.0 bf16
	REQUIRE(0x4040 == bfloat16::fmod(bfloat16::bfloat16(10), bfloat16::bfloat16(7)) ); // 0x4040 = 3.0 bf16
	REQUIRE(0x4000 == bfloat16::fmod(bfloat16::bfloat16(10), bfloat16::bfloat16(8)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x3F80 == bfloat16::fmod(bfloat16::bfloat16(10), bfloat16::bfloat16(9)) ); // 0x3F80 = 1.0 bf16


	REQUIRE(0x0000 == bfloat16::fmod(bfloat16::bfloat16(10), bfloat16::bfloat16(2.5)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x4040 == bfloat16::fmod(bfloat16::bfloat16(10), bfloat16::bfloat16(3.5)) ); // 0x4040 = 3.0 bf16
	REQUIRE(0x3F80 == bfloat16::fmod(bfloat16::bfloat16(10), bfloat16::bfloat16(4.5)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x4090 == bfloat16::fmod(bfloat16::bfloat16(10), bfloat16::bfloat16(5.5)) ); // 0x4090 = 4.5 bf16
	REQUIRE(0x4060 == bfloat16::fmod(bfloat16::bfloat16(10), bfloat16::bfloat16(6.5)) ); // 0x4060 = 3.5 bf16
	REQUIRE(0x4020 == bfloat16::fmod(bfloat16::bfloat16(10), bfloat16::bfloat16(7.5)) ); // 0x4020 = 2.5 bf16
	REQUIRE(0x3FC0 == bfloat16::fmod(bfloat16::bfloat16(10), bfloat16::bfloat16(8.5)) ); // 0x3FC0 = 1.5 bf16
	REQUIRE(0x3F00 == bfloat16::fmod(bfloat16::bfloat16(10), bfloat16::bfloat16(9.5)) ); // 0x3F00 = 0.5 bf16


	//negative
	REQUIRE(0x8000 == bfloat16::fmod(bfloat16::bfloat16(-10), bfloat16::bfloat16(2)) ); // 0x8000 = -0.0 bf16
	REQUIRE(0xBF80 == bfloat16::fmod(bfloat16::bfloat16(-10), bfloat16::bfloat16(3)) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0xC000 == bfloat16::fmod(bfloat16::bfloat16(-10), bfloat16::bfloat16(4)) ); // 0xC000 = -2.0 bf16
	REQUIRE(0x8000 == bfloat16::fmod(bfloat16::bfloat16(-10), bfloat16::bfloat16(5)) ); // 0x8000 = -0.0 bf16
	REQUIRE(0xC080 == bfloat16::fmod(bfloat16::bfloat16(-10), bfloat16::bfloat16(6)) ); // 0xC080 = -4.0 bf16
	REQUIRE(0xC040 == bfloat16::fmod(bfloat16::bfloat16(-10), bfloat16::bfloat16(7)) ); // 0xC040 = -3.0 bf16
	REQUIRE(0xC000 == bfloat16::fmod(bfloat16::bfloat16(-10), bfloat16::bfloat16(8)) ); // 0xC000 = -2.0 bf16
	REQUIRE(0xBF80 == bfloat16::fmod(bfloat16::bfloat16(-10), bfloat16::bfloat16(9)) ); // 0xBF80 = -1.0 bf16


	REQUIRE(0x8000 == bfloat16::fmod(bfloat16::bfloat16(-10), bfloat16::bfloat16(2.5)) ); // 0x8000 = -0.0 bf16
	REQUIRE(0xC040 == bfloat16::fmod(bfloat16::bfloat16(-10), bfloat16::bfloat16(3.5)) ); // 0xC040 = -3.0 bf16
	REQUIRE(0xBF80 == bfloat16::fmod(bfloat16::bfloat16(-10), bfloat16::bfloat16(4.5)) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0xC090 == bfloat16::fmod(bfloat16::bfloat16(-10), bfloat16::bfloat16(5.5)) ); // 0xC090 = -4.5 bf16
	REQUIRE(0xC060 == bfloat16::fmod(bfloat16::bfloat16(-10), bfloat16::bfloat16(6.5)) ); // 0xC060 = -3.5 bf16
	REQUIRE(0xC020 == bfloat16::fmod(bfloat16::bfloat16(-10), bfloat16::bfloat16(7.5)) ); // 0xC020 = -2.5 bf16
	REQUIRE(0xBFC0 == bfloat16::fmod(bfloat16::bfloat16(-10), bfloat16::bfloat16(8.5)) ); // 0xBFC0 = -1.5 bf16
	REQUIRE(0xBF00 == bfloat16::fmod(bfloat16::bfloat16(-10), bfloat16::bfloat16(9.5)) ); // 0xBF00 = -0.5 bf16

}



TEST_CASE("bfloat16_remainder", "[float16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(NAN == bfloat16::remainder(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), bfloat16::bfloat16(2)) );
	REQUIRE(NAN == bfloat16::remainder(bfloat16::bfloat16(2), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );

	REQUIRE(NAN == bfloat16::remainder(bfloat16::bfloat16(2), bfloat16::bfloat16(0)) );
	REQUIRE(NAN == bfloat16::remainder(bfloat16::bfloat16(2), bfloat16::bfloat16(-0)) );

	REQUIRE(NAN == bfloat16::remainder(std::numeric_limits<bfloat16::bfloat16>::infinity(), bfloat16::bfloat16(2)) );
	REQUIRE(NAN == bfloat16::remainder(-std::numeric_limits<bfloat16::bfloat16>::infinity(), bfloat16::bfloat16(2)) );


	//positive
	REQUIRE(0x0000 == bfloat16::remainder(bfloat16::bfloat16(10), bfloat16::bfloat16(2)) ); // 0x0000 = 0.0 bf16
	REQUIRE(0x3F80 == bfloat16::remainder(bfloat16::bfloat16(10), bfloat16::bfloat16(3)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x4000 == bfloat16::remainder(bfloat16::bfloat16(10), bfloat16::bfloat16(4)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x0000 == bfloat16::remainder(bfloat16::bfloat16(10), bfloat16::bfloat16(5)) ); // 0x0000 = 0.0 bf16
	REQUIRE(0xC000 == bfloat16::remainder(bfloat16::bfloat16(10), bfloat16::bfloat16(6)) ); // 0xC000 = -2.0 bf16
	REQUIRE(0x4040 == bfloat16::remainder(bfloat16::bfloat16(10), bfloat16::bfloat16(7)) ); // 0x4040 = 3.0 bf16
	REQUIRE(0x4000 == bfloat16::remainder(bfloat16::bfloat16(10), bfloat16::bfloat16(8)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x3F80 == bfloat16::remainder(bfloat16::bfloat16(10), bfloat16::bfloat16(9)) ); // 0x3F80 = 1.0 bf16


	REQUIRE(0x0000 == bfloat16::remainder(bfloat16::bfloat16(10), bfloat16::bfloat16(2.5)) ); // 0x0000 = 0.0 bf16
	REQUIRE(0xBF00 == bfloat16::remainder(bfloat16::bfloat16(10), bfloat16::bfloat16(3.5)) ); // 0xBF00 = -0.5 bf16
	REQUIRE(0x3F80 == bfloat16::remainder(bfloat16::bfloat16(10), bfloat16::bfloat16(4.5)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0xBF80 == bfloat16::remainder(bfloat16::bfloat16(10), bfloat16::bfloat16(5.5)) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0xC040 == bfloat16::remainder(bfloat16::bfloat16(10), bfloat16::bfloat16(6.5)) ); // 0xC040 = -2.0 bf16
	REQUIRE(0x4020 == bfloat16::remainder(bfloat16::bfloat16(10), bfloat16::bfloat16(7.5)) ); // 0x4020 = 2.5 bf16
	REQUIRE(0x3FC0 == bfloat16::remainder(bfloat16::bfloat16(10), bfloat16::bfloat16(8.5)) ); // 0x3FC0 = 1.5 bf16
	REQUIRE(0x3F00 == bfloat16::remainder(bfloat16::bfloat16(10), bfloat16::bfloat16(9.5)) ); // 0x3F00 = 0.5 bf16


	//negative
	REQUIRE(0x8000 == bfloat16::remainder(bfloat16::bfloat16(-10), bfloat16::bfloat16(2)) ); // 0x8000 = -0.0 bf16
	REQUIRE(0xBF80 == bfloat16::remainder(bfloat16::bfloat16(-10), bfloat16::bfloat16(3)) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0xC000 == bfloat16::remainder(bfloat16::bfloat16(-10), bfloat16::bfloat16(4)) ); // 0xC000 = -2.0 bf16
	REQUIRE(0x8000 == bfloat16::remainder(bfloat16::bfloat16(-10), bfloat16::bfloat16(5)) ); // 0x8000 = -0.0 bf16
	REQUIRE(0x4000 == bfloat16::remainder(bfloat16::bfloat16(-10), bfloat16::bfloat16(6)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0xC040 == bfloat16::remainder(bfloat16::bfloat16(-10), bfloat16::bfloat16(7)) ); // 0xC040 = -3.0 bf16
	REQUIRE(0xC000 == bfloat16::remainder(bfloat16::bfloat16(-10), bfloat16::bfloat16(8)) ); // 0xC000 = -2.0 bf16
	REQUIRE(0xBF80 == bfloat16::remainder(bfloat16::bfloat16(-10), bfloat16::bfloat16(9)) ); // 0xBF80 = -1.0 bf16


	REQUIRE(0x8000 == bfloat16::remainder(bfloat16::bfloat16(-10), bfloat16::bfloat16(2.5)) ); // 0x8000 = -0.0 bf16
	REQUIRE(0x3F00 == bfloat16::remainder(bfloat16::bfloat16(-10), bfloat16::bfloat16(3.5)) ); // 0x3F00 = 0.5 bf16
	REQUIRE(0xBF80 == bfloat16::remainder(bfloat16::bfloat16(-10), bfloat16::bfloat16(4.5)) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0x3F80 == bfloat16::remainder(bfloat16::bfloat16(-10), bfloat16::bfloat16(5.5)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x4040 == bfloat16::remainder(bfloat16::bfloat16(-10), bfloat16::bfloat16(6.5)) ); // 0x4040 = 3.0 bf16
	REQUIRE(0xC020 == bfloat16::remainder(bfloat16::bfloat16(-10), bfloat16::bfloat16(7.5)) ); // 0xC020 = -2.5 bf16
	REQUIRE(0xBFC0 == bfloat16::remainder(bfloat16::bfloat16(-10), bfloat16::bfloat16(8.5)) ); // 0xBFC0 = -1.5 bf16
	REQUIRE(0xBF00 == bfloat16::remainder(bfloat16::bfloat16(-10), bfloat16::bfloat16(9.5)) ); // 0xBF00 = -0.5 bf16

}



TEST_CASE("bfloat16_remquo", "[floa16]"){

	//works, but got illegal operand error, when i try to get the *quo value from the funtion return
	//I tested it without this part (quo) and it worked completely fine.

	int * quo;
	//positive
	REQUIRE(0x0000 == bfloat16::remquo(bfloat16::bfloat16(10), bfloat16::bfloat16(2), quo) ); // 0x0000 = 0.0 bf16
	REQUIRE(0x3F80 == bfloat16::remquo(bfloat16::bfloat16(10), bfloat16::bfloat16(3), quo) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x4000 == bfloat16::remquo(bfloat16::bfloat16(10), bfloat16::bfloat16(4), quo) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x0000 == bfloat16::remquo(bfloat16::bfloat16(10), bfloat16::bfloat16(5), quo) ); // 0x0000 = 0.0 bf16
	REQUIRE(0xC000 == bfloat16::remquo(bfloat16::bfloat16(10), bfloat16::bfloat16(6), quo) ); // 0xC000 = -2.0 bf16
	REQUIRE(0x4040 == bfloat16::remquo(bfloat16::bfloat16(10), bfloat16::bfloat16(7), quo) ); // 0x4040 = 3.0 bf16
	REQUIRE(0x4000 == bfloat16::remquo(bfloat16::bfloat16(10), bfloat16::bfloat16(8), quo) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x3F80 == bfloat16::remquo(bfloat16::bfloat16(10), bfloat16::bfloat16(9), quo) ); // 0x3F80 = 1.0 bf16


	REQUIRE(0x0000 == bfloat16::remquo(bfloat16::bfloat16(10), bfloat16::bfloat16(2.5), quo) ); // 0x0000 = 0.0 bf16
	REQUIRE(0xBF00 == bfloat16::remquo(bfloat16::bfloat16(10), bfloat16::bfloat16(3.5), quo) ); // 0xBF00 = -0.5 bf16
	REQUIRE(0x3F80 == bfloat16::remquo(bfloat16::bfloat16(10), bfloat16::bfloat16(4.5), quo) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0xBF80 == bfloat16::remquo(bfloat16::bfloat16(10), bfloat16::bfloat16(5.5), quo) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0xC040 == bfloat16::remquo(bfloat16::bfloat16(10), bfloat16::bfloat16(6.5), quo) ); // 0xC040 = -3.0 bf16
	REQUIRE(0x4020 == bfloat16::remquo(bfloat16::bfloat16(10), bfloat16::bfloat16(7.5), quo) ); // 0x4020 = 2.5 bf16
	REQUIRE(0x3FC0 == bfloat16::remquo(bfloat16::bfloat16(10), bfloat16::bfloat16(8.5), quo) ); // 0x3FC0 = 1.5 bf16
	REQUIRE(0x3F00 == bfloat16::remquo(bfloat16::bfloat16(10), bfloat16::bfloat16(9.5), quo) ); // 0x3F00 = 0.5 bf16


	//negative
	REQUIRE(0x8000 == bfloat16::remquo(bfloat16::bfloat16(-10), bfloat16::bfloat16(2), quo) ); // 0x8000 = -0.0 bf16
	REQUIRE(0xBF80 == bfloat16::remquo(bfloat16::bfloat16(-10), bfloat16::bfloat16(3), quo) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0xC000 == bfloat16::remquo(bfloat16::bfloat16(-10), bfloat16::bfloat16(4), quo) ); // 0xC000 = -2.0 bf16
	REQUIRE(0x8000 == bfloat16::remquo(bfloat16::bfloat16(-10), bfloat16::bfloat16(5), quo) ); // 0x8000 = -0.0 bf16
	REQUIRE(0x4000 == bfloat16::remquo(bfloat16::bfloat16(-10), bfloat16::bfloat16(6), quo) ); // 0x4000 = 2.0 bf16
	REQUIRE(0xC040 == bfloat16::remquo(bfloat16::bfloat16(-10), bfloat16::bfloat16(7), quo) ); // 0xC040 = -3.0 bf16
	REQUIRE(0xC000 == bfloat16::remquo(bfloat16::bfloat16(-10), bfloat16::bfloat16(8), quo) ); // 0xC000 = -4.0 bf16
	REQUIRE(0xBF80 == bfloat16::remquo(bfloat16::bfloat16(-10), bfloat16::bfloat16(9), quo) ); // 0xBF80 = -1.0 bf16


	REQUIRE(0x8000 == bfloat16::remquo(bfloat16::bfloat16(-10), bfloat16::bfloat16(2.5), quo) ); // 0x8000 = -0.0 bf16
	REQUIRE(0x3F00 == bfloat16::remquo(bfloat16::bfloat16(-10), bfloat16::bfloat16(3.5), quo) ); // 0x3F00 = 0.5 bf16
	REQUIRE(0xBF80 == bfloat16::remquo(bfloat16::bfloat16(-10), bfloat16::bfloat16(4.5), quo) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0x3F80 == bfloat16::remquo(bfloat16::bfloat16(-10), bfloat16::bfloat16(5.5), quo) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x4040 == bfloat16::remquo(bfloat16::bfloat16(-10), bfloat16::bfloat16(6.5), quo) ); // 0x4040 = 3.0 bf16
	REQUIRE(0xC020 == bfloat16::remquo(bfloat16::bfloat16(-10), bfloat16::bfloat16(7.5), quo) ); // 0xC020 = -2.5 bf16
	REQUIRE(0xBFC0 == bfloat16::remquo(bfloat16::bfloat16(-10), bfloat16::bfloat16(8.5), quo) ); // 0xBFC0 = -1.5 bf16
	REQUIRE(0xBF00 == bfloat16::remquo(bfloat16::bfloat16(-10), bfloat16::bfloat16(9.5), quo) ); // 0xBF00 = -0.5 bf16

}



TEST_CASE("bfloat16_fma", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(NAN == bfloat16::fma(bfloat16::bfloat16(0), std::numeric_limits<bfloat16::bfloat16>::infinity(), bfloat16::bfloat16(2)) );
	REQUIRE(NAN == bfloat16::fma(std::numeric_limits<bfloat16::bfloat16>::infinity(), bfloat16::bfloat16(0), bfloat16::bfloat16(2)) );

	REQUIRE(NAN == bfloat16::fma(bfloat16::bfloat16(0), std::numeric_limits<bfloat16::bfloat16>::infinity(), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	REQUIRE(NAN == bfloat16::fma(std::numeric_limits<bfloat16::bfloat16>::infinity(), bfloat16::bfloat16(0), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );

	REQUIRE(NAN == bfloat16::fma(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), bfloat16::bfloat16(2), bfloat16::bfloat16(2)) );
	REQUIRE(NAN == bfloat16::fma(bfloat16::bfloat16(2), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), bfloat16::bfloat16(2)) );

	REQUIRE(NAN == bfloat16::fma(bfloat16::bfloat16(2), bfloat16::bfloat16(2), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	REQUIRE(NAN == bfloat16::fma(bfloat16::bfloat16(2), bfloat16::bfloat16(2), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive 3x negative 0x
	REQUIRE(0x4250 == bfloat16::fma(bfloat16::bfloat16(10), bfloat16::bfloat16(5), bfloat16::bfloat16(2)) ); // 0x4250 = 52.0 bf16
	REQUIRE(0x41C8 == bfloat16::fma(bfloat16::bfloat16(10), bfloat16::bfloat16(2), bfloat16::bfloat16(5)) ); // 0x41C8 = 25.0 bf16
	REQUIRE(0x4250 == bfloat16::fma(bfloat16::bfloat16(5), bfloat16::bfloat16(10), bfloat16::bfloat16(2)) ); // 0x4250 = 52.0 bf16
	REQUIRE(0x41A0 == bfloat16::fma(bfloat16::bfloat16(5), bfloat16::bfloat16(2), bfloat16::bfloat16(10)) ); // 0x41A0 = 20.0 bf16
	REQUIRE(0x41C8 == bfloat16::fma(bfloat16::bfloat16(2), bfloat16::bfloat16(10), bfloat16::bfloat16(5)) ); // 0x41C8 = 25.0 bf16
	REQUIRE(0x41A0 == bfloat16::fma(bfloat16::bfloat16(2), bfloat16::bfloat16(5), bfloat16::bfloat16(10)) ); // 0x41A0 = 20.0 bf16


	//positive 2x negative 1x
	REQUIRE(0x4240 == bfloat16::fma(bfloat16::bfloat16(10), bfloat16::bfloat16(5), bfloat16::bfloat16(-2)) ); // 0x4240 = 48.0 bf16
	REQUIRE(0x4170 == bfloat16::fma(bfloat16::bfloat16(10), bfloat16::bfloat16(2), bfloat16::bfloat16(-5)) ); // 0x4170 = 15.0 bf16
	REQUIRE(0x4240 == bfloat16::fma(bfloat16::bfloat16(5), bfloat16::bfloat16(10), bfloat16::bfloat16(-2)) ); // 0x4240 = 48.0 bf16
	REQUIRE(0x0 == bfloat16::fma(bfloat16::bfloat16(5), bfloat16::bfloat16(2), bfloat16::bfloat16(-10)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x4170 == bfloat16::fma(bfloat16::bfloat16(2), bfloat16::bfloat16(10), bfloat16::bfloat16(-5)) ); // 0x4170 = 15.0 bf16
	REQUIRE(0x0 == bfloat16::fma(bfloat16::bfloat16(2), bfloat16::bfloat16(5), bfloat16::bfloat16(-10)) ); // 0x0 = 0.0 bf16


	//positive 2x negative 1x
	REQUIRE(0xC240 == bfloat16::fma(bfloat16::bfloat16(10), bfloat16::bfloat16(-5), bfloat16::bfloat16(2)) ); // 0xC240 = -48.0 bf16
	REQUIRE(0xC170 == bfloat16::fma(bfloat16::bfloat16(10), bfloat16::bfloat16(-2), bfloat16::bfloat16(5)) ); // 0xC170 = -15.0 bf16
	REQUIRE(0xC240 == bfloat16::fma(bfloat16::bfloat16(5), bfloat16::bfloat16(-10), bfloat16::bfloat16(2)) ); // 0xC240 = -48.0 bf16
	REQUIRE(0x0 == bfloat16::fma(bfloat16::bfloat16(5), bfloat16::bfloat16(-2), bfloat16::bfloat16(10)) ); // 0x0 = 0.0 bf16
	REQUIRE(0xC170 == bfloat16::fma(bfloat16::bfloat16(2), bfloat16::bfloat16(-10), bfloat16::bfloat16(5)) ); // 0xC170 = -15.0 bf16
	REQUIRE(0x0 == bfloat16::fma(bfloat16::bfloat16(2), bfloat16::bfloat16(-5), bfloat16::bfloat16(10)) ); // 0x0 = 0.0 bf16


	//positive 2x negative 1x
	REQUIRE(0xC240 == bfloat16::fma(bfloat16::bfloat16(-10), bfloat16::bfloat16(5), bfloat16::bfloat16(2)) ); // 0xC240 = -48.0 bf16
	REQUIRE(0xC170 == bfloat16::fma(bfloat16::bfloat16(-10), bfloat16::bfloat16(2), bfloat16::bfloat16(5)) ); // 0xC170 = -15.0 bf16
	REQUIRE(0xC240 == bfloat16::fma(bfloat16::bfloat16(-5), bfloat16::bfloat16(10), bfloat16::bfloat16(2)) ); // 0xC240 = -48.0 bf16
	REQUIRE(0x0 == bfloat16::fma(bfloat16::bfloat16(-5), bfloat16::bfloat16(2), bfloat16::bfloat16(10)) ); // 0x0 = 0.0 bf16
	REQUIRE(0xC170 == bfloat16::fma(bfloat16::bfloat16(-2), bfloat16::bfloat16(10), bfloat16::bfloat16(5)) ); // 0xC170 = -15.0 bf16
	REQUIRE(0x0 == bfloat16::fma(bfloat16::bfloat16(-2), bfloat16::bfloat16(5), bfloat16::bfloat16(10)) ); // 0x0 = 0.0 bf16


	//positive 1x negative x2
	REQUIRE(0xC250 == bfloat16::fma(bfloat16::bfloat16(10), bfloat16::bfloat16(-5), bfloat16::bfloat16(-2)) ); // 0xC250 = -52.0 bf16
	REQUIRE(0xC1C8 == bfloat16::fma(bfloat16::bfloat16(10), bfloat16::bfloat16(-2), bfloat16::bfloat16(-5)) ); // 0xC1C8 = -25.0 bf16
	REQUIRE(0xC250 == bfloat16::fma(bfloat16::bfloat16(5), bfloat16::bfloat16(-10), bfloat16::bfloat16(-2)) ); // 0xC250 = -52.0 bf16
	REQUIRE(0xC1A0 == bfloat16::fma(bfloat16::bfloat16(5), bfloat16::bfloat16(-2), bfloat16::bfloat16(-10)) ); // 0xC1A0 = -20.0 bf16
	REQUIRE(0xC1C8 == bfloat16::fma(bfloat16::bfloat16(2), bfloat16::bfloat16(-10), bfloat16::bfloat16(-5)) ); // 0xC1C8 = -25.0 bf16
	REQUIRE(0xC1A0 == bfloat16::fma(bfloat16::bfloat16(2), bfloat16::bfloat16(-5), bfloat16::bfloat16(-10)) ); // 0xC1A0 = -20.0 bf16


	//positive 1x negative x2
	REQUIRE(0x4250 == bfloat16::fma(bfloat16::bfloat16(-10), bfloat16::bfloat16(-5), bfloat16::bfloat16(2)) ); // 0x4250 = 52.0 bf16
	REQUIRE(0x41C8 == bfloat16::fma(bfloat16::bfloat16(-10), bfloat16::bfloat16(-2), bfloat16::bfloat16(5)) ); // 0x41C8 = 25.0 bf16
	REQUIRE(0x4250 == bfloat16::fma(bfloat16::bfloat16(-5), bfloat16::bfloat16(-10), bfloat16::bfloat16(2)) ); // 0x4250 = 52.0 bf16
	REQUIRE(0x41A0 == bfloat16::fma(bfloat16::bfloat16(-5), bfloat16::bfloat16(-2), bfloat16::bfloat16(10)) ); // 0x41A0 = 20.0 bf16
	REQUIRE(0x41C8 == bfloat16::fma(bfloat16::bfloat16(-2), bfloat16::bfloat16(-10), bfloat16::bfloat16(5)) ); // 0x41C8 = 25.0 bf16
	REQUIRE(0x41A0 == bfloat16::fma(bfloat16::bfloat16(-2), bfloat16::bfloat16(-5), bfloat16::bfloat16(10)) ); // 0x41A0 = 20.0 bf16


	//positive 0x negative x3
	REQUIRE(0x4240 == bfloat16::fma(bfloat16::bfloat16(-10), bfloat16::bfloat16(-5), bfloat16::bfloat16(-2)) ); // 0x4240 = 48.0 bf16
	REQUIRE(0x4170 == bfloat16::fma(bfloat16::bfloat16(-10), bfloat16::bfloat16(-2), bfloat16::bfloat16(-5)) ); // 0x4170 = 15.0 bf16
	REQUIRE(0x4240 == bfloat16::fma(bfloat16::bfloat16(-5), bfloat16::bfloat16(-10), bfloat16::bfloat16(-2)) ); // 0x4240 = 48.0 bf16
	REQUIRE(0x0 == bfloat16::fma(bfloat16::bfloat16(-5), bfloat16::bfloat16(-2), bfloat16::bfloat16(-10)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x4170 == bfloat16::fma(bfloat16::bfloat16(-2), bfloat16::bfloat16(-10), bfloat16::bfloat16(-5)) ); // 0x4170 = 15.0 bf16
	REQUIRE(0x0 == bfloat16::fma(bfloat16::bfloat16(-2), bfloat16::bfloat16(-5), bfloat16::bfloat16(-10)) ); // 0x0 = 0.0 bf16

}



TEST_CASE("bfloat16_fmax", "[float16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(0x4000 == bfloat16::fmax(bfloat16::bfloat16(2), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	REQUIRE(0x4000 == bfloat16::fmax(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), bfloat16::bfloat16(2)) );

	REQUIRE(NAN == bfloat16::fmax(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//numeric values
	REQUIRE(0x3F00 == bfloat16::fmax(bfloat16::bfloat16(0.5), bfloat16::bfloat16(0.1)) ); // 0x3F00 = 0.5 bf16
	REQUIRE(0x3DCC == bfloat16::fmax(bfloat16::bfloat16(-0.5), bfloat16::bfloat16(0.1)) ); // 0x3DCC = 0.1 bf16
	REQUIRE(0x3F00 == bfloat16::fmax(bfloat16::bfloat16(0.5), bfloat16::bfloat16(-0.1)) ); // 0x3F00 = 0.5 bf16
	REQUIRE(0xBDCC == bfloat16::fmax(bfloat16::bfloat16(-0.5), bfloat16::bfloat16(-0.1)) ); // 0xBDCC = -0.1 bf16
	
	REQUIRE(0x4120 == bfloat16::fmax(bfloat16::bfloat16(10), bfloat16::bfloat16(5)) ); // 0x4120 = 10 bf16
	REQUIRE(0x40A0 == bfloat16::fmax(bfloat16::bfloat16(-10), bfloat16::bfloat16(5)) ); // 0x40A0 = 5 bf16
	REQUIRE(0x4120 == bfloat16::fmax(bfloat16::bfloat16(10), bfloat16::bfloat16(-5)) ); // 0x4120 = 10 bf16
	REQUIRE(0xC0A0 == bfloat16::fmax(bfloat16::bfloat16(-10), bfloat16::bfloat16(-5)) ); // 0xC0A0 = -5 bf16

	REQUIRE(0x4128 == bfloat16::fmax(bfloat16::bfloat16(10.5), bfloat16::bfloat16(5.5)) ); // 0x4128 = 10.5 bf16
	REQUIRE(0x40B0 == bfloat16::fmax(bfloat16::bfloat16(-10.5), bfloat16::bfloat16(5.5)) ); // 0x40B0 = 5.5 bf16
	REQUIRE(0x4128 == bfloat16::fmax(bfloat16::bfloat16(10.5), bfloat16::bfloat16(-5.5)) ); // 0x4128 = 10.5 bf16
	REQUIRE(0xC0B0 == bfloat16::fmax(bfloat16::bfloat16(-10.5), bfloat16::bfloat16(-5.5)) ); // 0xC0B0 = -5.5 bf16

}



TEST_CASE("bfloat16_fmin", "[float16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(0x4000 == bfloat16::fmin(bfloat16::bfloat16(2), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	REQUIRE(0x4000 == bfloat16::fmin(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), bfloat16::bfloat16(2)) );

	REQUIRE(NAN == bfloat16::fmin(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//numeric values
	REQUIRE(0x3DCC == bfloat16::fmin(bfloat16::bfloat16(0.5), bfloat16::bfloat16(0.1)) ); // 0x3DCC = 0.1 bf16
	REQUIRE(0xBF00 == bfloat16::fmin(bfloat16::bfloat16(-0.5), bfloat16::bfloat16(0.1)) ); // 0xBF00 = -0.5 bf16
	REQUIRE(0xBDCC == bfloat16::fmin(bfloat16::bfloat16(0.5), bfloat16::bfloat16(-0.1)) ); // 0xBDCC = -0.1 bf16
	REQUIRE(0xBF00 == bfloat16::fmin(bfloat16::bfloat16(-0.5), bfloat16::bfloat16(-0.1)) ); // 0xBF00 = -0.5 bf16

	REQUIRE(0x40A0 == bfloat16::fmin(bfloat16::bfloat16(10), bfloat16::bfloat16(5)) ); // 0x40A0 = 5.0 bf16
	REQUIRE(0xC120 == bfloat16::fmin(bfloat16::bfloat16(-10), bfloat16::bfloat16(5)) ); // 0xC120 = -10.0 bf16
	REQUIRE(0xC0A0 == bfloat16::fmin(bfloat16::bfloat16(10), bfloat16::bfloat16(-5)) ); // 0xC0A0 = -5.0 bf16
	REQUIRE(0xC120 == bfloat16::fmin(bfloat16::bfloat16(-10), bfloat16::bfloat16(-5)) ); // 0xC120 = -10.0 bf16

	REQUIRE(0x40B0 == bfloat16::fmin(bfloat16::bfloat16(10.5), bfloat16::bfloat16(5.5)) ); // 0x40B0 = 5.5 bf16
	REQUIRE(0xC128 == bfloat16::fmin(bfloat16::bfloat16(-10.5), bfloat16::bfloat16(5.5)) ); // 0xC128 = -10.5 bf16
	REQUIRE(0xC0B0 == bfloat16::fmin(bfloat16::bfloat16(10.5), bfloat16::bfloat16(-5.5)) ); // 0xC0B0 = -5.5 bf16
	REQUIRE(0xC128 == bfloat16::fmin(bfloat16::bfloat16(-10.5), bfloat16::bfloat16(-5.5)) ); // 0xC128 = -10.5 bf16

}



TEST_CASE("bfloat16_fdim", "[float16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(NAN == bfloat16::fdim(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), bfloat16::bfloat16()));
	REQUIRE(NAN == bfloat16::fdim(bfloat16::bfloat16(), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()));


	//numeric values
	REQUIRE(0x40A0 == bfloat16::fdim(bfloat16::bfloat16(10), bfloat16::bfloat16(5)) ); // 0x40A0 = 5.0 bf16
	REQUIRE(0x0 == bfloat16::fdim(bfloat16::bfloat16(-10), bfloat16::bfloat16(5)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x4170 == bfloat16::fdim(bfloat16::bfloat16(10), bfloat16::bfloat16(-5)) ); // 0x4170 = 15.0 bf16
	REQUIRE(0x0 == bfloat16::fdim(bfloat16::bfloat16(-10), bfloat16::bfloat16(-5)) ); // 0x0 = 0.0 bf16

	REQUIRE(0x40A0 == bfloat16::fdim(bfloat16::bfloat16(10.5), bfloat16::bfloat16(5.5)) ); // 0x40A0 = 5.0 bf16
	REQUIRE(0x0 == bfloat16::fdim(bfloat16::bfloat16(-10.5), bfloat16::bfloat16(5.5)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x4180 == bfloat16::fdim(bfloat16::bfloat16(10.5), bfloat16::bfloat16(-5.5)) ); // 0x4180 = 16.0 bf16
	REQUIRE(0x0 == bfloat16::fdim(bfloat16::bfloat16(-10.5), bfloat16::bfloat16(-5.5)) ); // 0x0 = 0.0 bf16

}



TEST_CASE("bfloat16_nanh", "[float16]"){

	//works
	
	REQUIRE(NAN == bfloat16::nanh("h") );

}



TEST_CASE("bfloat16_exp", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(0x3F80 == bfloat16::exp(bfloat16::bfloat16(-0)) );
	REQUIRE(0x0 == bfloat16::exp(-std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x7F80 == bfloat16::exp(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::exp(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive
	REQUIRE(0x3F80 == bfloat16::exp(bfloat16::bfloat16(0)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x402D == bfloat16::exp(bfloat16::bfloat16(1)) ); // 0x402D = 2.703125 bf16
	REQUIRE(0x40EC == bfloat16::exp(bfloat16::bfloat16(2)) ); // 0x40EC = 7.375 bf16
	REQUIRE(0x41A0 == bfloat16::exp(bfloat16::bfloat16(3)) ); // 0x41A0 = 20.0 bf16
	REQUIRE(0x425A == bfloat16::exp(bfloat16::bfloat16(4)) ); // 0x425A = 54.5 bf16
	REQUIRE(0x4314 == bfloat16::exp(bfloat16::bfloat16(5)) ); // 0x4314 = 148.0 bf16
	REQUIRE(0x43C9 == bfloat16::exp(bfloat16::bfloat16(6)) ); // 0x43C9 = 402.0 bf16
	REQUIRE(0x4489 == bfloat16::exp(bfloat16::bfloat16(7)) ); // 0x4489 = 1096.0 bf16
	REQUIRE(0x453A == bfloat16::exp(bfloat16::bfloat16(8)) ); // 0x453A = 2976.0 bf16
	REQUIRE(0x45FD == bfloat16::exp(bfloat16::bfloat16(9)) ); // 0x45FD = 8096.0 bf16
	REQUIRE(0x46AC == bfloat16::exp(bfloat16::bfloat16(10)) ); // 0x46AC = 22016.0 bf16

	REQUIRE(0x4101 == bfloat16::exp(bfloat16::bfloat16(2.1)) ); // 0x4101 = 8.0625 bf16
	REQUIRE(0x410E == bfloat16::exp(bfloat16::bfloat16(2.2)) ); // 0x410E = 8.875 bf16
	REQUIRE(0x411F == bfloat16::exp(bfloat16::bfloat16(2.3)) ); // 0x411F = 9.9375 bf16
	REQUIRE(0x412E == bfloat16::exp(bfloat16::bfloat16(2.4)) ); // 0x412E = 10.875 bf16
	REQUIRE(0x4142 == bfloat16::exp(bfloat16::bfloat16(2.5)) ); // 0x4142 = 12.125 bf16
	REQUIRE(0x4156 == bfloat16::exp(bfloat16::bfloat16(2.6)) ); // 0x4156 = 13.375 bf16
	REQUIRE(0x416B == bfloat16::exp(bfloat16::bfloat16(2.7)) ); // 0x416B = 14.6875 bf16
	REQUIRE(0x4183 == bfloat16::exp(bfloat16::bfloat16(2.8)) ); // 0x4183 = 16.375 bf16
	REQUIRE(0x4190 == bfloat16::exp(bfloat16::bfloat16(2.9)) ); // 0x4190 = 18.0 bf16


	//negative
	REQUIRE(0x3EBC == bfloat16::exp(bfloat16::bfloat16(-1)) ); // 0x3EBC = 0.3671875 bf16
	REQUIRE(0x3E0A == bfloat16::exp(bfloat16::bfloat16(-2)) ); // 0x3E0A = 0.134765625 bf16
	REQUIRE(0x3D4B == bfloat16::exp(bfloat16::bfloat16(-3)) ); // 0x3D4B = 0.049560546875 bf16
	REQUIRE(0x3C96 == bfloat16::exp(bfloat16::bfloat16(-4)) ); // 0x3C96 = 0.018310546875 bf16
	REQUIRE(0x3BDC == bfloat16::exp(bfloat16::bfloat16(-5)) ); // 0x3BDC = 0.0067138671875 bf16
	REQUIRE(0x3B22 == bfloat16::exp(bfloat16::bfloat16(-6)) ); // 0x3B22 = 0.002471923828125 bf16
	REQUIRE(0x3A6F == bfloat16::exp(bfloat16::bfloat16(-7)) ); // 0x3A6F = 0.000911712646484375 bf16
	REQUIRE(0x39AF == bfloat16::exp(bfloat16::bfloat16(-8)) ); // 0x39AF = 0.0003337860107421875 bf16
	REQUIRE(0x3901 == bfloat16::exp(bfloat16::bfloat16(-9)) ); // 0x3901 = 0.00012302398681640625 bf16
	REQUIRE(0x02FA == bfloat16::exp(bfloat16::bfloat16(-10)) ); // 0x02FA = 3.67341984632e-37 bf16
	
	REQUIRE(0x3DFC == bfloat16::exp(bfloat16::bfloat16(-2.1)) ); // 0x3DFC = 0.123046875 bf16
	REQUIRE(0x3DE5 == bfloat16::exp(bfloat16::bfloat16(-2.2)) ); // 0x3DE5 = 0.11181640625 bf16
	REQUIRE(0x3DCD == bfloat16::exp(bfloat16::bfloat16(-2.3)) ); // 0x3DCD = 0.10009765625 bf16
	REQUIRE(0x3DBB == bfloat16::exp(bfloat16::bfloat16(-2.4)) ); // 0x3DBB = 0.09130859375 bf16
	REQUIRE(0x3DA8 == bfloat16::exp(bfloat16::bfloat16(-2.5)) ); // 0x3DA8 = 0.08203125 bf16
	REQUIRE(0x3D99 == bfloat16::exp(bfloat16::bfloat16(-2.6)) ); // 0x3D99 = 0.07470703125 bf16
	REQUIRE(0x3D8B == bfloat16::exp(bfloat16::bfloat16(-2.7)) ); // 0x3D8B = 0.06787109375 bf16
	REQUIRE(0x3D79 == bfloat16::exp(bfloat16::bfloat16(-2.8)) ); // 0x3D79 = 0.060791015625 bf16
	REQUIRE(0x3D63 == bfloat16::exp(bfloat16::bfloat16(-2.9)) ); // 0x3D63 = 0.055419921875 bf16

}



TEST_CASE("bfloat16_exp2", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(0x3F80 == bfloat16::exp2(bfloat16::bfloat16(-0)) );
	REQUIRE(0x0 == bfloat16::exp2(-std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x7F80 == bfloat16::exp2(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::exp2(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive
	REQUIRE(0x3F80 == bfloat16::exp2(bfloat16::bfloat16(0)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x4000 == bfloat16::exp2(bfloat16::bfloat16(1)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x4080 == bfloat16::exp2(bfloat16::bfloat16(2)) ); // 0x4080 = 4.0 bf16
	REQUIRE(0x4100 == bfloat16::exp2(bfloat16::bfloat16(3)) ); // 0x4100 = 8.0 bf16
	REQUIRE(0x4180 == bfloat16::exp2(bfloat16::bfloat16(4)) ); // 0x4180 = 16.0 bf16
	REQUIRE(0x4200 == bfloat16::exp2(bfloat16::bfloat16(5)) ); // 0x4200 = 32.0 bf16
	REQUIRE(0x4280 == bfloat16::exp2(bfloat16::bfloat16(6)) ); // 0x4280 = 64.0 bf16
	REQUIRE(0x4300 == bfloat16::exp2(bfloat16::bfloat16(7)) ); // 0x4300 = 128.0 bf16
	REQUIRE(0x4380 == bfloat16::exp2(bfloat16::bfloat16(8)) ); // 0x4380 = 256.0 bf16
	REQUIRE(0x4400 == bfloat16::exp2(bfloat16::bfloat16(9)) ); // 0x4400 = 512.0 bf16
	REQUIRE(0x4480 == bfloat16::exp2(bfloat16::bfloat16(10)) ); // 0x4480 = 1024.0 bf16

	REQUIRE(0x4088 == bfloat16::exp2(bfloat16::bfloat16(2.1)) ); // 0x4088 = 4.25 bf16
	REQUIRE(0x4091 == bfloat16::exp2(bfloat16::bfloat16(2.2)) ); // 0x4091 = 4.53125 bf16
	REQUIRE(0x409D == bfloat16::exp2(bfloat16::bfloat16(2.3)) ); // 0x409D = 4.90625 bf16
	REQUIRE(0x40A7 == bfloat16::exp2(bfloat16::bfloat16(2.4)) ); // 0x40A7 = 5.21875 bf16
	REQUIRE(0x40B5 == bfloat16::exp2(bfloat16::bfloat16(2.5)) ); // 0x40B5 = 5.65625 bf16
	REQUIRE(0x40C1 == bfloat16::exp2(bfloat16::bfloat16(2.6)) ); // 0x40C1 = 6.03125 bf16
	REQUIRE(0x40CE == bfloat16::exp2(bfloat16::bfloat16(2.7)) ); // 0x40CE = 6.4375 bf16
	REQUIRE(0x40DE == bfloat16::exp2(bfloat16::bfloat16(2.8)) ); // 0x40DE = 6.9375 bf16
	REQUIRE(0x40ED == bfloat16::exp2(bfloat16::bfloat16(2.9)) ); // 0x40ED = 7.40625 bf16

	//negative
	REQUIRE(0x3F00 == bfloat16::exp2(bfloat16::bfloat16(-1)) ); // 0x3F00 = 0.5 bf16
	REQUIRE(0x3E80 == bfloat16::exp2(bfloat16::bfloat16(-2)) ); // 0x3E80 = 0.25 bf16
	REQUIRE(0x3E00 == bfloat16::exp2(bfloat16::bfloat16(-3)) ); // 0x3E00 = 0.125 bf16
	REQUIRE(0x3D80 == bfloat16::exp2(bfloat16::bfloat16(-4)) ); // 0x3D80 = 0.0625 bf16
	REQUIRE(0x3D00 == bfloat16::exp2(bfloat16::bfloat16(-5)) ); // 0x3D00 = 0.03125 bf16
	REQUIRE(0x3C80 == bfloat16::exp2(bfloat16::bfloat16(-6)) ); // 0x3C80 = 0.015625 bf16
	REQUIRE(0x3C00 == bfloat16::exp2(bfloat16::bfloat16(-7)) ); // 0x3C00 = 0.0078125 bf16
	REQUIRE(0x3B80 == bfloat16::exp2(bfloat16::bfloat16(-8)) ); // 0x3B80 = 0.00390625 bf16
	REQUIRE(0x3B00 == bfloat16::exp2(bfloat16::bfloat16(-9)) ); // 0x3B00 = 0.001953125 bf16
	REQUIRE(0x3A80 == bfloat16::exp2(bfloat16::bfloat16(-10)) ); // 0x3A80 = 0.0009765625 bf16

	REQUIRE(0x3E6F == bfloat16::exp2(bfloat16::bfloat16(-2.1)) ); // 0x3E6F = 0.2333984375 bf16
	REQUIRE(0x3E60 == bfloat16::exp2(bfloat16::bfloat16(-2.2)) ); // 0x3E60 = 0.21875 bf16
	REQUIRE(0x3E50 == bfloat16::exp2(bfloat16::bfloat16(-2.3)) ); // 0x3E50 = 0.203125 bf16
	REQUIRE(0x3E43 == bfloat16::exp2(bfloat16::bfloat16(-2.4)) ); // 0x3E43 = 0.1904296875 bf16
	REQUIRE(0x3E35 == bfloat16::exp2(bfloat16::bfloat16(-2.5)) ); // 0x3E35 = 0.1767578125 bf16
	REQUIRE(0x3E29 == bfloat16::exp2(bfloat16::bfloat16(-2.6)) ); // 0x3E29 = 0.1650390625 bf16
	REQUIRE(0x3E1E == bfloat16::exp2(bfloat16::bfloat16(-2.7)) ); // 0x3E1E = 0.154296875 bf16
	REQUIRE(0x3E13 == bfloat16::exp2(bfloat16::bfloat16(-2.8)) ); // 0x3E13 = 0.1435546875 bf16
	REQUIRE(0x3E0A == bfloat16::exp2(bfloat16::bfloat16(-2.9)) ); // 0x3E0A = 0.134765625 bf16

}



TEST_CASE("bfloat16_expm1", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling	/ numeric Limits
	REQUIRE(0x0 == bfloat16::expm1(bfloat16::bfloat16(-0)) );
	REQUIRE(0x0 == bfloat16::expm1(bfloat16::bfloat16(0)) );
	REQUIRE(0xBF80 == bfloat16::expm1(-std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x7F80 == bfloat16::expm1(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::expm1(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive
	REQUIRE(0x0 == bfloat16::expm1(bfloat16::bfloat16(0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3FDB == bfloat16::expm1(bfloat16::bfloat16(1)) ); // 0x3FDB = 1.7109375 bf16
	REQUIRE(0x40CC == bfloat16::expm1(bfloat16::bfloat16(2)) ); // 0x40CC = 6.375 bf16
	REQUIRE(0x4198 == bfloat16::expm1(bfloat16::bfloat16(3)) ); // 0x4198 = 19.0 bf16
	REQUIRE(0x4256 == bfloat16::expm1(bfloat16::bfloat16(4)) ); // 0x4256 = 53.5 bf16
	REQUIRE(0x4313 == bfloat16::expm1(bfloat16::bfloat16(5)) ); // 0x4313 = 147.0 bf16
	REQUIRE(0x43C9 == bfloat16::expm1(bfloat16::bfloat16(6)) ); // 0x43C9 = 402.0 bf16
	REQUIRE(0x4488 == bfloat16::expm1(bfloat16::bfloat16(7)) ); // 0x4488 = 1088.0 bf16
	REQUIRE(0x453A == bfloat16::expm1(bfloat16::bfloat16(8)) ); // 0x453A = 2976.0 bf16
	REQUIRE(0x45FD == bfloat16::expm1(bfloat16::bfloat16(9)) ); // 0x45FD = 8096.0 bf16
	REQUIRE(0x46AC == bfloat16::expm1(bfloat16::bfloat16(10)) ); // 0x46AC = 22016.0 bf16

	REQUIRE(0x40E3 == bfloat16::expm1(bfloat16::bfloat16(2.1)) ); // 0x40E3 = 7.09375 bf16
	REQUIRE(0x40FD == bfloat16::expm1(bfloat16::bfloat16(2.2)) ); // 0x40FD = 7.90625 bf16
	REQUIRE(0x410F == bfloat16::expm1(bfloat16::bfloat16(2.3)) ); // 0x410F = 8.9375 bf16
	REQUIRE(0x411E == bfloat16::expm1(bfloat16::bfloat16(2.4)) ); // 0x411E = 9.875 bf16
	REQUIRE(0x4132 == bfloat16::expm1(bfloat16::bfloat16(2.5)) ); // 0x4132 = 11.125 bf16
	REQUIRE(0x4146 == bfloat16::expm1(bfloat16::bfloat16(2.6)) ); // 0x4146 = 12.375 bf16
	REQUIRE(0x415B == bfloat16::expm1(bfloat16::bfloat16(2.7)) ); // 0x415B = 13.6875 bf16
	REQUIRE(0x4176 == bfloat16::expm1(bfloat16::bfloat16(2.8)) ); // 0x4176 = 15.375 bf16
	REQUIRE(0x4188 == bfloat16::expm1(bfloat16::bfloat16(2.9)) ); // 0x4188 = 17.0 bf16


	//negative
	REQUIRE(0xBF21 == bfloat16::expm1(bfloat16::bfloat16(-1)) ); // 0xBF21 = -0.62890625 bf16
	REQUIRE(0xBF5D == bfloat16::expm1(bfloat16::bfloat16(-2)) ); // 0xBF5D = -0.86328125 bf16
	REQUIRE(0xBF73 == bfloat16::expm1(bfloat16::bfloat16(-3)) ); // 0xBF73 = -0.94921875 bf16
	REQUIRE(0xBF7B == bfloat16::expm1(bfloat16::bfloat16(-4)) ); // 0xBF7B = -0.98046875 bf16
	REQUIRE(0xBF7E == bfloat16::expm1(bfloat16::bfloat16(-5)) ); // 0xBF7E = -0.9921875 bf16
	REQUIRE(0xBF7F == bfloat16::expm1(bfloat16::bfloat16(-6)) ); // 0xBF7F = -0.99609375 bf16
	REQUIRE(0xBF7F == bfloat16::expm1(bfloat16::bfloat16(-7)) ); // 0xBF7F = -0.99609375 bf16
	REQUIRE(0xBF7F == bfloat16::expm1(bfloat16::bfloat16(-8)) ); // 0xBF7F = -0.99609375 bf16
	REQUIRE(0xBF7F == bfloat16::expm1(bfloat16::bfloat16(-9)) ); // 0xBF7F = -0.99609375 bf16
	REQUIRE(0xBF7F == bfloat16::expm1(bfloat16::bfloat16(-10)) ); // 0xBF7F = -0.99609375 bf16

	REQUIRE(0xBF60 == bfloat16::expm1(bfloat16::bfloat16(-2.1)) ); // 0xBF60 = -0.875 bf16
	REQUIRE(0xBF63 == bfloat16::expm1(bfloat16::bfloat16(-2.2)) ); // 0xBF63 = -0.88671875 bf16
	REQUIRE(0xBF66 == bfloat16::expm1(bfloat16::bfloat16(-2.3)) ); // 0xBF66 = -0.8984375 bf16
	REQUIRE(0xBF68 == bfloat16::expm1(bfloat16::bfloat16(-2.4)) ); // 0xBF68 = -0.90625 bf16
	REQUIRE(0xBF6A == bfloat16::expm1(bfloat16::bfloat16(-2.5)) ); // 0xBF6A = -0.9140625 bf16
	REQUIRE(0xBF6C == bfloat16::expm1(bfloat16::bfloat16(-2.6)) ); // 0xBF6C = -0.921875 bf16
	REQUIRE(0xBF6E == bfloat16::expm1(bfloat16::bfloat16(-2.7)) ); // 0xBF6E = -0.9296875 bf16
	REQUIRE(0xBF70 == bfloat16::expm1(bfloat16::bfloat16(-2.8)) ); // 0xBF70 = -0.9375 bf16
	REQUIRE(0xBF71 == bfloat16::expm1(bfloat16::bfloat16(-2.9)) ); // 0xBF71 = -0.94140625 bf16

}



TEST_CASE("bfloat16_log", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(0x7F80 == bfloat16::log(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::log(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive
	REQUIRE(0xFF80 == bfloat16::log(bfloat16::bfloat16(0)) ); // 0xFF80 = -inf bf16
	REQUIRE(0x0 == bfloat16::log(bfloat16::bfloat16(1)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3F31 == bfloat16::log(bfloat16::bfloat16(2)) ); // 0x3F31 = 0.69140625 bf16
	REQUIRE(0x3F8C == bfloat16::log(bfloat16::bfloat16(3)) ); // 0x3F8C = 1.09375 bf16
	REQUIRE(0x3FB1 == bfloat16::log(bfloat16::bfloat16(4)) ); // 0x3FB1 = 1.3828125 bf16
	REQUIRE(0x3FCE == bfloat16::log(bfloat16::bfloat16(5)) ); // 0x3FCE = 1.609375 bf16
	REQUIRE(0x3FE5 == bfloat16::log(bfloat16::bfloat16(6)) ); // 0x3FE5 = 1.7890625 bf16
	REQUIRE(0x3FF9 == bfloat16::log(bfloat16::bfloat16(7)) ); // 0x3FF9 = 1.9453125 bf16
	REQUIRE(0x4005 == bfloat16::log(bfloat16::bfloat16(8)) ); // 0x4005 = 2.078125 bf16
	REQUIRE(0x400C == bfloat16::log(bfloat16::bfloat16(9)) ); // 0x400C = 2.1875 bf16
	REQUIRE(0x4013 == bfloat16::log(bfloat16::bfloat16(10)) ); // 0x4013 = 2.296875 bf16

	REQUIRE(0x3F3D == bfloat16::log(bfloat16::bfloat16(2.1)) ); // 0x3F3D = 0.73828125 bf16
	REQUIRE(0x3F48 == bfloat16::log(bfloat16::bfloat16(2.2)) ); // 0x3F48 = 0.78125 bf16
	REQUIRE(0x3F54 == bfloat16::log(bfloat16::bfloat16(2.3)) ); // 0x3F54 = 0.828125 bf16
	REQUIRE(0x3F5F == bfloat16::log(bfloat16::bfloat16(2.4)) ); // 0x3F5F = 0.87109375 bf16
	REQUIRE(0x3F6A == bfloat16::log(bfloat16::bfloat16(2.5)) ); // 0x3F6A = 0.9140625 bf16
	REQUIRE(0x3F73 == bfloat16::log(bfloat16::bfloat16(2.6)) ); // 0x3F73 = 0.94921875 bf16
	REQUIRE(0x3F7D == bfloat16::log(bfloat16::bfloat16(2.7)) ); // 0x3F7D = 0.98828125 bf16
	REQUIRE(0x3F83 == bfloat16::log(bfloat16::bfloat16(2.8)) ); // 0x3F83 = 1.0234375 bf16
	REQUIRE(0x3F87 == bfloat16::log(bfloat16::bfloat16(2.9)) ); // 0x3F87 = 1.0546875 bf16


	//negative
	REQUIRE(0xFFC0 == bfloat16::log(bfloat16::bfloat16(-1)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log(bfloat16::bfloat16(-2)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log(bfloat16::bfloat16(-3)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log(bfloat16::bfloat16(-4)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log(bfloat16::bfloat16(-5)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log(bfloat16::bfloat16(-6)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log(bfloat16::bfloat16(-7)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log(bfloat16::bfloat16(-8)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log(bfloat16::bfloat16(-9)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log(bfloat16::bfloat16(-10)) ); // 0xFFC0 = NaN bf16

	REQUIRE(0xFFC0 == bfloat16::log(bfloat16::bfloat16(-2.1)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log(bfloat16::bfloat16(-2.2)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log(bfloat16::bfloat16(-2.3)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log(bfloat16::bfloat16(-2.4)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log(bfloat16::bfloat16(-2.5)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log(bfloat16::bfloat16(-2.6)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log(bfloat16::bfloat16(-2.7)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log(bfloat16::bfloat16(-2.8)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log(bfloat16::bfloat16(-2.9)) ); // 0xFFC0 = NaN bf16

}



TEST_CASE("bfloat16_log10", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(0x7F80 == bfloat16::log10(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::log10(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive
	REQUIRE(0xFF80 == bfloat16::log10(bfloat16::bfloat16(0)) ); // 0xFF80 = -inf bf16
	REQUIRE(0x0 == bfloat16::log10(bfloat16::bfloat16(1)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3E9A == bfloat16::log10(bfloat16::bfloat16(2)) ); // 0x3E9A = 0.30078125 bf16
	REQUIRE(0x3EF4 == bfloat16::log10(bfloat16::bfloat16(3)) ); // 0x3EF4 = 0.4765625 bf16
	REQUIRE(0x3F1A == bfloat16::log10(bfloat16::bfloat16(4)) ); // 0x3F1A = 0.6015625 bf16
	REQUIRE(0x3F32 == bfloat16::log10(bfloat16::bfloat16(5)) ); // 0x3F32 = 0.6953125 bf16
	REQUIRE(0x3F47 == bfloat16::log10(bfloat16::bfloat16(6)) ); // 0x3F47 = 0.77734375 bf16
	REQUIRE(0x3F58 == bfloat16::log10(bfloat16::bfloat16(7)) ); // 0x3F58 = 0.84375 bf16
	REQUIRE(0x3F67 == bfloat16::log10(bfloat16::bfloat16(8)) ); // 0x3F67 = 0.90234375 bf16
	REQUIRE(0x3F74 == bfloat16::log10(bfloat16::bfloat16(9)) ); // 0x3F74 = 0.953125 bf16
	REQUIRE(0x3F80 == bfloat16::log10(bfloat16::bfloat16(10)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x4000 == bfloat16::log10(bfloat16::bfloat16(100)) ); // 0x4000 = 2.0 bf16

	REQUIRE(0x3EA4 == bfloat16::log10(bfloat16::bfloat16(2.1)) ); // 0x3EA4 =  bf16
	REQUIRE(0x3EAE == bfloat16::log10(bfloat16::bfloat16(2.2)) ); // 0x3EAE =  bf16
	REQUIRE(0x3EB8 == bfloat16::log10(bfloat16::bfloat16(2.3)) ); // 0x3EB8 =  bf16
	REQUIRE(0x3EC1 == bfloat16::log10(bfloat16::bfloat16(2.4)) ); // 0x3EC1 =  bf16
	REQUIRE(0x3ECB == bfloat16::log10(bfloat16::bfloat16(2.5)) ); // 0x3ECB =  bf16
	REQUIRE(0x3ED3 == bfloat16::log10(bfloat16::bfloat16(2.6)) ); // 0x3ED3 =  bf16
	REQUIRE(0x3EDB == bfloat16::log10(bfloat16::bfloat16(2.7)) ); // 0x3EDB =  bf16
	REQUIRE(0x3EE4 == bfloat16::log10(bfloat16::bfloat16(2.8)) ); // 0x3EE4 =  bf16
	REQUIRE(0x3EEC == bfloat16::log10(bfloat16::bfloat16(2.9)) ); // 0x3EEC =  bf16


	//negative
	REQUIRE(0x7FC0 == bfloat16::log10(bfloat16::bfloat16(-1)) ); // 0x7FC0 = NaN bf16
	REQUIRE(0x7FC0 == bfloat16::log10(bfloat16::bfloat16(-2)) ); // 0x7FC0 = NaN bf16
	REQUIRE(0x7FC0 == bfloat16::log10(bfloat16::bfloat16(-3)) ); // 0x7FC0 = NaN bf16
	REQUIRE(0x7FC0 == bfloat16::log10(bfloat16::bfloat16(-4)) ); // 0x7FC0 = NaN bf16
	REQUIRE(0x7FC0 == bfloat16::log10(bfloat16::bfloat16(-5)) ); // 0x7FC0 = NaN bf16
	REQUIRE(0x7FC0 == bfloat16::log10(bfloat16::bfloat16(-6)) ); // 0x7FC0 = NaN bf16
	REQUIRE(0x7FC0 == bfloat16::log10(bfloat16::bfloat16(-7)) ); // 0x7FC0 = NaN bf16
	REQUIRE(0x7FC0 == bfloat16::log10(bfloat16::bfloat16(-8)) ); // 0x7FC0 = NaN bf16
	REQUIRE(0x7FC0 == bfloat16::log10(bfloat16::bfloat16(-9)) ); // 0x7FC0 = NaN bf16
	REQUIRE(0x7FC0 == bfloat16::log10(bfloat16::bfloat16(-10)) ); // 0x7FC0 = NaN bf16

	REQUIRE(0x7FC0 == bfloat16::log10(bfloat16::bfloat16(-2.1)) ); // 0x7FC0 = NaN bf16
	REQUIRE(0x7FC0 == bfloat16::log10(bfloat16::bfloat16(-2.2)) ); // 0x7FC0 = NaN bf16
	REQUIRE(0x7FC0 == bfloat16::log10(bfloat16::bfloat16(-2.3)) ); // 0x7FC0 = NaN bf16
	REQUIRE(0x7FC0 == bfloat16::log10(bfloat16::bfloat16(-2.4)) ); // 0x7FC0 = NaN bf16
	REQUIRE(0x7FC0 == bfloat16::log10(bfloat16::bfloat16(-2.5)) ); // 0x7FC0 = NaN bf16
	REQUIRE(0x7FC0 == bfloat16::log10(bfloat16::bfloat16(-2.6)) ); // 0x7FC0 = NaN bf16
	REQUIRE(0x7FC0 == bfloat16::log10(bfloat16::bfloat16(-2.7)) ); // 0x7FC0 = NaN bf16
	REQUIRE(0x7FC0 == bfloat16::log10(bfloat16::bfloat16(-2.8)) ); // 0x7FC0 = NaN bf16
	REQUIRE(0x7FC0 == bfloat16::log10(bfloat16::bfloat16(-2.9)) ); // 0x7FC0 = NaN bf16

}



TEST_CASE("bfloat16_log2", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(0x7F80 == bfloat16::log2(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::log2(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive
	REQUIRE(0xFF80 == bfloat16::log2(bfloat16::bfloat16(0)) ); // 0xFF80 = -inf bf16
	REQUIRE(0x0 == bfloat16::log2(bfloat16::bfloat16(1)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3F80 == bfloat16::log2(bfloat16::bfloat16(2)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x3FCA == bfloat16::log2(bfloat16::bfloat16(3)) ); // 0x3FCA = 1.578125 bf16
	REQUIRE(0x4000 == bfloat16::log2(bfloat16::bfloat16(4)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x4014 == bfloat16::log2(bfloat16::bfloat16(5)) ); // 0x4014 = 2.3125 bf16
	REQUIRE(0x4025 == bfloat16::log2(bfloat16::bfloat16(6)) ); // 0x4025 = 2.578125 bf16
	REQUIRE(0x4033 == bfloat16::log2(bfloat16::bfloat16(7)) ); // 0x4033 = 2.796875 bf16
	REQUIRE(0x4040 == bfloat16::log2(bfloat16::bfloat16(8)) ); // 0x4040 = 3.0 bf16
	REQUIRE(0x404A == bfloat16::log2(bfloat16::bfloat16(9)) ); // 0x404A = 3.15625 bf16
	REQUIRE(0x4054 == bfloat16::log2(bfloat16::bfloat16(10)) ); // 0x4054 = 3.3125 bf16

	REQUIRE(0x3F88 == bfloat16::log2(bfloat16::bfloat16(2.1)) ); // 0x3F88 = 1.0625 bf16
	REQUIRE(0x3F90 == bfloat16::log2(bfloat16::bfloat16(2.2)) ); // 0x3F90 = 1.125 bf16
	REQUIRE(0x3F99 == bfloat16::log2(bfloat16::bfloat16(2.3)) ); // 0x3F99 = 1.1953125 bf16
	REQUIRE(0x3FA0 == bfloat16::log2(bfloat16::bfloat16(2.4)) ); // 0x3FA0 = 1.25 bf16
	REQUIRE(0x3FA9 == bfloat16::log2(bfloat16::bfloat16(2.5)) ); // 0x3FA9 = 1.3203125 bf16
	REQUIRE(0x3FB0 == bfloat16::log2(bfloat16::bfloat16(2.6)) ); // 0x3FB0 = 1.375 bf16
	REQUIRE(0x3FB6 == bfloat16::log2(bfloat16::bfloat16(2.7)) ); // 0x3FB6 = 1.421875 bf16
	REQUIRE(0x3FBD == bfloat16::log2(bfloat16::bfloat16(2.8)) ); // 0x3FBD = 1.4765625 bf16
	REQUIRE(0x3FC4 == bfloat16::log2(bfloat16::bfloat16(2.9)) ); // 0x3FC4 = 1.53125 bf16

	REQUIRE(0x3F80 == bfloat16::log2(bfloat16::bfloat16(2)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x4000 == bfloat16::log2(bfloat16::bfloat16(4)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x4040 == bfloat16::log2(bfloat16::bfloat16(8)) ); // 0x4040 = 3.0 bf16
	REQUIRE(0x4080 == bfloat16::log2(bfloat16::bfloat16(16)) ); // 0x4080 = 4.0 bf16
	REQUIRE(0x40A0 == bfloat16::log2(bfloat16::bfloat16(32)) ); // 0x40A0 = 5.0 bf16
	REQUIRE(0x40C0 == bfloat16::log2(bfloat16::bfloat16(64)) ); // 0x40C0 = 6.0 bf16
	REQUIRE(0x40E0 == bfloat16::log2(bfloat16::bfloat16(128)) ); // 0x40E0 = 7.0 bf16
	REQUIRE(0x4100 == bfloat16::log2(bfloat16::bfloat16(256)) ); // 0x4100 = 8.0 bf16
	REQUIRE(0x4110 == bfloat16::log2(bfloat16::bfloat16(512)) ); // 0x4110 = 9.0 bf16
	REQUIRE(0x4120 == bfloat16::log2(bfloat16::bfloat16(1024)) ); // 0x4120 = 10.0 bf16


	//negative
	REQUIRE(0xFFC0 == bfloat16::log2(bfloat16::bfloat16(-1)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log2(bfloat16::bfloat16(-2)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log2(bfloat16::bfloat16(-3)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log2(bfloat16::bfloat16(-4)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log2(bfloat16::bfloat16(-5)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log2(bfloat16::bfloat16(-6)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log2(bfloat16::bfloat16(-7)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log2(bfloat16::bfloat16(-8)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log2(bfloat16::bfloat16(-9)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log2(bfloat16::bfloat16(-10)) ); // 0xFFC0 = NaN bf16

	REQUIRE(0xFFC0 == bfloat16::log2(bfloat16::bfloat16(-2.1)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log2(bfloat16::bfloat16(-2.2)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log2(bfloat16::bfloat16(-2.3)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log2(bfloat16::bfloat16(-2.4)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log2(bfloat16::bfloat16(-2.5)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log2(bfloat16::bfloat16(-2.6)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log2(bfloat16::bfloat16(-2.7)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log2(bfloat16::bfloat16(-2.8)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log2(bfloat16::bfloat16(-2.9)) ); // 0xFFC0 = NaN bf16

}



TEST_CASE("bfloat16_log1p", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling // numeric Limits
	REQUIRE(0x7F80 == bfloat16::log(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::log(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive
	REQUIRE(0x0 == bfloat16::log1p(bfloat16::bfloat16(0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3F31 == bfloat16::log1p(bfloat16::bfloat16(1)) ); // 0x3F31 = 0.69140625 bf16
	REQUIRE(0x3F8C == bfloat16::log1p(bfloat16::bfloat16(2)) ); // 0x3F8C = 1.09375 bf16
	REQUIRE(0x3FB1 == bfloat16::log1p(bfloat16::bfloat16(3)) ); // 0x3FB1 = 1.3828125 bf16
	REQUIRE(0x3FCE == bfloat16::log1p(bfloat16::bfloat16(4)) ); // 0x3FCE = 1.609375 bf16
	REQUIRE(0x3FE5 == bfloat16::log1p(bfloat16::bfloat16(5)) ); // 0x3FE5 = 1.7890625 bf16
	REQUIRE(0x3FF9 == bfloat16::log1p(bfloat16::bfloat16(6)) ); // 0x3FF9 = 1.9453125 bf16
	REQUIRE(0x4005 == bfloat16::log1p(bfloat16::bfloat16(7)) ); // 0x4005 = 2.078125 bf16
	REQUIRE(0x400C == bfloat16::log1p(bfloat16::bfloat16(8)) ); // 0x400C = 2.1875 bf16
	REQUIRE(0x4013 == bfloat16::log1p(bfloat16::bfloat16(9)) ); // 0x4013 = 2.296875 bf16
	REQUIRE(0x4019 == bfloat16::log1p(bfloat16::bfloat16(10)) ); // 0x4019 = 2.390625 bf16

	REQUIRE(0x3F90 == bfloat16::log1p(bfloat16::bfloat16(2.1)) ); // 0x3F90 = 1.125 bf16
	REQUIRE(0x3F94 == bfloat16::log1p(bfloat16::bfloat16(2.2)) ); // 0x3F94 = 1.15625 bf16
	REQUIRE(0x3F98 == bfloat16::log1p(bfloat16::bfloat16(2.3)) ); // 0x3F98 = 1.1875 bf16
	REQUIRE(0x3F9C == bfloat16::log1p(bfloat16::bfloat16(2.4)) ); // 0x3F9C = 1.21875 bf16
	REQUIRE(0x3FA0 == bfloat16::log1p(bfloat16::bfloat16(2.5)) ); // 0x3FA0 = 1.25 bf16
	REQUIRE(0x3FA3 == bfloat16::log1p(bfloat16::bfloat16(2.6)) ); // 0x3FA3 = 1.2734375 bf16
	REQUIRE(0x3FA7 == bfloat16::log1p(bfloat16::bfloat16(2.7)) ); // 0x3FA7 = 1.3046875 bf16
	REQUIRE(0x3FAA == bfloat16::log1p(bfloat16::bfloat16(2.8)) ); // 0x3FAA = 1.328125 bf16
	REQUIRE(0x3FAD == bfloat16::log1p(bfloat16::bfloat16(2.9)) ); // 0x3FAD = 1.3515625 bf16


	//negative
	REQUIRE(0xFF80 == bfloat16::log1p(bfloat16::bfloat16(-1)) ); // 0xFF80 = -inf bf16
	REQUIRE(0xFFC0 == bfloat16::log1p(bfloat16::bfloat16(-2)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log1p(bfloat16::bfloat16(-3)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log1p(bfloat16::bfloat16(-4)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log1p(bfloat16::bfloat16(-5)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log1p(bfloat16::bfloat16(-6)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log1p(bfloat16::bfloat16(-7)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log1p(bfloat16::bfloat16(-8)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log1p(bfloat16::bfloat16(-9)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log1p(bfloat16::bfloat16(-10)) ); // 0xFFC0 = NaN bf16

	REQUIRE(0xFFC0 == bfloat16::log1p(bfloat16::bfloat16(-2.1)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log1p(bfloat16::bfloat16(-2.2)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log1p(bfloat16::bfloat16(-2.3)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log1p(bfloat16::bfloat16(-2.4)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log1p(bfloat16::bfloat16(-2.5)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log1p(bfloat16::bfloat16(-2.6)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log1p(bfloat16::bfloat16(-2.7)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log1p(bfloat16::bfloat16(-2.8)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::log1p(bfloat16::bfloat16(-2.9)) ); // 0xFFC0 = NaN bf16

}



TEST_CASE("bfloat16_sqrt_temp", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(NAN == bfloat16::sqrt(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive
	REQUIRE(0x0 == bfloat16::sqrt(bfloat16::bfloat16(0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3F80 == bfloat16::sqrt(bfloat16::bfloat16(1)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x3FB5 == bfloat16::sqrt(bfloat16::bfloat16(2)) ); // 0x3FB5 = 1.4140625 bf16
	REQUIRE(0x3FDD == bfloat16::sqrt(bfloat16::bfloat16(3)) ); // 0x3FDD = 1.7265625 bf16
	REQUIRE(0x4000 == bfloat16::sqrt(bfloat16::bfloat16(4)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x400F == bfloat16::sqrt(bfloat16::bfloat16(5)) ); // 0x400F = 2.234375 bf16
	REQUIRE(0x401C == bfloat16::sqrt(bfloat16::bfloat16(6)) ); // 0x401C = 2.4375 bf16
	REQUIRE(0x4029 == bfloat16::sqrt(bfloat16::bfloat16(7)) ); // 0x4029 = 2.640625 bf16
	REQUIRE(0x4035 == bfloat16::sqrt(bfloat16::bfloat16(8)) ); // 0x4035 = 2.828125 bf16
	REQUIRE(0x4040 == bfloat16::sqrt(bfloat16::bfloat16(9)) ); // 0x4040 = 3.0 bf16
	REQUIRE(0x404A == bfloat16::sqrt(bfloat16::bfloat16(10)) ); // 0x404A = 3.15625 bf16

	REQUIRE(0x3F80 == bfloat16::sqrt(bfloat16::bfloat16(1)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x4000 == bfloat16::sqrt(bfloat16::bfloat16(4)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x4040 == bfloat16::sqrt(bfloat16::bfloat16(9)) ); // 0x4040 = 3.0 bf16
	REQUIRE(0x4080 == bfloat16::sqrt(bfloat16::bfloat16(16)) ); // 0x4080 = 4.0 bf16
	REQUIRE(0x40A0 == bfloat16::sqrt(bfloat16::bfloat16(25)) ); // 0x40A0 = 5.0 bf16
	REQUIRE(0x40C0 == bfloat16::sqrt(bfloat16::bfloat16(36)) ); // 0x40C0 = 6.0 bf16
	REQUIRE(0x40E0 == bfloat16::sqrt(bfloat16::bfloat16(49)) ); // 0x40E0 = 7.0 bf16
	REQUIRE(0x4100 == bfloat16::sqrt(bfloat16::bfloat16(64)) ); // 0x4100 = 8.0 bf16
	REQUIRE(0x4110 == bfloat16::sqrt(bfloat16::bfloat16(81)) ); // 0x4110 = 9.0 bf16
	REQUIRE(0x4120 == bfloat16::sqrt(bfloat16::bfloat16(100)) ); // 0x4120 = 10.0 bf16

	REQUIRE(0x3FB9 == bfloat16::sqrt(bfloat16::bfloat16(2.1)) ); // 0x3FB9 = 1.4453125 bf16
	REQUIRE(0x3FBD == bfloat16::sqrt(bfloat16::bfloat16(2.2)) ); // 0x3FBD = 1.4765625 bf16
	REQUIRE(0x3FC1 == bfloat16::sqrt(bfloat16::bfloat16(2.3)) ); // 0x3FC1 = 1.5078125 bf16
	REQUIRE(0x3FC5 == bfloat16::sqrt(bfloat16::bfloat16(2.4)) ); // 0x3FC5 = 1.5390625 bf16
	REQUIRE(0x3FCA == bfloat16::sqrt(bfloat16::bfloat16(2.5)) ); // 0x3FCA = 1.578125 bf16
	REQUIRE(0x3FCE == bfloat16::sqrt(bfloat16::bfloat16(2.6)) ); // 0x3FCE = 1.609375 bf16
	REQUIRE(0x3FD1 == bfloat16::sqrt(bfloat16::bfloat16(2.7)) ); // 0x3FD1 = 1.6328125 bf16
	REQUIRE(0x3FD6 == bfloat16::sqrt(bfloat16::bfloat16(2.8)) ); // 0x3FD6 = 1.671875 bf16
	REQUIRE(0x3FD9 == bfloat16::sqrt(bfloat16::bfloat16(2.9)) ); // 0x3FD9 = 1.6953125 bf16


	//negative
	REQUIRE(0xFFC0 == bfloat16::sqrt(bfloat16::bfloat16(-1)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::sqrt(bfloat16::bfloat16(-2)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::sqrt(bfloat16::bfloat16(-3)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::sqrt(bfloat16::bfloat16(-4)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::sqrt(bfloat16::bfloat16(-5)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::sqrt(bfloat16::bfloat16(-6)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::sqrt(bfloat16::bfloat16(-7)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::sqrt(bfloat16::bfloat16(-8)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::sqrt(bfloat16::bfloat16(-9)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::sqrt(bfloat16::bfloat16(-10)) ); // 0xFFC0 = NaN bf16

	REQUIRE(0xFFC0 == bfloat16::sqrt(bfloat16::bfloat16(-2.1)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::sqrt(bfloat16::bfloat16(-2.2)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::sqrt(bfloat16::bfloat16(-2.3)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::sqrt(bfloat16::bfloat16(-2.4)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::sqrt(bfloat16::bfloat16(-2.5)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::sqrt(bfloat16::bfloat16(-2.6)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::sqrt(bfloat16::bfloat16(-2.7)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::sqrt(bfloat16::bfloat16(-2.8)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::sqrt(bfloat16::bfloat16(-2.9)) ); // 0xFFC0 = NaN bf16

}



TEST_CASE("bfloat16_sqrt", "[floa16]"){

	//works
	
	//positive
	REQUIRE(0x4040 == bfloat16::sqrt(bfloat16::bfloat16(9)));
	REQUIRE(0x4080 == bfloat16::sqrt(bfloat16::bfloat16(16)));
	REQUIRE(0x40A0 == bfloat16::sqrt(bfloat16::bfloat16(25)));
	REQUIRE(0x40C0 == bfloat16::sqrt(bfloat16::bfloat16(36)));
	REQUIRE(0x40E0 == bfloat16::sqrt(bfloat16::bfloat16(49)));
	REQUIRE(0x4100 == bfloat16::sqrt(bfloat16::bfloat16(64)));
	REQUIRE(0x4120 == bfloat16::sqrt(bfloat16::bfloat16(100)));

}



TEST_CASE("bfloat16_cbrt", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(NAN == bfloat16::cbrt(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive
	REQUIRE(0x0 == bfloat16::cbrt(bfloat16::bfloat16(0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3F80 == bfloat16::cbrt(bfloat16::bfloat16(1)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x3FA1 == bfloat16::cbrt(bfloat16::bfloat16(2)) ); // 0x3FA1 = 1.2578125 bf16
	REQUIRE(0x3FB8 == bfloat16::cbrt(bfloat16::bfloat16(3)) ); // 0x3FB8 = 1.4375 bf16
	REQUIRE(0x3FCB == bfloat16::cbrt(bfloat16::bfloat16(4)) ); // 0x3FCB = 1.5859375 bf16
	REQUIRE(0x3FDA == bfloat16::cbrt(bfloat16::bfloat16(5)) ); // 0x3FDA = 1.703125 bf16
	REQUIRE(0x3FE8 == bfloat16::cbrt(bfloat16::bfloat16(6)) ); // 0x3FE8 = 1.8125 bf16
	REQUIRE(0x3FF4 == bfloat16::cbrt(bfloat16::bfloat16(7)) ); // 0x3FF4 = 1.90625 bf16
	REQUIRE(0x4000 == bfloat16::cbrt(bfloat16::bfloat16(8)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x4005 == bfloat16::cbrt(bfloat16::bfloat16(9)) ); // 0x4005 = 2.078125 bf16
	REQUIRE(0x4009 == bfloat16::cbrt(bfloat16::bfloat16(10)) ); // 0x4009 = 2.140625 bf16

	REQUIRE(0x3F80 == bfloat16::cbrt(bfloat16::bfloat16(1)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x4000 == bfloat16::cbrt(bfloat16::bfloat16(8)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x4040 == bfloat16::cbrt(bfloat16::bfloat16(27)) ); // 0x4040 = 3.0 bf16
	REQUIRE(0x4080 == bfloat16::cbrt(bfloat16::bfloat16(64)) ); // 0x4080 = 4.0 bf16
	REQUIRE(0x40A0 == bfloat16::cbrt(bfloat16::bfloat16(125)) ); // 0x40A0 = 5.0 bf16
	REQUIRE(0x40C0 == bfloat16::cbrt(bfloat16::bfloat16(216)) ); // 0x40C0 = 6.0 bf16
	REQUIRE(0x40DF == bfloat16::cbrt(bfloat16::bfloat16(343)) ); // 0x40DF = 7.0 bf16
	REQUIRE(0x4100 == bfloat16::cbrt(bfloat16::bfloat16(512)) ); // 0x4100 = 8.0 bf16
	REQUIRE(0x410F == bfloat16::cbrt(bfloat16::bfloat16(729)) ); // 0x410F = 9.0 bf16
	REQUIRE(0x4120 == bfloat16::cbrt(bfloat16::bfloat16(1000)) ); // 0x4120 = 10.0 bf16

	REQUIRE(0x3FA3 == bfloat16::cbrt(bfloat16::bfloat16(2.1)) ); // 0x3FA3 = 1.2734375 bf16
	REQUIRE(0x3FA6 == bfloat16::cbrt(bfloat16::bfloat16(2.2)) ); // 0x3FA6 = 1.296875 bf16
	REQUIRE(0x3FA8 == bfloat16::cbrt(bfloat16::bfloat16(2.3)) ); // 0x3FA8 = 1.3125 bf16
	REQUIRE(0x3FAB == bfloat16::cbrt(bfloat16::bfloat16(2.4)) ); // 0x3FAB = 1.3359375 bf16
	REQUIRE(0x3FAD == bfloat16::cbrt(bfloat16::bfloat16(2.5)) ); // 0x3FAD = 1.3515625 bf16
	REQUIRE(0x3FAF == bfloat16::cbrt(bfloat16::bfloat16(2.6)) ); // 0x3FAF = 1.3671875 bf16
	REQUIRE(0x3FB1 == bfloat16::cbrt(bfloat16::bfloat16(2.7)) ); // 0x3FB1 = 1.3828125 bf16
	REQUIRE(0x3FB4 == bfloat16::cbrt(bfloat16::bfloat16(2.8)) ); // 0x3FB4 = 1.40625 bf16
	REQUIRE(0x3FB6 == bfloat16::cbrt(bfloat16::bfloat16(2.9)) ); // 0x3FB6 = 1.421875 bf16


	//negative
	REQUIRE(0xBF80 == bfloat16::cbrt(bfloat16::bfloat16(-1)) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0xBFA1 == bfloat16::cbrt(bfloat16::bfloat16(-2)) ); // 0xBFA1 = -1.2578125 bf16
	REQUIRE(0xBFB8 == bfloat16::cbrt(bfloat16::bfloat16(-3)) ); // 0xBFB8 = -1.4375 bf16
	REQUIRE(0xBFCB == bfloat16::cbrt(bfloat16::bfloat16(-4)) ); // 0xBFCB = -1.5859375 bf16
	REQUIRE(0xBFDA == bfloat16::cbrt(bfloat16::bfloat16(-5)) ); // 0xBFDA = -1.703125 bf16
	REQUIRE(0xBFE8 == bfloat16::cbrt(bfloat16::bfloat16(-6)) ); // 0xBFE8 = -1.8125 bf16
	REQUIRE(0xBFF4 == bfloat16::cbrt(bfloat16::bfloat16(-7)) ); // 0xBFF4 = -1.90625 bf16
	REQUIRE(0xC000 == bfloat16::cbrt(bfloat16::bfloat16(-8)) ); // 0xC000 = -2.0 bf16
	REQUIRE(0xC005 == bfloat16::cbrt(bfloat16::bfloat16(-9)) ); // 0xC005 = -2.078125 bf16
	REQUIRE(0xC009 == bfloat16::cbrt(bfloat16::bfloat16(-10)) ); // 0xC009 = -2.140625 bf16

	REQUIRE(0xBFA3 == bfloat16::cbrt(bfloat16::bfloat16(-2.1)) ); // 0xBFA3 = -1.2734375 bf16
	REQUIRE(0xBFA6 == bfloat16::cbrt(bfloat16::bfloat16(-2.2)) ); // 0xBFA6 = -1.296875 bf16
	REQUIRE(0xBFA8 == bfloat16::cbrt(bfloat16::bfloat16(-2.3)) ); // 0xBFA8 = -1.3125 bf16
	REQUIRE(0xBFAB == bfloat16::cbrt(bfloat16::bfloat16(-2.4)) ); // 0xBFAB = -1.3359375 bf16
	REQUIRE(0xBFAD == bfloat16::cbrt(bfloat16::bfloat16(-2.5)) ); // 0xBFAD =-1.3515625  bf16
	REQUIRE(0xBFAF == bfloat16::cbrt(bfloat16::bfloat16(-2.6)) ); // 0xBFAF = -1.3671875 bf16
	REQUIRE(0xBFB1 == bfloat16::cbrt(bfloat16::bfloat16(-2.7)) ); // 0xBFB1 = -1.3828125 bf16
	REQUIRE(0xBFB4 == bfloat16::cbrt(bfloat16::bfloat16(-2.8)) ); // 0xBFB4 = -1.40625 bf16
	REQUIRE(0xBFB6 == bfloat16::cbrt(bfloat16::bfloat16(-2.9)) ); // 0xBFB6 = -1.421875 bf16

}



TEST_CASE("bfloat16_hypot2", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(NAN == bfloat16::hypot(bfloat16::bfloat16(2), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	REQUIRE(NAN == bfloat16::hypot(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), bfloat16::bfloat16(2)) );
	REQUIRE(NAN == bfloat16::hypot(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()));


	//positive
	REQUIRE(0x0 == bfloat16::hypot(bfloat16::bfloat16(0), bfloat16::bfloat16(0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x40A0 == bfloat16::hypot(bfloat16::bfloat16(3), bfloat16::bfloat16(4)) ); // 0x40A0 = 5.0 bf16
	REQUIRE(0x40A0 == bfloat16::hypot(bfloat16::bfloat16(4), bfloat16::bfloat16(3)) ); // 0x40A0 = 5.0 bf16
	REQUIRE(0x40BA == bfloat16::hypot(bfloat16::bfloat16(5), bfloat16::bfloat16(3)) ); // 0x40BA = 5.8125 bf16
	REQUIRE(0x40BA == bfloat16::hypot(bfloat16::bfloat16(3), bfloat16::bfloat16(5)) ); // 0x40BA = 5.8125 bf16
	REQUIRE(0x40CC == bfloat16::hypot(bfloat16::bfloat16(4), bfloat16::bfloat16(5)) ); // 0x40CC = 6.375 bf16
	REQUIRE(0x40CC == bfloat16::hypot(bfloat16::bfloat16(5), bfloat16::bfloat16(4)) ); // 0x40CC = 6.375 bf16
	REQUIRE(0x4150 == bfloat16::hypot(bfloat16::bfloat16(5), bfloat16::bfloat16(12)) ); // 0x4150 = 13.0 bf16
	REQUIRE(0x4150 == bfloat16::hypot(bfloat16::bfloat16(12), bfloat16::bfloat16(5)) ); // 0x4150 = 13.0 bf16
	REQUIRE(0x418D == bfloat16::hypot(bfloat16::bfloat16(12), bfloat16::bfloat16(13)) ); // 0x418D = 17.625 bf16
	REQUIRE(0x418D == bfloat16::hypot(bfloat16::bfloat16(13), bfloat16::bfloat16(12)) ); // 0x418D = 17.625 bf16
	REQUIRE(0x415E == bfloat16::hypot(bfloat16::bfloat16(5), bfloat16::bfloat16(13)) ); // 0x415E = 13.875 bf16
	REQUIRE(0x415E == bfloat16::hypot(bfloat16::bfloat16(13), bfloat16::bfloat16(5)) ); // 0x415E = 13.875 bf16

	//negative 1x
	REQUIRE(0x40A0 == bfloat16::hypot(bfloat16::bfloat16(-3), bfloat16::bfloat16(4)) ); // 0x40A0 = 5.0 bf16
	REQUIRE(0x40A0 == bfloat16::hypot(bfloat16::bfloat16(-4), bfloat16::bfloat16(3)) ); // 0x40A0 = 5.0 bf16
	REQUIRE(0x40BA == bfloat16::hypot(bfloat16::bfloat16(-5), bfloat16::bfloat16(3)) ); // 0x40BA = 5.8125 bf16
	REQUIRE(0x40BA == bfloat16::hypot(bfloat16::bfloat16(-3), bfloat16::bfloat16(5)) ); // 0x40BA = 5.8125 bf16
	REQUIRE(0x40CC == bfloat16::hypot(bfloat16::bfloat16(-4), bfloat16::bfloat16(5)) ); // 0x40CC = 6.375 bf16
	REQUIRE(0x40CC == bfloat16::hypot(bfloat16::bfloat16(-5), bfloat16::bfloat16(4)) ); // 0x40CC = 6.375 bf16
	REQUIRE(0x4150 == bfloat16::hypot(bfloat16::bfloat16(-5), bfloat16::bfloat16(12)) ); // 0x4150 = 13.0 bf16
	REQUIRE(0x4150 == bfloat16::hypot(bfloat16::bfloat16(-12), bfloat16::bfloat16(5)) ); // 0x4150 = 13.0 bf16
	REQUIRE(0x418D == bfloat16::hypot(bfloat16::bfloat16(-12), bfloat16::bfloat16(13)) ); // 0x418D = 17.625 bf16
	REQUIRE(0x418D == bfloat16::hypot(bfloat16::bfloat16(-13), bfloat16::bfloat16(12)) ); // 0x418D = 17.625 bf16
	REQUIRE(0x415E == bfloat16::hypot(bfloat16::bfloat16(-5), bfloat16::bfloat16(13)) ); // 0x415E = 13.875 bf16
	REQUIRE(0x415E == bfloat16::hypot(bfloat16::bfloat16(-13), bfloat16::bfloat16(5)) ); // 0x415E = 13.875 bf16

	REQUIRE(0x40A0 == bfloat16::hypot(bfloat16::bfloat16(3), bfloat16::bfloat16(-4)) ); // 0x40A0 = 5.0 bf16
	REQUIRE(0x40A0 == bfloat16::hypot(bfloat16::bfloat16(4), bfloat16::bfloat16(-3)) ); // 0x40A0 = 5.0 bf16
	REQUIRE(0x40BA == bfloat16::hypot(bfloat16::bfloat16(5), bfloat16::bfloat16(-3)) ); // 0x40BA = 5.8125 bf16
	REQUIRE(0x40BA == bfloat16::hypot(bfloat16::bfloat16(3), bfloat16::bfloat16(-5)) ); // 0x40BA = 5.8125 bf16
	REQUIRE(0x40CC == bfloat16::hypot(bfloat16::bfloat16(4), bfloat16::bfloat16(-5)) ); // 0x40CC = 6.375 bf16
	REQUIRE(0x40CC == bfloat16::hypot(bfloat16::bfloat16(5), bfloat16::bfloat16(-4)) ); // 0x40CC = 6.375 bf16
	REQUIRE(0x4150 == bfloat16::hypot(bfloat16::bfloat16(5), bfloat16::bfloat16(-12)) ); // 0x4150 = 13.0 bf16
	REQUIRE(0x4150 == bfloat16::hypot(bfloat16::bfloat16(12), bfloat16::bfloat16(-5)) ); // 0x4150 = 13.0 bf16
	REQUIRE(0x418D == bfloat16::hypot(bfloat16::bfloat16(12), bfloat16::bfloat16(-13)) ); // 0x418D = 17.625 bf16
	REQUIRE(0x418D == bfloat16::hypot(bfloat16::bfloat16(13), bfloat16::bfloat16(-12)) ); // 0x418D = 17.625 bf16
	REQUIRE(0x415E == bfloat16::hypot(bfloat16::bfloat16(5), bfloat16::bfloat16(-13)) ); // 0x415E = 13.875 bf16
	REQUIRE(0x415E == bfloat16::hypot(bfloat16::bfloat16(13), bfloat16::bfloat16(-5)) ); // 0x415E = 13.875 bf16

	//negative 2x
	REQUIRE(0x40A0 == bfloat16::hypot(bfloat16::bfloat16(-3), bfloat16::bfloat16(-4)) ); // 0x40A0 = 5.0 bf16
	REQUIRE(0x40A0 == bfloat16::hypot(bfloat16::bfloat16(-4), bfloat16::bfloat16(-3)) ); // 0x40A0 = 5.0 bf16
	REQUIRE(0x40BA == bfloat16::hypot(bfloat16::bfloat16(-5), bfloat16::bfloat16(-3)) ); // 0x40BA = 5.8125 bf16
	REQUIRE(0x40BA == bfloat16::hypot(bfloat16::bfloat16(-3), bfloat16::bfloat16(-5)) ); // 0x40BA = 5.8125 bf16
	REQUIRE(0x40CC == bfloat16::hypot(bfloat16::bfloat16(-4), bfloat16::bfloat16(-5)) ); // 0x40CC = 6.375 bf16
	REQUIRE(0x40CC == bfloat16::hypot(bfloat16::bfloat16(-5), bfloat16::bfloat16(-4)) ); // 0x40CC = 6.375 bf16
	REQUIRE(0x4150 == bfloat16::hypot(bfloat16::bfloat16(-5), bfloat16::bfloat16(-12)) ); // 0x4150 = 13.0 bf16
	REQUIRE(0x4150 == bfloat16::hypot(bfloat16::bfloat16(-12), bfloat16::bfloat16(-5)) ); // 0x4150 = 13.0 bf16
	REQUIRE(0x418D == bfloat16::hypot(bfloat16::bfloat16(-12), bfloat16::bfloat16(-13)) ); // 0x418D = 17.625 bf16
	REQUIRE(0x418D == bfloat16::hypot(bfloat16::bfloat16(-13), bfloat16::bfloat16(-12)) ); // 0x418D = 17.625 bf16
	REQUIRE(0x415E == bfloat16::hypot(bfloat16::bfloat16(-5), bfloat16::bfloat16(-13)) ); // 0x415E = 13.875 bf16
	REQUIRE(0x415E == bfloat16::hypot(bfloat16::bfloat16(-13), bfloat16::bfloat16(-5)) ); // 0x415E = 13.875 bf16

}



TEST_CASE("bfloat16_hypot3", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(NAN == bfloat16::hypot(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), bfloat16::bfloat16(2), bfloat16::bfloat16(2)) );
	REQUIRE(NAN == bfloat16::hypot(bfloat16::bfloat16(2), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), bfloat16::bfloat16(2)) );
	REQUIRE(NAN == bfloat16::hypot(bfloat16::bfloat16(2), bfloat16::bfloat16(2), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	REQUIRE(NAN == bfloat16::hypot(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), bfloat16::bfloat16(2)) );
	REQUIRE(NAN == bfloat16::hypot(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), bfloat16::bfloat16(2), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	REQUIRE(NAN == bfloat16::hypot(bfloat16::bfloat16(2), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	REQUIRE(NAN == bfloat16::hypot(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive 3x
	REQUIRE(0x0 == bfloat16::hypot(bfloat16::bfloat16(0), bfloat16::bfloat16(0), bfloat16::bfloat16(0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(3), bfloat16::bfloat16(4), bfloat16::bfloat16(5)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(3), bfloat16::bfloat16(5), bfloat16::bfloat16(4)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(4), bfloat16::bfloat16(5), bfloat16::bfloat16(3)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(4), bfloat16::bfloat16(3), bfloat16::bfloat16(5)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(5), bfloat16::bfloat16(4), bfloat16::bfloat16(3)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(5), bfloat16::bfloat16(3), bfloat16::bfloat16(4)) ); // 0x40E2 = 7.0625 bf16

	//positiv 2x negativ 1x
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-3), bfloat16::bfloat16(4), bfloat16::bfloat16(5)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-3), bfloat16::bfloat16(5), bfloat16::bfloat16(4)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-4), bfloat16::bfloat16(5), bfloat16::bfloat16(3)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-4), bfloat16::bfloat16(3), bfloat16::bfloat16(5)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-5), bfloat16::bfloat16(4), bfloat16::bfloat16(3)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-5), bfloat16::bfloat16(3), bfloat16::bfloat16(4)) ); // 0x40E2 = 7.0625 bf16

	//positiv 2x negativ 1x
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(3), bfloat16::bfloat16(-4), bfloat16::bfloat16(5)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(3), bfloat16::bfloat16(-5), bfloat16::bfloat16(4)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(4), bfloat16::bfloat16(-5), bfloat16::bfloat16(3)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(4), bfloat16::bfloat16(-3), bfloat16::bfloat16(5)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(5), bfloat16::bfloat16(-4), bfloat16::bfloat16(3)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(5), bfloat16::bfloat16(-3), bfloat16::bfloat16(4)) ); // 0x40E2 = 7.0625 bf16

	//positiv 2x negativ 1x
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(3), bfloat16::bfloat16(4), bfloat16::bfloat16(-5)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(3), bfloat16::bfloat16(5), bfloat16::bfloat16(-4)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(4), bfloat16::bfloat16(5), bfloat16::bfloat16(-3)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(4), bfloat16::bfloat16(3), bfloat16::bfloat16(-5)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(5), bfloat16::bfloat16(4), bfloat16::bfloat16(-3)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(5), bfloat16::bfloat16(3), bfloat16::bfloat16(-4)) ); // 0x40E2 = 7.0625 bf16

	//positiv 1x negativ 2x
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-3), bfloat16::bfloat16(-4), bfloat16::bfloat16(5)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-3), bfloat16::bfloat16(-5), bfloat16::bfloat16(4)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-4), bfloat16::bfloat16(-5), bfloat16::bfloat16(3)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-4), bfloat16::bfloat16(-3), bfloat16::bfloat16(5)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-5), bfloat16::bfloat16(-4), bfloat16::bfloat16(3)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-5), bfloat16::bfloat16(-3), bfloat16::bfloat16(4)) ); // 0x40E2 = 7.0625 bf16

	//positiv 1x negativ 2x
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-3), bfloat16::bfloat16(4), bfloat16::bfloat16(-5)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-3), bfloat16::bfloat16(5), bfloat16::bfloat16(-4)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-4), bfloat16::bfloat16(5), bfloat16::bfloat16(-3)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-4), bfloat16::bfloat16(3), bfloat16::bfloat16(-5)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-5), bfloat16::bfloat16(4), bfloat16::bfloat16(-3)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-5), bfloat16::bfloat16(3), bfloat16::bfloat16(-4)) ); // 0x40E2 = 7.0625 bf16

	//positiv 1x negativ 2x
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(3), bfloat16::bfloat16(-4), bfloat16::bfloat16(-5)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(3), bfloat16::bfloat16(-5), bfloat16::bfloat16(-4)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(4), bfloat16::bfloat16(-5), bfloat16::bfloat16(-3)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(4), bfloat16::bfloat16(-3), bfloat16::bfloat16(-5)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(5), bfloat16::bfloat16(-4), bfloat16::bfloat16(-3)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(5), bfloat16::bfloat16(-3), bfloat16::bfloat16(-4)) ); // 0x40E2 = 7.0625 bf16

	//negativ 3x
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-3), bfloat16::bfloat16(-4), bfloat16::bfloat16(-5)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-3), bfloat16::bfloat16(-5), bfloat16::bfloat16(-4)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-4), bfloat16::bfloat16(-5), bfloat16::bfloat16(-3)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-4), bfloat16::bfloat16(-3), bfloat16::bfloat16(-5)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-5), bfloat16::bfloat16(-4), bfloat16::bfloat16(-3)) ); // 0x40E2 = 7.0625 bf16
	REQUIRE(0x40E2 == bfloat16::hypot(bfloat16::bfloat16(-5), bfloat16::bfloat16(-3), bfloat16::bfloat16(-4)) ); // 0x40E2 = 7.0625 bf16

}



TEST_CASE("bfloat16_pow", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(0x7F80 == bfloat16::pow(bfloat16::bfloat16(0), bfloat16::bfloat16(-3)) );
	REQUIRE(0x7F80 == bfloat16::pow(bfloat16::bfloat16(-0), bfloat16::bfloat16(-3)) );//-inf
	REQUIRE(0x7F80 == bfloat16::pow(bfloat16::bfloat16(0), bfloat16::bfloat16(-4)) );
	REQUIRE(0x7F80 == bfloat16::pow(bfloat16::bfloat16(-0), bfloat16::bfloat16(-4)) );
	REQUIRE(0x7F80 == bfloat16::pow(bfloat16::bfloat16(0), -std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x7F80 == bfloat16::pow(bfloat16::bfloat16(-0), -std::numeric_limits<bfloat16::bfloat16>::infinity() ) );

	REQUIRE(0x0 == bfloat16::pow(bfloat16::bfloat16(0), bfloat16::bfloat16(3)) );
	REQUIRE(0x0 == bfloat16::pow(bfloat16::bfloat16(-0), bfloat16::bfloat16(3)) ); // -0
	REQUIRE(0x0 == bfloat16::pow(bfloat16::bfloat16(0), bfloat16::bfloat16(4)) );
	REQUIRE(0x0 == bfloat16::pow(bfloat16::bfloat16(-0), bfloat16::bfloat16(4)) );
	REQUIRE(0x3F80 == bfloat16::pow(bfloat16::bfloat16(-1), std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x3F80 == bfloat16::pow(bfloat16::bfloat16(-1), -std::numeric_limits<bfloat16::bfloat16>::infinity()) );

	REQUIRE(0x3F80 == bfloat16::pow(bfloat16::bfloat16(1), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	REQUIRE(0x3F80 == bfloat16::pow(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), bfloat16::bfloat16(0)) );
	REQUIRE(NAN == bfloat16::pow(bfloat16::bfloat16(-2), bfloat16::bfloat16(1.5)) );
	REQUIRE(0x3F80 == bfloat16::pow(bfloat16::bfloat16(-1), -std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x0 == bfloat16::pow(bfloat16::bfloat16(2), -std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x3F80 == bfloat16::pow(bfloat16::bfloat16(-1), std::numeric_limits<bfloat16::bfloat16>::infinity()) );

	REQUIRE(0x7F80 == bfloat16::pow(bfloat16::bfloat16(2), std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::pow(bfloat16::bfloat16(2), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	REQUIRE(NAN == bfloat16::pow(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), bfloat16::bfloat16(2)) );
	REQUIRE(NAN == bfloat16::pow(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );

	REQUIRE(0x8000 == bfloat16::pow(-std::numeric_limits<bfloat16::bfloat16>::infinity(), bfloat16::bfloat16(-3)) );
	REQUIRE(0x0 == bfloat16::pow(-std::numeric_limits<bfloat16::bfloat16>::infinity(), bfloat16::bfloat16(-1.5)) );
	REQUIRE(0x0 == bfloat16::pow(-std::numeric_limits<bfloat16::bfloat16>::infinity(), bfloat16::bfloat16(-4)) );
	REQUIRE(0xFF80 == bfloat16::pow(-std::numeric_limits<bfloat16::bfloat16>::infinity(), bfloat16::bfloat16(3)) );
	REQUIRE(0x7F80 == bfloat16::pow(-std::numeric_limits<bfloat16::bfloat16>::infinity(), bfloat16::bfloat16(1.5)) );
	REQUIRE(0x7F80 == bfloat16::pow(-std::numeric_limits<bfloat16::bfloat16>::infinity(), bfloat16::bfloat16(4)) );

	REQUIRE(0x0 == bfloat16::pow(std::numeric_limits<bfloat16::bfloat16>::infinity(), bfloat16::bfloat16(-5)) );
	REQUIRE(0x7F80 == bfloat16::pow(std::numeric_limits<bfloat16::bfloat16>::infinity(), bfloat16::bfloat16(5)) );


	//positive
	REQUIRE(0x3F80 == bfloat16::pow(bfloat16::bfloat16(0), bfloat16::bfloat16(0)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x3F80 == bfloat16::pow(bfloat16::bfloat16(1), bfloat16::bfloat16(0)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x3F80 == bfloat16::pow(bfloat16::bfloat16(1), bfloat16::bfloat16(2)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x3F80 == bfloat16::pow(bfloat16::bfloat16(1), bfloat16::bfloat16(10)) ); // 0x3F80 = 1.0 bf16

	REQUIRE(0x3F80 == bfloat16::pow(bfloat16::bfloat16(2), bfloat16::bfloat16(0)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x4000 == bfloat16::pow(bfloat16::bfloat16(2), bfloat16::bfloat16(1)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x4080 == bfloat16::pow(bfloat16::bfloat16(2), bfloat16::bfloat16(2)) ); // 0x4080 = 4.0 bf16
	REQUIRE(0x4100 == bfloat16::pow(bfloat16::bfloat16(2), bfloat16::bfloat16(3)) ); // 0x4100 = 8.0 bf16
	REQUIRE(0x4180 == bfloat16::pow(bfloat16::bfloat16(2), bfloat16::bfloat16(4)) ); // 0x4180 = 16.0 bf16
	REQUIRE(0x4200 == bfloat16::pow(bfloat16::bfloat16(2), bfloat16::bfloat16(5)) ); // 0x4200 = 32.0 bf16
	REQUIRE(0x4280 == bfloat16::pow(bfloat16::bfloat16(2), bfloat16::bfloat16(6)) ); // 0x4280 = 64.0 bf16

	//negative
	REQUIRE(0x3F00 == bfloat16::pow(bfloat16::bfloat16(2), bfloat16::bfloat16(-1)) ); // 0x3F00 = 0.5 bf16
	REQUIRE(0x3e80 == bfloat16::pow(bfloat16::bfloat16(2), bfloat16::bfloat16(-2)) ); // 0x3e80 = 0.25 bf16
	REQUIRE(0x3e00 == bfloat16::pow(bfloat16::bfloat16(2), bfloat16::bfloat16(-3)) ); // 0x3e00 = 0.125 bf16
	REQUIRE(0x3d80 == bfloat16::pow(bfloat16::bfloat16(2), bfloat16::bfloat16(-4)) ); // 0x3d80 = 0.0625 bf16
	REQUIRE(0x3d00 == bfloat16::pow(bfloat16::bfloat16(2), bfloat16::bfloat16(-5)) ); // 0x3d00 = 0.03125 bf16
	REQUIRE(0x3c80 == bfloat16::pow(bfloat16::bfloat16(2), bfloat16::bfloat16(-6)) ); // 0x3c80 = 0.015625 bf16

}


//TODO pointers do something stupid, won't get the result
TEST_CASE("bfloat16_sincos", "[bfloat16]"){


	bfloat16::bfloat16 *sin;
	bfloat16::bfloat16 *cos;
	bfloat16::sincos(bfloat16::bfloat16(1), sin, cos);

	bfloat16::bfloat16 sinv = *sin;
	bfloat16::bfloat16 cosv = *cos;

	std::cout << "sin: " << sinv << " cos: " << cosv << std::endl;


}



TEST_CASE("bfloat16_sin", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(NAN == bfloat16::sin(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::sin(-std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::sin(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	

	//solution in rad
	//positive
	REQUIRE(0x0 == bfloat16::sin(bfloat16::bfloat16(0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3F57 == bfloat16::sin(bfloat16::bfloat16(1)) ); // 0x3F57 = 0.83984375 bf16
	REQUIRE(0x3F68 == bfloat16::sin(bfloat16::bfloat16(2)) ); // 0x3F68 = 0.90625 bf16
	REQUIRE(0x3E10 == bfloat16::sin(bfloat16::bfloat16(3)) ); // 0x3E10 = 0.140625 bf16
	REQUIRE(0xBF41 == bfloat16::sin(bfloat16::bfloat16(4)) ); // 0xBF41 = -0.75390625 bf16
	REQUIRE(0xBF75 == bfloat16::sin(bfloat16::bfloat16(5)) ); // 0xBF75 = -0.95703125 bf16
	REQUIRE(0xBE8F == bfloat16::sin(bfloat16::bfloat16(6)) ); // 0xBE8F = -0.279296875 bf16
	REQUIRE(0x3F28 == bfloat16::sin(bfloat16::bfloat16(7)) ); // 0x3F28 = 0.65625 bf16
	REQUIRE(0x3F7D == bfloat16::sin(bfloat16::bfloat16(8)) ); // 0x3F7D = 0.98828125 bf16
	REQUIRE(0x3ED3 == bfloat16::sin(bfloat16::bfloat16(9)) ); // 0x3ED3 = 0.412109375 bf16


	//negative
	REQUIRE(0xBF57 == bfloat16::sin(bfloat16::bfloat16(-1)) ); // 0xBF57 = -0.83984375 bf16
	REQUIRE(0xBF68 == bfloat16::sin(bfloat16::bfloat16(-2)) ); // 0xBF68 = -0.90625 bf16
	REQUIRE(0xBE10 == bfloat16::sin(bfloat16::bfloat16(-3)) ); // 0xBE10 = -0.140625 bf16
	REQUIRE(0x3F41 == bfloat16::sin(bfloat16::bfloat16(-4)) ); // 0x3F41 = 0.75390625 bf16
	REQUIRE(0x3F75 == bfloat16::sin(bfloat16::bfloat16(-5)) ); // 0x3F75 = 0.95703125 bf16
	REQUIRE(0x3E8F == bfloat16::sin(bfloat16::bfloat16(-6)) ); // 0x3E8F = 0.279296875 bf16
	REQUIRE(0xBF28 == bfloat16::sin(bfloat16::bfloat16(-7)) ); // 0xBF28 = -0.65625 bf16
	REQUIRE(0xBF7D == bfloat16::sin(bfloat16::bfloat16(-8)) ); // 0xBF7D = -0.98828125 bf16
	REQUIRE(0xBED3 == bfloat16::sin(bfloat16::bfloat16(-9)) ); // 0xBED3 = -0.412109375 bf16

}



TEST_CASE("bfloat16_cos", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(NAN == bfloat16::cos(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::cos(-std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::cos(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//solution in rad
	//positive
	REQUIRE(0x3F80 == bfloat16::cos(bfloat16::bfloat16(0)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x3F0A == bfloat16::cos(bfloat16::bfloat16(1)) ); // 0x3F0A = 0.5390625 bf16
	REQUIRE(0xBED5 == bfloat16::cos(bfloat16::bfloat16(2)) ); // 0xBED5 = -0.416015625 bf16
	REQUIRE(0xBF7D == bfloat16::cos(bfloat16::bfloat16(3)) ); // 0xBF7D = -0.98828125 bf16
	REQUIRE(0xBF27 == bfloat16::cos(bfloat16::bfloat16(4)) ); // 0xBF27 = -0.65234375 bf16
	REQUIRE(0x3E91 == bfloat16::cos(bfloat16::bfloat16(5)) ); // 0x3E91 = 0.283203125 bf16
	REQUIRE(0x3F75 == bfloat16::cos(bfloat16::bfloat16(6)) ); // 0x3F75 = 0.95703125 bf16
	REQUIRE(0x3F40 == bfloat16::cos(bfloat16::bfloat16(7)) ); // 0x3F40 = 0.75 bf16
	REQUIRE(0xBE14 == bfloat16::cos(bfloat16::bfloat16(8)) ); // 0xBE14 = -0.14453125 bf16
	REQUIRE(0xBF69 == bfloat16::cos(bfloat16::bfloat16(9)) ); // 0xBF69 = -0.91015625 bf16

	
	//negative
	REQUIRE(0x3F0A == bfloat16::cos(bfloat16::bfloat16(-1)) ); // 0x3F0A = 0.5390625 bf16
	REQUIRE(0xBED5 == bfloat16::cos(bfloat16::bfloat16(-2)) ); // 0xBED5 = -0.416015625 bf16
	REQUIRE(0xBF7D == bfloat16::cos(bfloat16::bfloat16(-3)) ); // 0xBF7D = -0.98828125 bf16
	REQUIRE(0xBF27 == bfloat16::cos(bfloat16::bfloat16(-4)) ); // 0xBF27 = -0.65234375 bf16
	REQUIRE(0x3E91 == bfloat16::cos(bfloat16::bfloat16(-5)) ); // 0x3E91 = 0.283203125 bf16
	REQUIRE(0x3F75 == bfloat16::cos(bfloat16::bfloat16(-6)) ); // 0x3F75 = 0.95703125 bf16
	REQUIRE(0x3F40 == bfloat16::cos(bfloat16::bfloat16(-7)) ); // 0x3F40 = 0.75 bf16
	REQUIRE(0xBE14 == bfloat16::cos(bfloat16::bfloat16(-8)) ); // 0xBE14 = -0.14453125 bf16
	REQUIRE(0xBF69 == bfloat16::cos(bfloat16::bfloat16(-9)) ); // 0xBF69 = -0.91015625 bf16

}



TEST_CASE("bfloat16_tan", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(NAN == bfloat16::tan(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::tan(-std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::tan(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );

	//solution in rad
	//positive
	REQUIRE(0x0 == bfloat16::tan(bfloat16::bfloat16(0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3FC7 == bfloat16::tan(bfloat16::bfloat16(1)) ); // 0x3FC7 = 1.5546875 bf16
	REQUIRE(0xC00B == bfloat16::tan(bfloat16::bfloat16(2)) ); // 0xC00B = -2.171875 bf16
	REQUIRE(0xBE11 == bfloat16::tan(bfloat16::bfloat16(3)) ); // 0xBE11 = -0.1416015625 bf16
	REQUIRE(0x3F94 == bfloat16::tan(bfloat16::bfloat16(4)) ); // 0x3F94 = 1.15625 bf16
	REQUIRE(0xC058 == bfloat16::tan(bfloat16::bfloat16(5)) ); // 0xC058 = -3.375 bf16
	REQUIRE(0xBE94 == bfloat16::tan(bfloat16::bfloat16(6)) ); // 0xBE94 = -0.2890625 bf16
	REQUIRE(0x3F5F == bfloat16::tan(bfloat16::bfloat16(7)) ); // 0x3F5F = 0.87109375 bf16
	REQUIRE(0xC0D9 == bfloat16::tan(bfloat16::bfloat16(8)) ); // 0xC0D9 = -6.78125 bf16
	REQUIRE(0xBEE7 == bfloat16::tan(bfloat16::bfloat16(9)) ); // 0xBEE7 = -0.451171875 bf16

	//negative
	REQUIRE(0xBFC7 == bfloat16::tan(bfloat16::bfloat16(-1)) ); // 0xBFC7 = -1.5546875 bf16
	REQUIRE(0x400B == bfloat16::tan(bfloat16::bfloat16(-2)) ); // 0x400B = 2.171875 bf16
	REQUIRE(0x3E11 == bfloat16::tan(bfloat16::bfloat16(-3)) ); // 0x3E11 = 0.1416015625 bf16
	REQUIRE(0xBF94 == bfloat16::tan(bfloat16::bfloat16(-4)) ); // 0xBF94 = -1.15625 bf16
	REQUIRE(0x4058 == bfloat16::tan(bfloat16::bfloat16(-5)) ); // 0x4058 = 3.375 bf16
	REQUIRE(0x3E94 == bfloat16::tan(bfloat16::bfloat16(-6)) ); // 0x3E94 = 0.2890625 bf16
	REQUIRE(0xBF5F == bfloat16::tan(bfloat16::bfloat16(-7)) ); // 0xBF5F = -0.87109375 bf16
	REQUIRE(0x40D9 == bfloat16::tan(bfloat16::bfloat16(-8)) ); // 0x40D9 = 6.78125 bf16
	REQUIRE(0x3EE7 == bfloat16::tan(bfloat16::bfloat16(-9)) ); // 0x3EE7 = 0.451171875 bf16

}



TEST_CASE("bfloat16_asin", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(NAN == bfloat16::asin(bfloat16::bfloat16(2)) );
	REQUIRE(NAN == bfloat16::asin(bfloat16::bfloat16(-2)) );
	REQUIRE(NAN == bfloat16::asin(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//solution in rad
	//positive
	REQUIRE(0x0 == bfloat16::asin(bfloat16::bfloat16(0.0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3DCC == bfloat16::asin(bfloat16::bfloat16(0.1)) ); // 0x3DCC = 0.099609375 bf16
	REQUIRE(0x3E4D == bfloat16::asin(bfloat16::bfloat16(0.2)) ); // 0x3E4D = 0.2001953125 bf16
	REQUIRE(0x3E9B == bfloat16::asin(bfloat16::bfloat16(0.3)) ); // 0x3E9B = 0.302734375 bf16
	REQUIRE(0x3ED1 == bfloat16::asin(bfloat16::bfloat16(0.4)) ); // 0x3ED1 = 0.408203125 bf16
	REQUIRE(0x3F06 == bfloat16::asin(bfloat16::bfloat16(0.5)) ); // 0x3F06 = 0.5234375 bf16
	REQUIRE(0x3F23 == bfloat16::asin(bfloat16::bfloat16(0.6)) ); // 0x3F23 = 0.63671875 bf16
	REQUIRE(0x3F46 == bfloat16::asin(bfloat16::bfloat16(0.7)) ); // 0x3F46 = 0.7734375 bf16
	REQUIRE(0x3F6C == bfloat16::asin(bfloat16::bfloat16(0.8)) ); // 0x3F6C = 0.921875 bf16
	REQUIRE(0x3F8E == bfloat16::asin(bfloat16::bfloat16(0.9)) ); // 0x3F8E = 1.109375 bf16
	REQUIRE(0x3FC9 == bfloat16::asin(bfloat16::bfloat16(1.0)) ); // 0x3FC9 = 1.5703125 bf16


	//negative
	REQUIRE(0xBDCC == bfloat16::asin(bfloat16::bfloat16(-0.1)) ); // 0xBDCC = -0.099609375 bf16
	REQUIRE(0xBE4D == bfloat16::asin(bfloat16::bfloat16(-0.2)) ); // 0xBE4D = -0.2001953125 bf16
	REQUIRE(0xBE9B == bfloat16::asin(bfloat16::bfloat16(-0.3)) ); // 0xBE9B = -0.302734375 bf16
	REQUIRE(0xBED1 == bfloat16::asin(bfloat16::bfloat16(-0.4)) ); // 0xBED1 = -0.408203125 bf16
	REQUIRE(0xBF06 == bfloat16::asin(bfloat16::bfloat16(-0.5)) ); // 0xBF06 = -0.5234375 bf16
	REQUIRE(0xBF23 == bfloat16::asin(bfloat16::bfloat16(-0.6)) ); // 0xBF23 = -0.63671875 bf16
	REQUIRE(0xBF46 == bfloat16::asin(bfloat16::bfloat16(-0.7)) ); // 0xBF46 = -0.7734375 bf16
	REQUIRE(0xBF6C == bfloat16::asin(bfloat16::bfloat16(-0.8)) ); // 0xBF6C = -0.921875 bf16
	REQUIRE(0xBF8E == bfloat16::asin(bfloat16::bfloat16(-0.9)) ); // 0xBF8E = -1.109375 bf16
	REQUIRE(0xBFC9 == bfloat16::asin(bfloat16::bfloat16(-1.0)) ); // 0xBFC9 = -1.5703125 bf16

}



TEST_CASE("bfloat16_acos", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(NAN == bfloat16::acos(bfloat16::bfloat16(2)) );
	REQUIRE(NAN == bfloat16::acos(bfloat16::bfloat16(-2)) );
	REQUIRE(NAN == bfloat16::acos(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//solution in rad
	//positive
	REQUIRE(0x3FC9 == bfloat16::acos(bfloat16::bfloat16(0.0)) ); // 0x3FC9 = 1.5703125 bf16
	REQUIRE(0x3FBC == bfloat16::acos(bfloat16::bfloat16(0.1)) ); // 0x3FBC = 1.46875 bf16
	REQUIRE(0x3FAF == bfloat16::acos(bfloat16::bfloat16(0.2)) ); // 0x3FAF = 1.3671875 bf16
	REQUIRE(0x3FA2 == bfloat16::acos(bfloat16::bfloat16(0.3)) ); // 0x3FA2 = 1.265625 bf16
	REQUIRE(0x3F94 == bfloat16::acos(bfloat16::bfloat16(0.4)) ); // 0x3F94 = 1.15625 bf16
	REQUIRE(0x3F86 == bfloat16::acos(bfloat16::bfloat16(0.5)) ); // 0x3F86 = 1.046875 bf16
	REQUIRE(0x3F6E == bfloat16::acos(bfloat16::bfloat16(0.6)) ); // 0x3F6E = 0.9296875 bf16
	REQUIRE(0x3F4B == bfloat16::acos(bfloat16::bfloat16(0.7)) ); // 0x3F4B =  bf16
	REQUIRE(0x3F26 == bfloat16::acos(bfloat16::bfloat16(0.8)) ); // 0x3F26 = 0.79296875 bf16
	REQUIRE(0x3EE8 == bfloat16::acos(bfloat16::bfloat16(0.9)) ); // 0x3EE8 = 0.453125 bf16
	REQUIRE(0x0 == bfloat16::acos(bfloat16::bfloat16(1.0)) ); // 0x0 = 0.0 bf16


	//negative
	REQUIRE(0x3FD5 == bfloat16::acos(bfloat16::bfloat16(-0.1)) ); // 0x3FD5 = 1.6640625 bf16
	REQUIRE(0x3FE2 == bfloat16::acos(bfloat16::bfloat16(-0.2)) ); // 0x3FE2 = 1.765625 bf16
	REQUIRE(0x3FEF == bfloat16::acos(bfloat16::bfloat16(-0.3)) ); // 0x3FEF = 1.8671875 bf16
	REQUIRE(0x3FFD == bfloat16::acos(bfloat16::bfloat16(-0.4)) ); // 0x3FFD = 1.9765625 bf16
	REQUIRE(0x4006 == bfloat16::acos(bfloat16::bfloat16(-0.5)) ); // 0x4006 = 2.09375 bf16
	REQUIRE(0x400D == bfloat16::acos(bfloat16::bfloat16(-0.6)) ); // 0x400D = 2.203125 bf16
	REQUIRE(0x4016 == bfloat16::acos(bfloat16::bfloat16(-0.7)) ); // 0x4016 = 2.34375 bf16
	REQUIRE(0x401F == bfloat16::acos(bfloat16::bfloat16(-0.8)) ); // 0x401F = 2.484375 bf16
	REQUIRE(0x402B == bfloat16::acos(bfloat16::bfloat16(-0.9)) ); // 0x402B = 2.671875 bf16
	REQUIRE(0x4049 == bfloat16::acos(bfloat16::bfloat16(-1.0)) ); // 0x4049 = 3.140625 bf16

}



TEST_CASE("bfloat16_atan", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(0x3FC9 == bfloat16::atan(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0xBFC9 == bfloat16::atan(-std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::sin(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//solution in rad
	//positive
	REQUIRE(0x0 == bfloat16::atan(bfloat16::bfloat16(0.0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3E49 == bfloat16::atan(bfloat16::bfloat16(0.2)) ); // 0x3E49 = 0.1962890625 bf16
	REQUIRE(0x3EC2 == bfloat16::atan(bfloat16::bfloat16(0.4)) ); // 0x3EC2 = 0.37890625 bf16
	REQUIRE(0x3F09 == bfloat16::atan(bfloat16::bfloat16(0.6)) ); // 0x3F09 = 0.53515625 bf16
	REQUIRE(0x3F2C == bfloat16::atan(bfloat16::bfloat16(0.8)) ); // 0x3F2C = 0.671875 bf16
	REQUIRE(0x3F49 == bfloat16::atan(bfloat16::bfloat16(1.0)) ); // 0x3F49 = 0.78515625 bf16
	REQUIRE(0x3F5F == bfloat16::atan(bfloat16::bfloat16(1.2)) ); // 0x3F5F = 0.87109375 bf16
	REQUIRE(0x3F73 == bfloat16::atan(bfloat16::bfloat16(1.4)) ); // 0x3F73 = 0.94921875 bf16
	REQUIRE(0x3F81 == bfloat16::atan(bfloat16::bfloat16(1.6)) ); // 0x3F81 = 1.0078125 bf16
	REQUIRE(0x3F88 == bfloat16::atan(bfloat16::bfloat16(1.8)) ); // 0x3F88 = 1.0625 bf16
	REQUIRE(0x3F8D == bfloat16::atan(bfloat16::bfloat16(2.0)) ); // 0x3F8D = 1.1015625 bf16


	//negative
	REQUIRE(0xBE49 == bfloat16::atan(bfloat16::bfloat16(-0.2)) ); // 0xBE49 = -0.1962890625 bf16
	REQUIRE(0xBEC2 == bfloat16::atan(bfloat16::bfloat16(-0.4)) ); // 0xBEC2 = -0.37890625 bf16
	REQUIRE(0xBF09 == bfloat16::atan(bfloat16::bfloat16(-0.6)) ); // 0xBF09 = -0.53515625 bf16
	REQUIRE(0xBF2C == bfloat16::atan(bfloat16::bfloat16(-0.8)) ); // 0xBF2C = -0.671875 bf16
	REQUIRE(0xBF49 == bfloat16::atan(bfloat16::bfloat16(-1.0)) ); // 0xBF49 = -0.78515625 bf16
	REQUIRE(0xBF5F == bfloat16::atan(bfloat16::bfloat16(-1.2)) ); // 0xBF5F = -0.87109375 bf16
	REQUIRE(0xBF73 == bfloat16::atan(bfloat16::bfloat16(-1.4)) ); // 0xBF73 = -0.94921875 bf16
	REQUIRE(0xBF81 == bfloat16::atan(bfloat16::bfloat16(-1.6)) ); // 0xBF81 = -1.0078125 bf16
	REQUIRE(0xBF88 == bfloat16::atan(bfloat16::bfloat16(-1.8)) ); // 0xBF88 = -1.0625 bf16
	REQUIRE(0xBF8D == bfloat16::atan(bfloat16::bfloat16(-2.0)) ); // 0xBF8D = -1.1015625 bf16

}



TEST_CASE("bfloat16_atan2", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(0x0 == bfloat16::atan2(bfloat16::bfloat16(-0), bfloat16::bfloat16(-0)) ); // should be -+pi == 0x4049/0xC049
	REQUIRE(0x4049 == bfloat16::atan2(bfloat16::bfloat16(-0), bfloat16::bfloat16(-3)) );
	REQUIRE(0x0 == bfloat16::atan2(bfloat16::bfloat16(-0), bfloat16::bfloat16(0)) );
	REQUIRE(0x0 == bfloat16::atan2(bfloat16::bfloat16(-0), bfloat16::bfloat16(3)) );

	REQUIRE(0x0 == bfloat16::atan2(bfloat16::bfloat16(0), bfloat16::bfloat16(-0)) );
	REQUIRE(0x4049 == bfloat16::atan2(bfloat16::bfloat16(0), bfloat16::bfloat16(-3)) );
	REQUIRE(0x0 == bfloat16::atan2(bfloat16::bfloat16(0), bfloat16::bfloat16(0)) );
	REQUIRE(0x0 == bfloat16::atan2(bfloat16::bfloat16(0), bfloat16::bfloat16(3)) );

	REQUIRE(0x3FC9 == bfloat16::atan2(std::numeric_limits<bfloat16::bfloat16>::infinity(), bfloat16::bfloat16(3)) );
	REQUIRE(0x3FC9 == bfloat16::atan2(std::numeric_limits<bfloat16::bfloat16>::infinity(), bfloat16::bfloat16(-3)) );
	REQUIRE(0xBFC9 == bfloat16::atan2(-std::numeric_limits<bfloat16::bfloat16>::infinity(), bfloat16::bfloat16(3)) );
	REQUIRE(0xBFC9 == bfloat16::atan2(-std::numeric_limits<bfloat16::bfloat16>::infinity(), bfloat16::bfloat16(-3)) );

	REQUIRE(0x4016 == bfloat16::atan2(std::numeric_limits<bfloat16::bfloat16>::infinity(), -std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0xC016 == bfloat16::atan2(-std::numeric_limits<bfloat16::bfloat16>::infinity(), -std::numeric_limits<bfloat16::bfloat16>::infinity()) );

	REQUIRE(0x3F49 == bfloat16::atan2(std::numeric_limits<bfloat16::bfloat16>::infinity(), std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0xBF49 == bfloat16::atan2(-std::numeric_limits<bfloat16::bfloat16>::infinity(), std::numeric_limits<bfloat16::bfloat16>::infinity()) );

	REQUIRE(0xBFC9 == bfloat16::atan2(bfloat16::bfloat16(-3), bfloat16::bfloat16(0)) );
	REQUIRE(0xBFC9 == bfloat16::atan2(bfloat16::bfloat16(-3), bfloat16::bfloat16(-0)) );

	REQUIRE(0x3FC9 == bfloat16::atan2(bfloat16::bfloat16(3), bfloat16::bfloat16(0)) );
	REQUIRE(0x3FC9 == bfloat16::atan2(bfloat16::bfloat16(3), bfloat16::bfloat16(-0)) );

	REQUIRE(0x4049 == bfloat16::atan2(bfloat16::bfloat16(3), -std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0xC049 == bfloat16::atan2(bfloat16::bfloat16(-3), -std::numeric_limits<bfloat16::bfloat16>::infinity()) );

	REQUIRE(0x0 == bfloat16::atan2(bfloat16::bfloat16(3), std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x8000 == bfloat16::atan2(bfloat16::bfloat16(-3), std::numeric_limits<bfloat16::bfloat16>::infinity()) );

	REQUIRE(NAN == bfloat16::atan2(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), bfloat16::bfloat16(3)) );
	REQUIRE(NAN == bfloat16::atan2(bfloat16::bfloat16(3), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	REQUIRE(NAN == bfloat16::atan2(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//solution in rad
	//positive
	REQUIRE(0x0 == bfloat16::atan2(bfloat16::bfloat16(0.0), bfloat16::bfloat16(2.0)) ); // 0x0 = 0.0 bf16 = 0 d
	REQUIRE(0x3E49 == bfloat16::atan2(bfloat16::bfloat16(0.4), bfloat16::bfloat16(2.0)) ); // 0x3E49 = 0.1962890625 bf16
	REQUIRE(0x3EC2 == bfloat16::atan2(bfloat16::bfloat16(0.8), bfloat16::bfloat16(2.0)) ); // 0x3EC2 = 0.37890625 bf16
	REQUIRE(0x3F09 == bfloat16::atan2(bfloat16::bfloat16(1.2), bfloat16::bfloat16(2.0)) ); // 0x3F09 = 0.53515625 bf16
	REQUIRE(0x3F2C == bfloat16::atan2(bfloat16::bfloat16(1.6), bfloat16::bfloat16(2.0)) ); // 0x3F2C = 0.671875 bf16
	REQUIRE(0x3F49 == bfloat16::atan2(bfloat16::bfloat16(2.0), bfloat16::bfloat16(2.0)) ); // 0x3F49 = 0.78515625 bf16
	REQUIRE(0x3F5F == bfloat16::atan2(bfloat16::bfloat16(2.4), bfloat16::bfloat16(2.0)) ); // 0x3F5F = 0.87109375 bf16
	REQUIRE(0x3F73 == bfloat16::atan2(bfloat16::bfloat16(2.8), bfloat16::bfloat16(2.0)) ); // 0x3F73 = 0.94921875 bf16
	REQUIRE(0x3F81 == bfloat16::atan2(bfloat16::bfloat16(3.2), bfloat16::bfloat16(2.0)) ); // 0x3F81 = 1.0078125 bf16
	REQUIRE(0x3F88 == bfloat16::atan2(bfloat16::bfloat16(3.6), bfloat16::bfloat16(2.0)) ); // 0x3F88 = 1.0625 bf16
	REQUIRE(0x3F8D == bfloat16::atan2(bfloat16::bfloat16(4.0), bfloat16::bfloat16(2.0)) ); // 0x3F8D = 1.1015625 bf16


	//negative
	REQUIRE(0xBE49 == bfloat16::atan2(bfloat16::bfloat16(-0.4), bfloat16::bfloat16(2.0)) ); // 0xBE49 = -0.1962890625 bf16
	REQUIRE(0xBEC2 == bfloat16::atan2(bfloat16::bfloat16(-0.8), bfloat16::bfloat16(2.0)) ); // 0xBEC2 = -0.37890625 bf16
	REQUIRE(0xBF09 == bfloat16::atan2(bfloat16::bfloat16(-1.2), bfloat16::bfloat16(2.0)) ); // 0xBF09 = -0.53515625 bf16
	REQUIRE(0xBF2C == bfloat16::atan2(bfloat16::bfloat16(-1.6), bfloat16::bfloat16(2.0)) ); // 0xBF2C = -0.671875 bf16
	REQUIRE(0xBF49 == bfloat16::atan2(bfloat16::bfloat16(-2.0), bfloat16::bfloat16(2.0)) ); // 0xBF49 = -0.78515625 bf16
	REQUIRE(0xBF5F == bfloat16::atan2(bfloat16::bfloat16(-2.4), bfloat16::bfloat16(2.0)) ); // 0xBF5F = -0.87109375 bf16
	REQUIRE(0xBF73 == bfloat16::atan2(bfloat16::bfloat16(-2.8), bfloat16::bfloat16(2.0)) ); // 0xBF73 = -0.94921875 bf16
	REQUIRE(0xBF81 == bfloat16::atan2(bfloat16::bfloat16(-3.2), bfloat16::bfloat16(2.0)) ); // 0xBF81 = -1.0078125 bf16
	REQUIRE(0xBF88 == bfloat16::atan2(bfloat16::bfloat16(-3.6), bfloat16::bfloat16(2.0)) ); // 0xBF88 = -1.0625 bf16
	REQUIRE(0xBF8D == bfloat16::atan2(bfloat16::bfloat16(-4.0), bfloat16::bfloat16(2.0)) ); // 0xBF8D = -1.1015625 bf16

}



TEST_CASE("bfloat16_sinh", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(0x0 == bfloat16::sinh(bfloat16::bfloat16(-0)) );
	REQUIRE(0x7F80 == bfloat16::sinh(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0xFF80 == bfloat16::sinh(-std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::sinh(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//solution in rad
	//positive
	REQUIRE(0x0 == bfloat16::sinh(bfloat16::bfloat16(0.0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3E4D == bfloat16::sinh(bfloat16::bfloat16(0.2)) ); // 0x3E4D = 0.2001953125 bf16
	REQUIRE(0x3ED1 == bfloat16::sinh(bfloat16::bfloat16(0.4)) ); // 0x3ED1 = 0.408203125 bf16
	REQUIRE(0x3F22 == bfloat16::sinh(bfloat16::bfloat16(0.6)) ); // 0x3F22 = 0.6328125 bf16
	REQUIRE(0x3F62 == bfloat16::sinh(bfloat16::bfloat16(0.8)) ); // 0x3F62 = 0.8828125 bf16
	REQUIRE(0x3F96 == bfloat16::sinh(bfloat16::bfloat16(1.0)) ); // 0x3F96 = 1.171875 bf16
	REQUIRE(0x3FC0 == bfloat16::sinh(bfloat16::bfloat16(1.2)) ); // 0x3FC0 = 1.5 bf16
	REQUIRE(0x3FF3 == bfloat16::sinh(bfloat16::bfloat16(1.4)) ); // 0x3FF3 = 1.8984375 bf16
	REQUIRE(0x4017 == bfloat16::sinh(bfloat16::bfloat16(1.6)) ); // 0x4017 = 2.359375 bf16
	REQUIRE(0x403B == bfloat16::sinh(bfloat16::bfloat16(1.8)) ); // 0x403B = 2.921875 bf16
	REQUIRE(0x4068 == bfloat16::sinh(bfloat16::bfloat16(2.0)) ); // 0x4068 = 3.625 bf16


	//negative
	REQUIRE(0xBE4D == bfloat16::sinh(bfloat16::bfloat16(-0.2)) ); // 0xBE4D = -0.2001953125 bf16
	REQUIRE(0xBED1 == bfloat16::sinh(bfloat16::bfloat16(-0.4)) ); // 0xBED1 = -0.408203125 bf16
	REQUIRE(0xBF22 == bfloat16::sinh(bfloat16::bfloat16(-0.6)) ); // 0xBF22 = -0.6328125 bf16
	REQUIRE(0xBF62 == bfloat16::sinh(bfloat16::bfloat16(-0.8)) ); // 0xBF62 = -0.8828125 bf16
	REQUIRE(0xBF96 == bfloat16::sinh(bfloat16::bfloat16(-1.0)) ); // 0xBF96 = -1.171875 bf16
	REQUIRE(0xBFC0 == bfloat16::sinh(bfloat16::bfloat16(-1.2)) ); // 0xBFC0 = -1.5 bf16
	REQUIRE(0xBFF3 == bfloat16::sinh(bfloat16::bfloat16(-1.4)) ); // 0xBFF3 = -1.8984375 bf16
	REQUIRE(0xC017 == bfloat16::sinh(bfloat16::bfloat16(-1.6)) ); // 0xC017 = -2.359375 bf16
	REQUIRE(0xC03B == bfloat16::sinh(bfloat16::bfloat16(-1.8)) ); // 0xC03B = -2.921875 bf16
	REQUIRE(0xC068 == bfloat16::sinh(bfloat16::bfloat16(-2.0)) ); // 0xC068 = -3.625 bf16

}



TEST_CASE("bfloat16_cosh", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(0x7F80 == bfloat16::cosh(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x7F80 == bfloat16::cosh(-std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::cosh(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//solution in rad
 	//positive
	REQUIRE(0x3F80 == bfloat16::cosh(bfloat16::bfloat16(0.0)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x3F82 == bfloat16::cosh(bfloat16::bfloat16(0.2)) ); // 0x3F82 = 1.015625 bf16
	REQUIRE(0x3F8A == bfloat16::cosh(bfloat16::bfloat16(0.4)) ); // 0x3F8A = 1.078125 bf16
	REQUIRE(0x3F97 == bfloat16::cosh(bfloat16::bfloat16(0.6)) ); // 0x3F97 = 1.1796875 bf16
	REQUIRE(0x3FAA == bfloat16::cosh(bfloat16::bfloat16(0.8)) ); // 0x3FAA = 1.328125 bf16
	REQUIRE(0x3FC5 == bfloat16::cosh(bfloat16::bfloat16(1.0)) ); // 0x3FC5 = 1.5390625 bf16
	REQUIRE(0x3FE6 == bfloat16::cosh(bfloat16::bfloat16(1.2)) ); // 0x3FE6 = 1.796875 bf16
	REQUIRE(0x4009 == bfloat16::cosh(bfloat16::bfloat16(1.4)) ); // 0x4009 = 2.140625 bf16
	REQUIRE(0x4024 == bfloat16::cosh(bfloat16::bfloat16(1.6)) ); // 0x4024 = 2.5625 bf16
	REQUIRE(0x4046 == bfloat16::cosh(bfloat16::bfloat16(1.8)) ); // 0x4046 = 3.09375 bf16
	REQUIRE(0x4070 == bfloat16::cosh(bfloat16::bfloat16(2.0)) ); // 0x4070 = 3.75 bf16


	//negative
	REQUIRE(0x3F82 == bfloat16::cosh(bfloat16::bfloat16(-0.2)) ); // 0x3F82 = 1.015625 bf16
	REQUIRE(0x3F8A == bfloat16::cosh(bfloat16::bfloat16(-0.4)) ); // 0x3F8A = 1.078125 bf16
	REQUIRE(0x3F97 == bfloat16::cosh(bfloat16::bfloat16(-0.6)) ); // 0x3F97 = 1.1796875 bf16
	REQUIRE(0x3FAA == bfloat16::cosh(bfloat16::bfloat16(-0.8)) ); // 0x3FAA = 1.328125 bf16
	REQUIRE(0x3FC5 == bfloat16::cosh(bfloat16::bfloat16(-1.0)) ); // 0x3FC5 = 1.5390625 bf16
	REQUIRE(0x3FE6 == bfloat16::cosh(bfloat16::bfloat16(-1.2)) ); // 0x3FE6 = 1.796875 bf16
	REQUIRE(0x4009 == bfloat16::cosh(bfloat16::bfloat16(-1.4)) ); // 0x4009 = 2.140625 bf16
	REQUIRE(0x4024 == bfloat16::cosh(bfloat16::bfloat16(-1.6)) ); // 0x4024 = 2.5625 bf16
	REQUIRE(0x4046 == bfloat16::cosh(bfloat16::bfloat16(-1.8)) ); // 0x4046 = 3.09375 bf16
	REQUIRE(0x4070 == bfloat16::cosh(bfloat16::bfloat16(-2.0)) ); // 0x4070 = 3.75 bf16

}



TEST_CASE("bfloat16_tanh", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(0x3F80 == bfloat16::tanh(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0xBF80 == bfloat16::tanh(-std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::tanh(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//solution in rad
	//positive
	REQUIRE(0x0 == bfloat16::tanh(bfloat16::bfloat16(0.0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3E49 == bfloat16::tanh(bfloat16::bfloat16(0.2)) ); // 0x3E49 = 0.1962890625 bf16
	REQUIRE(0x3EC1 == bfloat16::tanh(bfloat16::bfloat16(0.4)) ); // 0x3EC1 = 0.376953125 bf16
	REQUIRE(0x3F09 == bfloat16::tanh(bfloat16::bfloat16(0.6)) ); // 0x3F09 = 0.53515625 bf16
	REQUIRE(0x3F29 == bfloat16::tanh(bfloat16::bfloat16(0.8)) ); // 0x3F29 = 0.66015625 bf16
	REQUIRE(0x3F42 == bfloat16::tanh(bfloat16::bfloat16(1.0)) ); // 0x3F42 = 0.7578125 bf16
	REQUIRE(0x3F55 == bfloat16::tanh(bfloat16::bfloat16(1.2)) ); // 0x3F55 = 0.83203125 bf16
	REQUIRE(0x3F62 == bfloat16::tanh(bfloat16::bfloat16(1.4)) ); // 0x3F62 = 0.8828125 bf16
	REQUIRE(0x3F6B == bfloat16::tanh(bfloat16::bfloat16(1.6)) ); // 0x3F6B = 0.91796875 bf16
	REQUIRE(0x3F72 == bfloat16::tanh(bfloat16::bfloat16(1.8)) ); // 0x3F72 = 0.9453125 bf16
	REQUIRE(0x3F76 == bfloat16::tanh(bfloat16::bfloat16(2.0)) ); // 0x3F76 = 0.9609375 bf16


	//negative
	REQUIRE(0xBE49 == bfloat16::tanh(bfloat16::bfloat16(-0.2)) ); // 0xBE49 = -0.1962890625 bf16
	REQUIRE(0xBEC1 == bfloat16::tanh(bfloat16::bfloat16(-0.4)) ); // 0xBEC1 = -0.376953125 bf16
	REQUIRE(0xBF09 == bfloat16::tanh(bfloat16::bfloat16(-0.6)) ); // 0xBF09 = -0.53515625 bf16
	REQUIRE(0xBF29 == bfloat16::tanh(bfloat16::bfloat16(-0.8)) ); // 0xBF29 = -0.66015625 bf16
	REQUIRE(0xBF42 == bfloat16::tanh(bfloat16::bfloat16(-1.0)) ); // 0xBF42 = -0.7578125 bf16
	REQUIRE(0xBF55 == bfloat16::tanh(bfloat16::bfloat16(-1.2)) ); // 0xBF55 = -0.83203125 bf16
	REQUIRE(0xBF62 == bfloat16::tanh(bfloat16::bfloat16(-1.4)) ); // 0xBF62 = -0.8828125 bf16
	REQUIRE(0xBF6B == bfloat16::tanh(bfloat16::bfloat16(-1.6)) ); // 0xBF6B = -0.91796875 bf16
	REQUIRE(0xBF72 == bfloat16::tanh(bfloat16::bfloat16(-1.8)) ); // 0xBF72 = -0.9453125 bf16
	REQUIRE(0xBF76 == bfloat16::tanh(bfloat16::bfloat16(-2.0)) ); // 0xBF76 = -0.9609375 bf16

}



TEST_CASE("bfloat16_asinh", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(0x0 == bfloat16::asinh(bfloat16::bfloat16(-0)) );
	REQUIRE(0x7F80 == bfloat16::asinh(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0xFF80 == bfloat16::asinh(-std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::asinh(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//solution in rad
	//positive
	REQUIRE(0x0 == bfloat16::asinh(bfloat16::bfloat16(0.0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3E4A == bfloat16::asinh(bfloat16::bfloat16(0.2)) ); // 0x3E4A = 0.197265625 bf16
	REQUIRE(0x3EC6 == bfloat16::asinh(bfloat16::bfloat16(0.4)) ); // 0x3EC6 = 0.38671875 bf16
	REQUIRE(0x3F11 == bfloat16::asinh(bfloat16::bfloat16(0.6)) ); // 0x3F11 = 0.56640625 bf16
	REQUIRE(0x3F3A == bfloat16::asinh(bfloat16::bfloat16(0.8)) ); // 0x3F3A = 0.7265625 bf16
	REQUIRE(0x3F61 == bfloat16::asinh(bfloat16::bfloat16(1.0)) ); // 0x3F61 = 0.87890625 bf16
	REQUIRE(0x3F81 == bfloat16::asinh(bfloat16::bfloat16(1.2)) ); // 0x3F81 = 1.0078125 bf16
	REQUIRE(0x3F91 == bfloat16::asinh(bfloat16::bfloat16(1.4)) ); // 0x3F91 = 1.1328125 bf16
	REQUIRE(0x3F9F == bfloat16::asinh(bfloat16::bfloat16(1.6)) ); // 0x3F9F = 1.2421875 bf16
	REQUIRE(0x3FAC == bfloat16::asinh(bfloat16::bfloat16(1.8)) ); // 0x3FAC = 1.34375 bf16
	REQUIRE(0x3FB8 == bfloat16::asinh(bfloat16::bfloat16(2.0)) ); // 0x3FB8 = 1.4375 bf16


	//negative
	REQUIRE(0xBE4A == bfloat16::asinh(bfloat16::bfloat16(-0.2)) ); // 0xBE4A = -0.197265625 bf16
	REQUIRE(0xBEC6 == bfloat16::asinh(bfloat16::bfloat16(-0.4)) ); // 0xBEC6 = -0.38671875 bf16
	REQUIRE(0xBF11 == bfloat16::asinh(bfloat16::bfloat16(-0.6)) ); // 0xBF11 = -0.56640625 bf16
	REQUIRE(0xBF3A == bfloat16::asinh(bfloat16::bfloat16(-0.8)) ); // 0xBF3A = -0.7265625 bf16
	REQUIRE(0xBF61 == bfloat16::asinh(bfloat16::bfloat16(-1.0)) ); // 0xBF61 = -0.87890625 bf16
	REQUIRE(0xBF81 == bfloat16::asinh(bfloat16::bfloat16(-1.2)) ); // 0xBF81 = -1.0078125 bf16
	REQUIRE(0xBF91 == bfloat16::asinh(bfloat16::bfloat16(-1.4)) ); // 0xBF91 = -1.1328125 bf16
	REQUIRE(0xBF9F == bfloat16::asinh(bfloat16::bfloat16(-1.6)) ); // 0xBF9F = -1.2421875 bf16
	REQUIRE(0xBFAC == bfloat16::asinh(bfloat16::bfloat16(-1.8)) ); // 0xBFAC = -1.34375 bf16
	REQUIRE(0xBFB8 == bfloat16::asinh(bfloat16::bfloat16(-2.0)) ); // 0xBFB8 = -1.4375 bf16

}



TEST_CASE("bfloat16_acosh", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(0x7F80 == bfloat16::acosh(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::acosh(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//solution in rad
	//positive
	REQUIRE(0xffc0 == bfloat16::acosh(bfloat16::bfloat16(0.0)) ); // 0xffc0 = NaN bf16
	REQUIRE(0xffc0 == bfloat16::acosh(bfloat16::bfloat16(0.5)) ); // 0xffc0 = NaN bf16
	REQUIRE(0x0 == bfloat16::acosh(bfloat16::bfloat16(1.0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3FA8 == bfloat16::acosh(bfloat16::bfloat16(2.0)) ); // 0x3FA8 = 1.3125 bf16
	REQUIRE(0x3FE1 == bfloat16::acosh(bfloat16::bfloat16(3.0)) ); // 0x3FE1 = 1.7578125 bf16
	REQUIRE(0x4004 == bfloat16::acosh(bfloat16::bfloat16(4.0)) ); // 0x4004 = 2.0625 bf16
	REQUIRE(0x4012 == bfloat16::acosh(bfloat16::bfloat16(5.0)) ); // 0x4012 = 2.28125 bf16
	REQUIRE(0x401E == bfloat16::acosh(bfloat16::bfloat16(6.0)) ); // 0x401E = 2.46875 bf16


	//negative
	REQUIRE(0xffc0 == bfloat16::acosh(bfloat16::bfloat16(-0.5)) ); // 0xffc0 = NaN bf16
	REQUIRE(0xffc0 == bfloat16::acosh(bfloat16::bfloat16(-1.0)) ); // 0xffc0 = NaN bf16
	REQUIRE(0xffc0 == bfloat16::acosh(bfloat16::bfloat16(-5.0)) ); // 0xffc0 = NaN bf16

}



TEST_CASE("bfloat16_atanh", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(std::numeric_limits<bfloat16::bfloat16>::infinity() == bfloat16::atanh(bfloat16::bfloat16(1)) );
	REQUIRE(-std::numeric_limits<bfloat16::bfloat16>::infinity() == bfloat16::atanh(bfloat16::bfloat16(-1)) );
	REQUIRE(NAN == bfloat16::atanh(bfloat16::bfloat16(3)) );
	REQUIRE(NAN == bfloat16::atanh(bfloat16::bfloat16(-3)) );
	REQUIRE(NAN == bfloat16::atanh(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//solution in rad
	//positive
	REQUIRE(0x0 == bfloat16::atanh(bfloat16::bfloat16(0.0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3DCC == bfloat16::atanh(bfloat16::bfloat16(0.1)) ); // 0x3DCC = 0.099609375 bf16
	REQUIRE(0x3E4E == bfloat16::atanh(bfloat16::bfloat16(0.2)) ); // 0x3E4E = 0.201171875 bf16
	REQUIRE(0x3E9D == bfloat16::atanh(bfloat16::bfloat16(0.3)) ); // 0x3E9D = 0.306640625 bf16
	REQUIRE(0x3ED7 == bfloat16::atanh(bfloat16::bfloat16(0.4)) ); // 0x3ED7 = 0.419921875 bf16
	REQUIRE(0x3F0C == bfloat16::atanh(bfloat16::bfloat16(0.5)) ); // 0x3F0C = 0.546875 bf16
	REQUIRE(0x3F30 == bfloat16::atanh(bfloat16::bfloat16(0.6)) ); // 0x3F30 = 0.6875 bf16
	REQUIRE(0x3F5D == bfloat16::atanh(bfloat16::bfloat16(0.7)) ); // 0x3F5D = 0.86328125 bf16
	REQUIRE(0x3F8B == bfloat16::atanh(bfloat16::bfloat16(0.8)) ); // 0x3F8B = 1.0859375 bf16
	REQUIRE(0x3FBB == bfloat16::atanh(bfloat16::bfloat16(0.9)) ); // 0x3FBB = 1.4609375 bf16


	//negative
	REQUIRE(0xBDCC == bfloat16::atanh(bfloat16::bfloat16(-0.1)) ); // 0xBDCC = -0.099609375 bf16
	REQUIRE(0xBE4E == bfloat16::atanh(bfloat16::bfloat16(-0.2)) ); // 0xBE4E = -0.201171875 bf16
	REQUIRE(0xBE9D == bfloat16::atanh(bfloat16::bfloat16(-0.3)) ); // 0xBE9D = -0.306640625 bf16
	REQUIRE(0xBED7 == bfloat16::atanh(bfloat16::bfloat16(-0.4)) ); // 0xBED7 = -0.419921875 bf16
	REQUIRE(0xBF0C == bfloat16::atanh(bfloat16::bfloat16(-0.5)) ); // 0xBF0C = -0.546875 bf16
	REQUIRE(0xBF30 == bfloat16::atanh(bfloat16::bfloat16(-0.6)) ); // 0xBF30 = -0.6875 bf16
	REQUIRE(0xBF5D == bfloat16::atanh(bfloat16::bfloat16(-0.7)) ); // 0xBF5D = -0.86328125 bf16
	REQUIRE(0xBF8B == bfloat16::atanh(bfloat16::bfloat16(-0.8)) ); // 0xBF8B = -1.0859375 bf16
	REQUIRE(0xBFBB == bfloat16::atanh(bfloat16::bfloat16(-0.9)) ); // 0xBFBB = -1.4609375 bf16

}



TEST_CASE("bfloat16_erf", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(0x3F80 == bfloat16::erf(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0xBF80 == bfloat16::erf(-std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::erf(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive
	REQUIRE(0x0 == bfloat16::erf(bfloat16::bfloat16(0.0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3E63 == bfloat16::erf(bfloat16::bfloat16(0.2)) ); // 0x3E63 = 0.2216796875 bf16
	REQUIRE(0x3EDA == bfloat16::erf(bfloat16::bfloat16(0.4)) ); // 0x3EDA = 0.42578125 bf16
	REQUIRE(0x3F1A == bfloat16::erf(bfloat16::bfloat16(0.6)) ); // 0x3F1A = 0.6015625 bf16
	REQUIRE(0x3F3D == bfloat16::erf(bfloat16::bfloat16(0.8)) ); // 0x3F3D = 0.73828125 bf16
	REQUIRE(0x3F57 == bfloat16::erf(bfloat16::bfloat16(1.0)) ); // 0x3F57 = 0.83984375 bf16
	REQUIRE(0x3F68 == bfloat16::erf(bfloat16::bfloat16(1.2)) ); // 0x3F68 = 0.90625 bf16
	REQUIRE(0x3F73 == bfloat16::erf(bfloat16::bfloat16(1.4)) ); // 0x3F73 = 0.94921875 bf16
	REQUIRE(0x3F79 == bfloat16::erf(bfloat16::bfloat16(1.6)) ); // 0x3F79 = 0.97265625 bf16
	REQUIRE(0x3F7D == bfloat16::erf(bfloat16::bfloat16(1.8)) ); // 0x3F7D = 0.98828125 bf16
	REQUIRE(0x3F7E == bfloat16::erf(bfloat16::bfloat16(2.0)) ); // 0x3F7E = 0.9921875 bf16


	//negative
	REQUIRE(0xBE63 == bfloat16::erf(bfloat16::bfloat16(-0.2)) ); // 0xBE63 = -0.2216796875 bf16
	REQUIRE(0xBEDA == bfloat16::erf(bfloat16::bfloat16(-0.4)) ); // 0xBEDA = -0.42578125 bf16
	REQUIRE(0xBF1A == bfloat16::erf(bfloat16::bfloat16(-0.6)) ); // 0xBF1A = -0.6015625 bf16
	REQUIRE(0xBF3D == bfloat16::erf(bfloat16::bfloat16(-0.8)) ); // 0xBF3D = -0.73828125 bf16
	REQUIRE(0xBF57 == bfloat16::erf(bfloat16::bfloat16(-1.0)) ); // 0xBF57 = -0.83984375 bf16
	REQUIRE(0xBF68 == bfloat16::erf(bfloat16::bfloat16(-1.2)) ); // 0xBF68 = -0.90625 bf16
	REQUIRE(0xBF73 == bfloat16::erf(bfloat16::bfloat16(-1.4)) ); // 0xBF73 = -0.94921875 bf16
	REQUIRE(0xBF79 == bfloat16::erf(bfloat16::bfloat16(-1.6)) ); // 0xBF79 = -0.97265625 bf16
	REQUIRE(0xBF7D == bfloat16::erf(bfloat16::bfloat16(-1.8)) ); // 0xBF7D = -0.98828125 bf16
	REQUIRE(0xBF7E == bfloat16::erf(bfloat16::bfloat16(-2.0)) ); // 0xBF7E = -0.9921875 bf16
}



TEST_CASE("bfloat16_erfc", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(0x0 == bfloat16::erfc(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x4000 == bfloat16::erfc(-std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::erfc(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );

	
	//positive
	REQUIRE(0x3F80 == bfloat16::erfc(bfloat16::bfloat16(0.0)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x3F47 == bfloat16::erfc(bfloat16::bfloat16(0.2)) ); // 0x3F47 = 0.77734375 bf16
	REQUIRE(0x3F12 == bfloat16::erfc(bfloat16::bfloat16(0.4)) ); // 0x3F12 = 0.5703125 bf16
	REQUIRE(0x3ECB == bfloat16::erfc(bfloat16::bfloat16(0.6)) ); // 0x3ECB = 0.396484375 bf16
	REQUIRE(0x3E84 == bfloat16::erfc(bfloat16::bfloat16(0.8)) ); // 0x3E84 = 0.2578125 bf16
	REQUIRE(0x3E21 == bfloat16::erfc(bfloat16::bfloat16(1.0)) ); // 0x3E21 = 0.1572265625 bf16
	REQUIRE(0x3DBA == bfloat16::erfc(bfloat16::bfloat16(1.2)) ); // 0x3DBA = 0.0908203125 bf16
	REQUIRE(0x3D44 == bfloat16::erfc(bfloat16::bfloat16(1.4)) ); // 0x3D44 = 0.0478515625 bf16
	REQUIRE(0x3CC6 == bfloat16::erfc(bfloat16::bfloat16(1.6)) ); // 0x3CC6 = 0.024169921875 bf16
	REQUIRE(0x3C35 == bfloat16::erfc(bfloat16::bfloat16(1.8)) ); // 0x3C35 = 0.01104736328125 bf16
	REQUIRE(0x3B99 == bfloat16::erfc(bfloat16::bfloat16(2.0)) ); // 0x3B99 = 0.004669189453125 bf16


	//negative
	REQUIRE(0x3F9C == bfloat16::erfc(bfloat16::bfloat16(-0.2)) ); // 0x3F9C = 1.21875 bf16
	REQUIRE(0x3FB6 == bfloat16::erfc(bfloat16::bfloat16(-0.4)) ); // 0x3FB6 = 1.421875 bf16
	REQUIRE(0x3FCD == bfloat16::erfc(bfloat16::bfloat16(-0.6)) ); // 0x3FCD = 1.6015625 bf16
	REQUIRE(0x3FDE == bfloat16::erfc(bfloat16::bfloat16(-0.8)) ); // 0x3FDE = 1.734375 bf16
	REQUIRE(0x3FEB == bfloat16::erfc(bfloat16::bfloat16(-1.0)) ); // 0x3FEB = 1.8359375 bf16
	REQUIRE(0x3FF4 == bfloat16::erfc(bfloat16::bfloat16(-1.2)) ); // 0x3FF4 = 1.90625 bf16
	REQUIRE(0x3FF9 == bfloat16::erfc(bfloat16::bfloat16(-1.4)) ); // 0x3FF9 = 1.9453125 bf16
	REQUIRE(0x3FFC == bfloat16::erfc(bfloat16::bfloat16(-1.6)) ); // 0x3FFC = 1.96875 bf16
	REQUIRE(0x3FFE == bfloat16::erfc(bfloat16::bfloat16(-1.8)) ); // 0x3FFE = 1.984375 bf16
	REQUIRE(0x3FFF == bfloat16::erfc(bfloat16::bfloat16(-2.0)) ); // 0x3FFF = 1.9921875 bf16

}



TEST_CASE("bfloat16_lgamma", "[float16]"){

	//calculated with the temporary solution
	
	//ERRhandling / numeric Limits
	REQUIRE(0x7F80 == bfloat16::lgamma(bfloat16::bfloat16(-0)) );
	REQUIRE(0x7F80 == bfloat16::lgamma(bfloat16::bfloat16(0)) );
	REQUIRE(0x7F80 == bfloat16::lgamma(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x7F80 == bfloat16::lgamma(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::lgamma(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive
	REQUIRE(0x0 == bfloat16::lgamma(bfloat16::bfloat16(1.0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x0 == bfloat16::lgamma(bfloat16::bfloat16(2.0)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3F31 == bfloat16::lgamma(bfloat16::bfloat16(3.0)) ); // 0x3F31 = 0.69140625 bf16
	REQUIRE(0x3FE5 == bfloat16::lgamma(bfloat16::bfloat16(4.0)) ); // 0x3FE5 = 1.7890625 bf16
	REQUIRE(0x404B == bfloat16::lgamma(bfloat16::bfloat16(5.0)) ); // 0x404B = 3.171875 bf16
	REQUIRE(0x4099 == bfloat16::lgamma(bfloat16::bfloat16(6.0)) ); // 0x4099 = 4.78125 bf16
	REQUIRE(0x40D2 == bfloat16::lgamma(bfloat16::bfloat16(7.0)) ); // 0x40D2 = 6.5625 bf16
	REQUIRE(0x4108 == bfloat16::lgamma(bfloat16::bfloat16(8.0)) ); // 0x4108 = 8.5 bf16
	REQUIRE(0x4129 == bfloat16::lgamma(bfloat16::bfloat16(9.0)) ); // 0x4129 = 10.5625 bf16
	REQUIRE(0x414C == bfloat16::lgamma(bfloat16::bfloat16(10.0)) ); // 0x414C = 12.75 bf16
	REQUIRE(0x4171 == bfloat16::lgamma(bfloat16::bfloat16(11.0)) ); // 0x4171 = 15.0625 bf16


	//negative
	REQUIRE(0x7F80 == bfloat16::lgamma(bfloat16::bfloat16(-1.0)) ); // 0x7F80 = inf bf16
	REQUIRE(0x7F80 == bfloat16::lgamma(bfloat16::bfloat16(-2.0)) ); // 0x7F80 = inf bf16

}



TEST_CASE("bfloat16_tgamma", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(0x7F80 == bfloat16::tgamma(bfloat16::bfloat16(-0)) );
	REQUIRE(0x7F80 == bfloat16::tgamma(bfloat16::bfloat16(0)) );
	REQUIRE(0x7F80 == bfloat16::tgamma(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::tgamma(-std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::tgamma(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	

	//positive
	REQUIRE(0x3F80 == bfloat16::tgamma(bfloat16::bfloat16(1.0)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x3F80 == bfloat16::tgamma(bfloat16::bfloat16(2.0)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x4000 == bfloat16::tgamma(bfloat16::bfloat16(3.0)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x40C0 == bfloat16::tgamma(bfloat16::bfloat16(4.0)) ); // 0x40C0 = 6.0 bf16
	REQUIRE(0x41C0 == bfloat16::tgamma(bfloat16::bfloat16(5.0)) ); // 0x41C0 = 24.0 bf16
	REQUIRE(0x42F0 == bfloat16::tgamma(bfloat16::bfloat16(6.0)) ); // 0x42F0 = 120.0 bf16
	REQUIRE(0x4434 == bfloat16::tgamma(bfloat16::bfloat16(7.0)) ); // 0x4434 = 720.0 bf16
	REQUIRE(0x459D == bfloat16::tgamma(bfloat16::bfloat16(8.0)) ); // 0x459D = 5024.0 bf16
	REQUIRE(0x471D == bfloat16::tgamma(bfloat16::bfloat16(9.0)) ); // 0x471D = 40192.0 bf16
	REQUIRE(0x7F80 == bfloat16::tgamma(bfloat16::bfloat16(10.0)) ); // 0x7F80 = inf bf16
	REQUIRE(0x7F80 == bfloat16::tgamma(bfloat16::bfloat16(11.0)) ); // 0x7F80 = inf bf16


	//negative
	REQUIRE(0xFFC0 == bfloat16::tgamma(bfloat16::bfloat16(-1.0)) ); // 0xFFC0 = NaN bf16
	REQUIRE(0xFFC0 == bfloat16::tgamma(bfloat16::bfloat16(-2.0)) ); // 0xFFC0 = NaN bf16

}



TEST_CASE("bfloat16_ceil", "[floa16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(0x7F80 == bfloat16::ceil(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x7F80 == bfloat16::ceil(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x0 == bfloat16::ceil(bfloat16::bfloat16(0)) );
	REQUIRE(0x0 == bfloat16::ceil(bfloat16::bfloat16(-0)) );
	REQUIRE(NAN == bfloat16::ceil(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive
	REQUIRE(0x3F80 == bfloat16::ceil(bfloat16::bfloat16(0.3)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x3F80 == bfloat16::ceil(bfloat16::bfloat16(0.5)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x3F80 == bfloat16::ceil(bfloat16::bfloat16(0.8)) ); // 0x3F80 = 1.0 bf16

	REQUIRE(0x40E0 == bfloat16::ceil(bfloat16::bfloat16(6.3)) ); // 0x40E0 = 7.0 bf16
	REQUIRE(0x40E0 == bfloat16::ceil(bfloat16::bfloat16(6.5)) ); // 0x40E0 = 7.0 bf16
	REQUIRE(0x40E0 == bfloat16::ceil(bfloat16::bfloat16(6.8)) ); // 0x40E0 = 7.0 bf16


	//negative
	REQUIRE(0x8000 == bfloat16::ceil(bfloat16::bfloat16(-0.3)) ); // 0x8000 = -0.0 bf16
	REQUIRE(0x8000 == bfloat16::ceil(bfloat16::bfloat16(-0.5)) ); // 0x8000 = -0.0 bf16
	REQUIRE(0x8000 == bfloat16::ceil(bfloat16::bfloat16(-0.8)) ); // 0x8000 = -0.0 bf16

	REQUIRE(0xC0C0 == bfloat16::ceil(bfloat16::bfloat16(-6.3)) ); // 0xC0C0 = -6.0 bf16
	REQUIRE(0xC0C0 == bfloat16::ceil(bfloat16::bfloat16(-6.5)) ); // 0xC0C0 = -6.0 bf16
	REQUIRE(0xC0C0 == bfloat16::ceil(bfloat16::bfloat16(-6.8)) ); // 0xC0C0 = -6.0 bf16

}



TEST_CASE("bfloat16_floor", "[floa16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(0x7F80 == bfloat16::floor(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x7F80 == bfloat16::floor(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x0 == bfloat16::floor(bfloat16::bfloat16(0)) );
	REQUIRE(0x0 == bfloat16::floor(bfloat16::bfloat16(-0)) );
	REQUIRE(NAN == bfloat16::floor(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive
	REQUIRE(0x0 == bfloat16::floor(bfloat16::bfloat16(0.3)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x0 == bfloat16::floor(bfloat16::bfloat16(0.5)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x0 == bfloat16::floor(bfloat16::bfloat16(0.8)) ); // 0x0 = 0.0 bf16

	REQUIRE(0x40C0 == bfloat16::floor(bfloat16::bfloat16(6.3)) ); // 0x40C0 = 6.0 bf16
	REQUIRE(0x40C0 == bfloat16::floor(bfloat16::bfloat16(6.5)) ); // 0x40C0 = 6.0 bf16
	REQUIRE(0x40C0 == bfloat16::floor(bfloat16::bfloat16(6.8)) ); // 0x40C0 = 6.0 bf16


	//negative
	REQUIRE(0xBF80 == bfloat16::floor(bfloat16::bfloat16(-0.3)) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0xBF80 == bfloat16::floor(bfloat16::bfloat16(-0.5)) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0xBF80 == bfloat16::floor(bfloat16::bfloat16(-0.8)) ); // 0xBF80 = -1.0 bf16

	REQUIRE(0xC0E0 == bfloat16::floor(bfloat16::bfloat16(-6.3)) ); // 0xC0E0 = -7.0 bf16
	REQUIRE(0xC0E0 == bfloat16::floor(bfloat16::bfloat16(-6.5)) ); // 0xC0E0 = -7.0 bf16
	REQUIRE(0xC0E0 == bfloat16::floor(bfloat16::bfloat16(-6.8)) ); // 0xC0E0 = -7.0 bf16

}



TEST_CASE("bfloat16_trunc", "[floa16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(0x7F80 == bfloat16::trunc(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x7F80 == bfloat16::trunc(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x0 == bfloat16::trunc(bfloat16::bfloat16(0)) );
	REQUIRE(0x0 == bfloat16::trunc(bfloat16::bfloat16(-0)) );
	REQUIRE(NAN == bfloat16::trunc(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive
	REQUIRE(0x0 == bfloat16::trunc(bfloat16::bfloat16(0.3)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x0 == bfloat16::trunc(bfloat16::bfloat16(0.5)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x0 == bfloat16::trunc(bfloat16::bfloat16(0.8)) ); // 0x0 = 0.0 bf16

	REQUIRE(0x0 == bfloat16::trunc(bfloat16::bfloat16(6.3)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x0 == bfloat16::trunc(bfloat16::bfloat16(6.5)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x0 == bfloat16::trunc(bfloat16::bfloat16(6.8)) ); // 0x0 = 0.0 bf16


	//negative
	REQUIRE(0x8000 == bfloat16::trunc(bfloat16::bfloat16(-0.3)) ); // 0x8000 = -0.0 bf16
	REQUIRE(0x8000 == bfloat16::trunc(bfloat16::bfloat16(-0.5)) ); // 0x8000 = -0.0 bf16
	REQUIRE(0x8000 == bfloat16::trunc(bfloat16::bfloat16(-0.8)) ); // 0x8000 = -0.0 bf16

	REQUIRE(0x0 == bfloat16::trunc(bfloat16::bfloat16(-6.3)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x0 == bfloat16::trunc(bfloat16::bfloat16(-6.5)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x0 == bfloat16::trunc(bfloat16::bfloat16(-6.8)) ); // 0x0 = 0.0 bf16

}



TEST_CASE("bfloat16_round", "[float16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(0x7F80 == bfloat16::round(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x7F80 == bfloat16::round(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x0 == bfloat16::round(bfloat16::bfloat16(0)) );
	REQUIRE(0x0 == bfloat16::round(bfloat16::bfloat16(-0)) );
	REQUIRE(NAN == bfloat16::round(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive
	REQUIRE(0x0 == bfloat16::round(bfloat16::bfloat16(0.3)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3F80 == bfloat16::round(bfloat16::bfloat16(0.5)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x3F80 == bfloat16::round(bfloat16::bfloat16(0.8)) ); // 0x3F80 = 1.0 bf16

	REQUIRE(0x40C0 == bfloat16::round(bfloat16::bfloat16(6.3)) ); // 0x40C0 = 6.0 bf16
	REQUIRE(0x40C0 == bfloat16::round(bfloat16::bfloat16(6.5)) ); // 0x40C0 = 6.0 bf16
	REQUIRE(0x40E0 == bfloat16::round(bfloat16::bfloat16(6.8)) ); // 0x40E0 = 7.0 bf16


	//negative
	REQUIRE(0x8000 == bfloat16::round(bfloat16::bfloat16(-0.3)) ); // 0x8000 = -0.0 bf16
	REQUIRE(0xBF80 == bfloat16::round(bfloat16::bfloat16(-0.5)) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0xBF80 == bfloat16::round(bfloat16::bfloat16(-0.8)) ); // 0xBF80 = -1.0 bf16

	REQUIRE(0xC0C0 == bfloat16::round(bfloat16::bfloat16(-6.3)) ); // 0xC0C0 = -6.0 bf16
	REQUIRE(0xC0C0 == bfloat16::round(bfloat16::bfloat16(-6.5)) ); // 0xC0C0 = -6.0 bf16
	REQUIRE(0xC0E0 == bfloat16::round(bfloat16::bfloat16(-6.8)) ); // 0xC0E0 = -7.0 bf16

}



TEST_CASE("bfloat16_lround", "[float16]"){

	//works

	//positive
	REQUIRE(0 == bfloat16::lround(bfloat16::bfloat16(0.3)) );
	REQUIRE(1 == bfloat16::lround(bfloat16::bfloat16(0.5)) );
	REQUIRE(1 == bfloat16::lround(bfloat16::bfloat16(0.8)) );

	REQUIRE(6 == bfloat16::lround(bfloat16::bfloat16(6.3)) );
	REQUIRE(7 == bfloat16::lround(bfloat16::bfloat16(6.5)) );
	REQUIRE(7 == bfloat16::lround(bfloat16::bfloat16(6.8)) );


	//negative
	REQUIRE(0 == bfloat16::lround(bfloat16::bfloat16(-0.3)) );
	REQUIRE(-1 == bfloat16::lround(bfloat16::bfloat16(-0.5)) );
	REQUIRE(-1 == bfloat16::lround(bfloat16::bfloat16(-0.8)) );

	REQUIRE(-6 == bfloat16::lround(bfloat16::bfloat16(-6.3)) );
	REQUIRE(-7 == bfloat16::lround(bfloat16::bfloat16(-6.5)) );
	REQUIRE(-7 == bfloat16::lround(bfloat16::bfloat16(-6.8)) );

}



TEST_CASE("bfloat16_lrint", "[float16]"){

	//works

	//positive
	REQUIRE(0 == bfloat16::lrint(bfloat16::bfloat16(0.3)) );
	REQUIRE(0 == bfloat16::lrint(bfloat16::bfloat16(0.5)) );
	REQUIRE(1 == bfloat16::lrint(bfloat16::bfloat16(0.8)) );

	REQUIRE(6 == bfloat16::lrint(bfloat16::bfloat16(6.3)) );
	REQUIRE(6 == bfloat16::lrint(bfloat16::bfloat16(6.5)) );
	REQUIRE(7 == bfloat16::lrint(bfloat16::bfloat16(6.8)) );


	//negative
	REQUIRE(0 == bfloat16::lrint(bfloat16::bfloat16(-0.3)) );
	REQUIRE(0 == bfloat16::lrint(bfloat16::bfloat16(-0.5)) );
	REQUIRE(-1 == bfloat16::lrint(bfloat16::bfloat16(-0.8)) );

	REQUIRE(-6 == bfloat16::lrint(bfloat16::bfloat16(-6.3)) );
	REQUIRE(-6 == bfloat16::lrint(bfloat16::bfloat16(-6.5)) );
	REQUIRE(-7 == bfloat16::lrint(bfloat16::bfloat16(-6.8)) );

}



TEST_CASE("bfloat16_nearbyint", "[floa16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(0x7F80 == bfloat16::nearbyint(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x7F80 == bfloat16::nearbyint(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x0 == bfloat16::nearbyint(bfloat16::bfloat16(0)) );
	REQUIRE(0x0 == bfloat16::nearbyint(bfloat16::bfloat16(-0)) );
	REQUIRE(NAN == bfloat16::nearbyint(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive
	REQUIRE(0x0 == bfloat16::nearbyint(bfloat16::bfloat16(0.3)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x0 == bfloat16::nearbyint(bfloat16::bfloat16(0.5)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3F80 == bfloat16::nearbyint(bfloat16::bfloat16(0.8)) ); // 0x3F80 = 1.0 bf16

	REQUIRE(0x40C0 == bfloat16::nearbyint(bfloat16::bfloat16(6.3)) ); // 0x40C0 = 6.0 bf16
	REQUIRE(0x40E0 == bfloat16::nearbyint(bfloat16::bfloat16(6.5)) ); // 0x40E0 = 7.0 bf16
	REQUIRE(0x40E0 == bfloat16::nearbyint(bfloat16::bfloat16(6.8)) ); // 0x40E0 = 7.0 bf16


	//negative
	REQUIRE(0x8000 == bfloat16::nearbyint(bfloat16::bfloat16(-0.3)) ); // 0x8000 = -0.0 bf16
	REQUIRE(0x8000 == bfloat16::nearbyint(bfloat16::bfloat16(-0.5)) ); // 0x8000 = -0.0 bf16
	REQUIRE(0xBF80 == bfloat16::nearbyint(bfloat16::bfloat16(-0.8)) ); // 0xBF80 = -1.0 bf16

	REQUIRE(0xC0C0 == bfloat16::nearbyint(bfloat16::bfloat16(-6.3)) ); // 0xC0C0 = -6.0 bf16
	REQUIRE(0xC0E0 == bfloat16::nearbyint(bfloat16::bfloat16(-6.5)) ); // 0xC0E0 = -7.0 bf16
	REQUIRE(0xC0E0 == bfloat16::nearbyint(bfloat16::bfloat16(-6.8)) ); // 0xC0E0 = -7.0 bf16

}



TEST_CASE("bfloat16_llround", "[floa16]"){

	//works

	//positive
	REQUIRE(0 == bfloat16::llround(bfloat16::bfloat16(0.3)) );
	REQUIRE(1 == bfloat16::llround(bfloat16::bfloat16(0.5)) );
	REQUIRE(1 == bfloat16::llround(bfloat16::bfloat16(0.8)) );

	REQUIRE(6 == bfloat16::llround(bfloat16::bfloat16(6.3)) );
	REQUIRE(7 == bfloat16::llround(bfloat16::bfloat16(6.5)) );
	REQUIRE(7 == bfloat16::llround(bfloat16::bfloat16(6.8)) );


	//negative
	REQUIRE(0 == bfloat16::llround(bfloat16::bfloat16(-0.3)) );
	REQUIRE(-1 == bfloat16::llround(bfloat16::bfloat16(-0.5)) );
	REQUIRE(-1 == bfloat16::llround(bfloat16::bfloat16(-0.8)) );

	REQUIRE(-6 == bfloat16::llround(bfloat16::bfloat16(-6.3)) );
	REQUIRE(-7 == bfloat16::llround(bfloat16::bfloat16(-6.5)) );
	REQUIRE(-7 == bfloat16::llround(bfloat16::bfloat16(-6.8)) );

}



TEST_CASE("bfloat16_llrint", "[float16]"){

	//works

	//positive
	REQUIRE(0 == bfloat16::llrint(bfloat16::bfloat16(0.3)) );
	REQUIRE(0 == bfloat16::llrint(bfloat16::bfloat16(0.5)) );
	REQUIRE(1 == bfloat16::llrint(bfloat16::bfloat16(0.8)) );

	REQUIRE(6 == bfloat16::llrint(bfloat16::bfloat16(6.3)) );
	REQUIRE(6 == bfloat16::llrint(bfloat16::bfloat16(6.5)) );
	REQUIRE(7 == bfloat16::llrint(bfloat16::bfloat16(6.8)) );


	//negative
	REQUIRE(0 == bfloat16::llrint(bfloat16::bfloat16(-0.3)) );
	REQUIRE(0 == bfloat16::llrint(bfloat16::bfloat16(-0.5)) );
	REQUIRE(-1 == bfloat16::llrint(bfloat16::bfloat16(-0.8)) );

	REQUIRE(-6 == bfloat16::llrint(bfloat16::bfloat16(-6.3)) );
	REQUIRE(-6 == bfloat16::llrint(bfloat16::bfloat16(-6.5)) );
	REQUIRE(-7 == bfloat16::llrint(bfloat16::bfloat16(-6.8)) );

}



TEST_CASE("bfloat16_frexp", "[float16]"){

}



TEST_CASE("bfloat16_scalbln", "[float16]"){

	//calculated with the temporary solution
	
	//ERRhandling / numeric Limits
	REQUIRE(0x0 == bfloat16::scalbln(bfloat16::bfloat16(0), 1) );
	REQUIRE(0x0 == bfloat16::scalbln(bfloat16::bfloat16(-0), 1) );
	REQUIRE(0x7F80 == bfloat16::scalbln(std::numeric_limits<bfloat16::bfloat16>::infinity(), 1) );
	REQUIRE(0xFF80 == bfloat16::scalbln(-std::numeric_limits<bfloat16::bfloat16>::infinity(), 1) );
	REQUIRE(NAN == bfloat16::scalbln(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), 1) );

	
	//positive
	REQUIRE(0x3F80 == bfloat16::scalbln(bfloat16::bfloat16(1), 0) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x4000 == bfloat16::scalbln(bfloat16::bfloat16(1), 1) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x4080 == bfloat16::scalbln(bfloat16::bfloat16(1), 2) ); // 0x4080 = 4.0 bf16

	//negative
	REQUIRE(0xBF80 == bfloat16::scalbln(bfloat16::bfloat16(-1), 0) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0xC000 == bfloat16::scalbln(bfloat16::bfloat16(-1), 1) ); // 0xC000 = -2.0 bf16
	REQUIRE(0xC080 == bfloat16::scalbln(bfloat16::bfloat16(-1), 2) ); // 0xC080 = -4.0 bf16

}



TEST_CASE("bfloat16_scalbln2", "[float16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(0x0 == bfloat16::scalbn(bfloat16::bfloat16(0), 1) );
	REQUIRE(0x0 == bfloat16::scalbn(bfloat16::bfloat16(-0), 1) );
	REQUIRE(0x7F80 == bfloat16::scalbn(std::numeric_limits<bfloat16::bfloat16>::infinity(), 1) );
	REQUIRE(0xFF80 == bfloat16::scalbn(-std::numeric_limits<bfloat16::bfloat16>::infinity(), 1) );
	REQUIRE(NAN == bfloat16::scalbn(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), 1) );



	//depends on scalbln
	//positive
	REQUIRE(0x3F80 == bfloat16::scalbn(bfloat16::bfloat16(1), 0) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x4000 == bfloat16::scalbn(bfloat16::bfloat16(1), 1) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x4080 == bfloat16::scalbn(bfloat16::bfloat16(1), 2) ); // 0x4080 = 4.0 bf16

	//negative
	REQUIRE(0xBF80 == bfloat16::scalbn(bfloat16::bfloat16(-1), 0) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0xC000 == bfloat16::scalbn(bfloat16::bfloat16(-1), 1) ); // 0xC000 = -2.0 bf16
	REQUIRE(0xC080 == bfloat16::scalbn(bfloat16::bfloat16(-1), 2) ); // 0xC080 = -4.0 bf16

}



TEST_CASE("bfloat16_ldexp", "[float16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(0x0 == bfloat16::ldexp(bfloat16::bfloat16(0), 1) );
	REQUIRE(0x0 == bfloat16::ldexp(bfloat16::bfloat16(-0), 1) );
	REQUIRE(0x7F80 == bfloat16::ldexp(std::numeric_limits<bfloat16::bfloat16>::infinity(), 1) );
	REQUIRE(0xFF80 == bfloat16::ldexp(-std::numeric_limits<bfloat16::bfloat16>::infinity(), 1) );
	REQUIRE(NAN == bfloat16::ldexp(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), 1) );

	//depends on scalbln
	//positive
	REQUIRE(0x3F80 == bfloat16::ldexp(bfloat16::bfloat16(1), 0) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x4000 == bfloat16::ldexp(bfloat16::bfloat16(1), 1) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x4080 == bfloat16::ldexp(bfloat16::bfloat16(1), 2) ); // 0x4080 = 4.0 bf16

	//negative
	REQUIRE(0xBF80 == bfloat16::ldexp(bfloat16::bfloat16(-1), 0) ); // 0xBF80 = -1.0 bf16
	REQUIRE(0xC000 == bfloat16::ldexp(bfloat16::bfloat16(-1), 1) ); // 0xC000 = -2.0 bf16
	REQUIRE(0xC080 == bfloat16::ldexp(bfloat16::bfloat16(-1), 2) ); // 0xC080 = -4.0 bf16

}



TEST_CASE("bfloat16_modf", "[float16]"){

}



TEST_CASE("bfloat16_ilogb", "[float16]"){

	//works

	//postive
	REQUIRE(0 == bfloat16::ilogb(bfloat16::bfloat16(1)) );
	REQUIRE(1 == bfloat16::ilogb(bfloat16::bfloat16(2)) );
	REQUIRE(1 == bfloat16::ilogb(bfloat16::bfloat16(3)) );
	REQUIRE(2 == bfloat16::ilogb(bfloat16::bfloat16(4)) );
	REQUIRE(2 == bfloat16::ilogb(bfloat16::bfloat16(5)) );
	REQUIRE(2 == bfloat16::ilogb(bfloat16::bfloat16(6)) );
	REQUIRE(2 == bfloat16::ilogb(bfloat16::bfloat16(7)) );
	REQUIRE(3 == bfloat16::ilogb(bfloat16::bfloat16(8)) );
	REQUIRE(3 == bfloat16::ilogb(bfloat16::bfloat16(9)) );
	REQUIRE(3 == bfloat16::ilogb(bfloat16::bfloat16(10)) );

	//negative
	REQUIRE(0 == bfloat16::ilogb(bfloat16::bfloat16(-1)) );
	REQUIRE(1 == bfloat16::ilogb(bfloat16::bfloat16(-2)) );
	REQUIRE(1 == bfloat16::ilogb(bfloat16::bfloat16(-3)) );
	REQUIRE(2 == bfloat16::ilogb(bfloat16::bfloat16(-4)) );
	REQUIRE(2 == bfloat16::ilogb(bfloat16::bfloat16(-5)) );
	REQUIRE(2 == bfloat16::ilogb(bfloat16::bfloat16(-6)) );
	REQUIRE(2 == bfloat16::ilogb(bfloat16::bfloat16(-7)) );
	REQUIRE(3 == bfloat16::ilogb(bfloat16::bfloat16(-8)) );
	REQUIRE(3 == bfloat16::ilogb(bfloat16::bfloat16(-9)) );
	REQUIRE(3 == bfloat16::ilogb(bfloat16::bfloat16(-10)) );

}



TEST_CASE("bfloat16_logb", "[float16]"){

	//calculated with the temporary solution

	//ERRhandling / numeric Limits
	REQUIRE(0xFF80 == bfloat16::logb(bfloat16::bfloat16(-0)) );
	REQUIRE(0xFF80 == bfloat16::logb(bfloat16::bfloat16(0)) );
	REQUIRE(0x7F80 == bfloat16::logb(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(0x7F80 == bfloat16::logb(-std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(NAN == bfloat16::logb(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive
	REQUIRE(0x0 == bfloat16::logb(bfloat16::bfloat16(1)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3F80 == bfloat16::logb(bfloat16::bfloat16(2)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x3F80 == bfloat16::logb(bfloat16::bfloat16(3)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x4000 == bfloat16::logb(bfloat16::bfloat16(4)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x4000 == bfloat16::logb(bfloat16::bfloat16(5)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x4000 == bfloat16::logb(bfloat16::bfloat16(6)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x4000 == bfloat16::logb(bfloat16::bfloat16(7)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x4040 == bfloat16::logb(bfloat16::bfloat16(8)) ); // 0x4040 = 3.0 bf16
	REQUIRE(0x4040 == bfloat16::logb(bfloat16::bfloat16(9)) ); // 0x4040 = 3.0 bf16
	REQUIRE(0x4040 == bfloat16::logb(bfloat16::bfloat16(10)) ); // 0x4040 = 3.0 bf16


	//negative
	REQUIRE(0x0 == bfloat16::logb(bfloat16::bfloat16(-1)) ); // 0x0 = 0.0 bf16
	REQUIRE(0x3F80 == bfloat16::logb(bfloat16::bfloat16(-2)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x3F80 == bfloat16::logb(bfloat16::bfloat16(-3)) ); // 0x3F80 = 1.0 bf16
	REQUIRE(0x4000 == bfloat16::logb(bfloat16::bfloat16(-4)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x4000 == bfloat16::logb(bfloat16::bfloat16(-5)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x4000 == bfloat16::logb(bfloat16::bfloat16(-6)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x4000 == bfloat16::logb(bfloat16::bfloat16(-7)) ); // 0x4000 = 2.0 bf16
	REQUIRE(0x4040 == bfloat16::logb(bfloat16::bfloat16(-8)) ); // 0x4040 = 3.0 bf16
	REQUIRE(0x4040 == bfloat16::logb(bfloat16::bfloat16(-9)) ); // 0x4040 = 3.0 bf16
	REQUIRE(0x4040 == bfloat16::logb(bfloat16::bfloat16(-10)) ); // 0x4040 = 3.0 bf16

}



TEST_CASE("bfloat16_nextafter", "[float16]"){

}



TEST_CASE("bfloat16_nexttoward", "[float16]"){

}



TEST_CASE("bfloat16_copysign", "[float16]"){

	//works

	REQUIRE(0xC000 == bfloat16::copysign(bfloat16::bfloat16(2), bfloat16::bfloat16(-2)) ); // 0xC000 = -2.0 bf16
	REQUIRE(0x4000 == bfloat16::copysign(bfloat16::bfloat16(-2), bfloat16::bfloat16(2)) ); // 0x4000 = 2.0 bf16

}



TEST_CASE("bfloat16_fpclassify", "[float16]"){

	//works

	//ERRhandling / numeric Limits
	bfloat16::bfloat16 inf = std::numeric_limits<bfloat16::bfloat16>::infinity();	//0x7F80
	bfloat16::bfloat16 QNaN = std::numeric_limits<bfloat16::bfloat16>::quiet_NaN();	//0x7FFF
	bfloat16::bfloat16 den = std::numeric_limits<bfloat16::bfloat16>::denorm_min();	//0x0001

	REQUIRE(0 == bfloat16::fpclassify(QNaN) ); //FP_NAN = 0
	REQUIRE(1 == bfloat16::fpclassify(inf) ); //FP_INFINITE = 1
	REQUIRE(2 == bfloat16::fpclassify(bfloat16::bfloat16(0)) ); // FP_ZERO = 2
	REQUIRE(3 == bfloat16::fpclassify(den) ); // FP_SUBNORMAL = 3
	REQUIRE(4 == bfloat16::fpclassify(bfloat16::bfloat16(2)) ); // FP_NORMAL = 4
	REQUIRE(4 == bfloat16::fpclassify(bfloat16::bfloat16(5)) ); // FP_NORMAL = 4
	REQUIRE(4 == bfloat16::fpclassify(bfloat16::bfloat16(100)) ); // FP_NORMAL = 4
	REQUIRE(4 == bfloat16::fpclassify(bfloat16::bfloat16(264)) ); // FP_NORMAL = 4

}



TEST_CASE("bfloat16_isfinite", "[float16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(false == bfloat16::isfinite(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(false == bfloat16::isfinite(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	REQUIRE(true == bfloat16::isfinite(std::numeric_limits<bfloat16::bfloat16>::min()) );
	REQUIRE(true == bfloat16::isfinite(std::numeric_limits<bfloat16::bfloat16>::lowest()) );
	REQUIRE(true == bfloat16::isfinite(std::numeric_limits<bfloat16::bfloat16>::max()) );
	REQUIRE(true == bfloat16::isfinite(std::numeric_limits<bfloat16::bfloat16>::epsilon()) );
	REQUIRE(true == bfloat16::isfinite(std::numeric_limits<bfloat16::bfloat16>::round_error()) );
	REQUIRE(false == bfloat16::isfinite(std::numeric_limits<bfloat16::bfloat16>::signaling_NaN()) );
	REQUIRE(true == bfloat16::isfinite(std::numeric_limits<bfloat16::bfloat16>::denorm_min()) );


	//positive
	REQUIRE(true == bfloat16::isfinite(bfloat16::bfloat16(0.10)) );
	REQUIRE(true == bfloat16::isfinite(bfloat16::bfloat16(0.5)) );
	REQUIRE(true == bfloat16::isfinite(bfloat16::bfloat16(10)) );
	REQUIRE(true == bfloat16::isfinite(bfloat16::bfloat16(10.1)) );
	REQUIRE(true == bfloat16::isfinite(bfloat16::bfloat16(10.5)) );


	//negative
	REQUIRE(true == bfloat16::isfinite(bfloat16::bfloat16(-0.10)) );
	REQUIRE(true == bfloat16::isfinite(bfloat16::bfloat16(-0.5)) );
	REQUIRE(true == bfloat16::isfinite(bfloat16::bfloat16(-10)) );
	REQUIRE(true == bfloat16::isfinite(bfloat16::bfloat16(-10.1)) );
	REQUIRE(true == bfloat16::isfinite(bfloat16::bfloat16(-10.5)) );
	
}



TEST_CASE("bfloat16_isinf", "[float16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(true == bfloat16::isinf(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(false == bfloat16::isinf(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	REQUIRE(false == bfloat16::isinf(std::numeric_limits<bfloat16::bfloat16>::min()) );
	REQUIRE(false == bfloat16::isinf(std::numeric_limits<bfloat16::bfloat16>::lowest()) );
	REQUIRE(false == bfloat16::isinf(std::numeric_limits<bfloat16::bfloat16>::max()) );
	REQUIRE(false == bfloat16::isinf(std::numeric_limits<bfloat16::bfloat16>::epsilon()) );
	REQUIRE(false == bfloat16::isinf(std::numeric_limits<bfloat16::bfloat16>::round_error()) );
	REQUIRE(false == bfloat16::isinf(std::numeric_limits<bfloat16::bfloat16>::signaling_NaN()) );
	REQUIRE(false == bfloat16::isinf(std::numeric_limits<bfloat16::bfloat16>::denorm_min()) );

	
	//positive
	REQUIRE(false == bfloat16::isinf(bfloat16::bfloat16(0.10)) );
	REQUIRE(false == bfloat16::isinf(bfloat16::bfloat16(0.5)) );
	REQUIRE(false == bfloat16::isinf(bfloat16::bfloat16(10)) );
	REQUIRE(false == bfloat16::isinf(bfloat16::bfloat16(10.1)) );
	REQUIRE(false == bfloat16::isinf(bfloat16::bfloat16(10.5)) );


	//negative
	REQUIRE(false == bfloat16::isinf(bfloat16::bfloat16(-0.10)) );
	REQUIRE(false == bfloat16::isinf(bfloat16::bfloat16(-0.5)) );
	REQUIRE(false == bfloat16::isinf(bfloat16::bfloat16(-10)) );
	REQUIRE(false == bfloat16::isinf(bfloat16::bfloat16(-10.1)) );
	REQUIRE(false == bfloat16::isinf(bfloat16::bfloat16(-10.5)) );

}



TEST_CASE("bfloat16_isnan", "[float16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(false == bfloat16::isnan(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(true == bfloat16::isnan(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	REQUIRE(false == bfloat16::isnan(std::numeric_limits<bfloat16::bfloat16>::min()) );
	REQUIRE(false == bfloat16::isnan(std::numeric_limits<bfloat16::bfloat16>::lowest()) );
	REQUIRE(false == bfloat16::isnan(std::numeric_limits<bfloat16::bfloat16>::max()) );
	REQUIRE(false == bfloat16::isnan(std::numeric_limits<bfloat16::bfloat16>::epsilon()) );
	REQUIRE(false == bfloat16::isnan(std::numeric_limits<bfloat16::bfloat16>::round_error()) );
	REQUIRE(true == bfloat16::isnan(std::numeric_limits<bfloat16::bfloat16>::signaling_NaN()) );
	REQUIRE(false == bfloat16::isnan(std::numeric_limits<bfloat16::bfloat16>::denorm_min()) );


	//positive
	REQUIRE(false == bfloat16::isnan(bfloat16::bfloat16(0.10)) );
	REQUIRE(false == bfloat16::isnan(bfloat16::bfloat16(0.5)) );
	REQUIRE(false == bfloat16::isnan(bfloat16::bfloat16(10)) );
	REQUIRE(false == bfloat16::isnan(bfloat16::bfloat16(10.1)) );
	REQUIRE(false == bfloat16::isnan(bfloat16::bfloat16(10.5)) );


	//negative
	REQUIRE(false == bfloat16::isnan(bfloat16::bfloat16(-0.10)) );
	REQUIRE(false == bfloat16::isnan(bfloat16::bfloat16(-0.5)) );
	REQUIRE(false == bfloat16::isnan(bfloat16::bfloat16(-10)) );
	REQUIRE(false == bfloat16::isnan(bfloat16::bfloat16(-10.1)) );
	REQUIRE(false == bfloat16::isnan(bfloat16::bfloat16(-10.5)) );

}



TEST_CASE("bfloat16_isnormal", "[float16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(false == bfloat16::isnormal(std::numeric_limits<bfloat16::bfloat16>::denorm_min()) );
	REQUIRE(false == bfloat16::isnormal(bfloat16::bfloat16(0)) );
	REQUIRE(false == bfloat16::isnormal(bfloat16::bfloat16(-0)) );
	REQUIRE(false == bfloat16::isnormal(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(false == bfloat16::isnormal(-std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(false == bfloat16::isnormal(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive
	REQUIRE(true == bfloat16::isnormal(bfloat16::bfloat16(0.1)) );
	REQUIRE(true == bfloat16::isnormal(bfloat16::bfloat16(0.5)) );
	REQUIRE(true == bfloat16::isnormal(bfloat16::bfloat16(1)) );
	REQUIRE(true == bfloat16::isnormal(bfloat16::bfloat16(5)) );


	//negative
	REQUIRE(true == bfloat16::isnormal(bfloat16::bfloat16(-0.1)) );
	REQUIRE(true == bfloat16::isnormal(bfloat16::bfloat16(-0.5)) );
	REQUIRE(true == bfloat16::isnormal(bfloat16::bfloat16(-1)) );
	REQUIRE(true == bfloat16::isnormal(bfloat16::bfloat16(-5)) );

}



TEST_CASE("bfloat16_signbit", "[float16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(false == bfloat16::signbit(std::numeric_limits<bfloat16::bfloat16>::denorm_min()) );
	REQUIRE(false == bfloat16::signbit(bfloat16::bfloat16(0)) );
	REQUIRE(false == bfloat16::signbit(bfloat16::bfloat16(-0)) );
	REQUIRE(false == bfloat16::signbit(std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(true == bfloat16::signbit(-std::numeric_limits<bfloat16::bfloat16>::infinity()) );
	REQUIRE(false == bfloat16::signbit(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//positive
	REQUIRE(false == bfloat16::signbit(bfloat16::bfloat16(0.1)) );
	REQUIRE(false == bfloat16::signbit(bfloat16::bfloat16(0.5)) );
	REQUIRE(false == bfloat16::signbit(bfloat16::bfloat16(1)) );
	REQUIRE(false == bfloat16::signbit(bfloat16::bfloat16(5)) );


	//negative
	REQUIRE(true == bfloat16::signbit(bfloat16::bfloat16(-0.1)) );
	REQUIRE(true == bfloat16::signbit(bfloat16::bfloat16(-0.5)) );
	REQUIRE(true == bfloat16::signbit(bfloat16::bfloat16(-1)) );
	REQUIRE(true == bfloat16::signbit(bfloat16::bfloat16(-5)) );

}



TEST_CASE("bfloat16_isgreater", "[float16]"){

	//works

	//positive
	REQUIRE(false == bfloat16::isgreater(bfloat16::bfloat16(0.5), bfloat16::bfloat16(0.5)) );
	REQUIRE(false == bfloat16::isgreater(bfloat16::bfloat16(0.1), bfloat16::bfloat16(0.5)) );
	REQUIRE(true == bfloat16::isgreater(bfloat16::bfloat16(0.5), bfloat16::bfloat16(0.1)) );

	REQUIRE(false == bfloat16::isgreater(bfloat16::bfloat16(10), bfloat16::bfloat16(10)) );
	REQUIRE(true == bfloat16::isgreater(bfloat16::bfloat16(20), bfloat16::bfloat16(10)) );
	REQUIRE(false == bfloat16::isgreater(bfloat16::bfloat16(10), bfloat16::bfloat16(20)) );

	REQUIRE(false == bfloat16::isgreater(bfloat16::bfloat16(10.1), bfloat16::bfloat16(10.1)) );
	REQUIRE(true == bfloat16::isgreater(bfloat16::bfloat16(20.1), bfloat16::bfloat16(10.1)) );
	REQUIRE(false == bfloat16::isgreater(bfloat16::bfloat16(10.1), bfloat16::bfloat16(20.1)) );

	REQUIRE(false == bfloat16::isgreater(bfloat16::bfloat16(10.5), bfloat16::bfloat16(10.5)) );
	REQUIRE(true == bfloat16::isgreater(bfloat16::bfloat16(20.5), bfloat16::bfloat16(10.5)) );
	REQUIRE(false == bfloat16::isgreater(bfloat16::bfloat16(10.5), bfloat16::bfloat16(20.5)) );


	//negative
	REQUIRE(false == bfloat16::isgreater(bfloat16::bfloat16(-0.5), bfloat16::bfloat16(-0.5)) );
	REQUIRE(true == bfloat16::isgreater(bfloat16::bfloat16(-0.1), bfloat16::bfloat16(-0.5)) );
	REQUIRE(false == bfloat16::isgreater(bfloat16::bfloat16(-0.5), bfloat16::bfloat16(-0.1)) );

	REQUIRE(false == bfloat16::isgreater(bfloat16::bfloat16(-10), bfloat16::bfloat16(-10)) );
	REQUIRE(false == bfloat16::isgreater(bfloat16::bfloat16(-20), bfloat16::bfloat16(-10)) );
	REQUIRE(true == bfloat16::isgreater(bfloat16::bfloat16(-10), bfloat16::bfloat16(-20)) );

	REQUIRE(false == bfloat16::isgreater(bfloat16::bfloat16(-10.1), bfloat16::bfloat16(-10.1)) );
	REQUIRE(false == bfloat16::isgreater(bfloat16::bfloat16(-20.1), bfloat16::bfloat16(-10.1)) );
	REQUIRE(true == bfloat16::isgreater(bfloat16::bfloat16(-10.1), bfloat16::bfloat16(-20.1)) );

	REQUIRE(false == bfloat16::isgreater(bfloat16::bfloat16(-10.5), bfloat16::bfloat16(-10.5)) );
	REQUIRE(false == bfloat16::isgreater(bfloat16::bfloat16(-20.5), bfloat16::bfloat16(-10.5)) );
	REQUIRE(true == bfloat16::isgreater(bfloat16::bfloat16(-10.5), bfloat16::bfloat16(-20.5)) );

}



TEST_CASE("bfloat16_isgreaterequal", "[float16]"){

	//works

	//positive
	REQUIRE(true == bfloat16::isgreaterequal(bfloat16::bfloat16(0.10), bfloat16::bfloat16(0.10)) );
	REQUIRE(true == bfloat16::isgreaterequal(bfloat16::bfloat16(0.5), bfloat16::bfloat16(0.10)) );
	REQUIRE(false == bfloat16::isgreaterequal(bfloat16::bfloat16(0.10), bfloat16::bfloat16(0.5)) );

	REQUIRE(true == bfloat16::isgreaterequal(bfloat16::bfloat16(10), bfloat16::bfloat16(10)) );
	REQUIRE(true == bfloat16::isgreaterequal(bfloat16::bfloat16(20), bfloat16::bfloat16(10)) );
	REQUIRE(false == bfloat16::isgreaterequal(bfloat16::bfloat16(10), bfloat16::bfloat16(20)) );

	REQUIRE(true == bfloat16::isgreaterequal(bfloat16::bfloat16(10.1), bfloat16::bfloat16(10.1)) );
	REQUIRE(true == bfloat16::isgreaterequal(bfloat16::bfloat16(20.1), bfloat16::bfloat16(10.1)) );
	REQUIRE(false == bfloat16::isgreaterequal(bfloat16::bfloat16(10.1), bfloat16::bfloat16(20.1)) );

	REQUIRE(true == bfloat16::isgreaterequal(bfloat16::bfloat16(10.5), bfloat16::bfloat16(10.5)) );
	REQUIRE(true == bfloat16::isgreaterequal(bfloat16::bfloat16(20.5), bfloat16::bfloat16(10.5)) );
	REQUIRE(false == bfloat16::isgreaterequal(bfloat16::bfloat16(10.5), bfloat16::bfloat16(20.5)) );


	//negative
	REQUIRE(true == bfloat16::isgreaterequal(bfloat16::bfloat16(-0.10), bfloat16::bfloat16(-0.10)) );
	REQUIRE(false == bfloat16::isgreaterequal(bfloat16::bfloat16(-0.5), bfloat16::bfloat16(-0.10)) );
	REQUIRE(true == bfloat16::isgreaterequal(bfloat16::bfloat16(-0.10), bfloat16::bfloat16(-0.5)) );

	REQUIRE(true == bfloat16::isgreaterequal(bfloat16::bfloat16(-10), bfloat16::bfloat16(-10)) );
	REQUIRE(false == bfloat16::isgreaterequal(bfloat16::bfloat16(-20), bfloat16::bfloat16(-10)) );
	REQUIRE(true == bfloat16::isgreaterequal(bfloat16::bfloat16(-10), bfloat16::bfloat16(-20)) );

	REQUIRE(true == bfloat16::isgreaterequal(bfloat16::bfloat16(-10.1), bfloat16::bfloat16(-10.1)) );
	REQUIRE(false == bfloat16::isgreaterequal(bfloat16::bfloat16(-20.1), bfloat16::bfloat16(-10.1)) );
	REQUIRE(true == bfloat16::isgreaterequal(bfloat16::bfloat16(-10.1), bfloat16::bfloat16(-20.1)) );

	REQUIRE(true == bfloat16::isgreaterequal(bfloat16::bfloat16(-10.5), bfloat16::bfloat16(-10.5)) );
	REQUIRE(false == bfloat16::isgreaterequal(bfloat16::bfloat16(-20.5), bfloat16::bfloat16(-10.5)) );
	REQUIRE(true == bfloat16::isgreaterequal(bfloat16::bfloat16(-10.5), bfloat16::bfloat16(-20.5)) );

}



TEST_CASE("bfloat16_isless", "[float16]"){

	//works

	//positive
	REQUIRE(false == bfloat16::isless(bfloat16::bfloat16(0.10), bfloat16::bfloat16(0.10)) );
	REQUIRE(false == bfloat16::isless(bfloat16::bfloat16(0.5), bfloat16::bfloat16(0.10)) );
	REQUIRE(true == bfloat16::isless(bfloat16::bfloat16(0.10), bfloat16::bfloat16(0.5)) );

	REQUIRE(false == bfloat16::isless(bfloat16::bfloat16(10), bfloat16::bfloat16(10)) );
	REQUIRE(false == bfloat16::isless(bfloat16::bfloat16(20), bfloat16::bfloat16(10)) );
	REQUIRE(true == bfloat16::isless(bfloat16::bfloat16(10), bfloat16::bfloat16(20)) );

	REQUIRE(false == bfloat16::isless(bfloat16::bfloat16(10.1), bfloat16::bfloat16(10.1)) );
	REQUIRE(false == bfloat16::isless(bfloat16::bfloat16(20.1), bfloat16::bfloat16(10.1)) );
	REQUIRE(true == bfloat16::isless(bfloat16::bfloat16(10.1), bfloat16::bfloat16(20.1)) );

	REQUIRE(false == bfloat16::isless(bfloat16::bfloat16(10.5), bfloat16::bfloat16(10.5)) );
	REQUIRE(false == bfloat16::isless(bfloat16::bfloat16(20.5), bfloat16::bfloat16(10.5)) );
	REQUIRE(true == bfloat16::isless(bfloat16::bfloat16(10.5), bfloat16::bfloat16(20.5)) );


	//negative
	REQUIRE(false == bfloat16::isless(bfloat16::bfloat16(-0.10), bfloat16::bfloat16(-0.10)) );
	REQUIRE(true == bfloat16::isless(bfloat16::bfloat16(-0.5), bfloat16::bfloat16(-0.10)) );
	REQUIRE(false == bfloat16::isless(bfloat16::bfloat16(-0.10), bfloat16::bfloat16(-0.5)) );

	REQUIRE(false == bfloat16::isless(bfloat16::bfloat16(-10), bfloat16::bfloat16(-10)) );
	REQUIRE(true == bfloat16::isless(bfloat16::bfloat16(-20), bfloat16::bfloat16(-10)) );
	REQUIRE(false == bfloat16::isless(bfloat16::bfloat16(-10), bfloat16::bfloat16(-20)) );

	REQUIRE(false == bfloat16::isless(bfloat16::bfloat16(-10.1), bfloat16::bfloat16(-10.1)) );
	REQUIRE(true == bfloat16::isless(bfloat16::bfloat16(-20.1), bfloat16::bfloat16(-10.1)) );
	REQUIRE(false == bfloat16::isless(bfloat16::bfloat16(-10.1), bfloat16::bfloat16(-20.1)) );

	REQUIRE(false == bfloat16::isless(bfloat16::bfloat16(-10.5), bfloat16::bfloat16(-10.5)) );
	REQUIRE(true == bfloat16::isless(bfloat16::bfloat16(-20.5), bfloat16::bfloat16(-10.5)) );
	REQUIRE(false == bfloat16::isless(bfloat16::bfloat16(-10.5), bfloat16::bfloat16(-20.5)) );

}



TEST_CASE("bfloat16_islessequal", "[float16]"){

	//works

	//positive
	REQUIRE(true == bfloat16::islessequal(bfloat16::bfloat16(0.10), bfloat16::bfloat16(0.10)) );
	REQUIRE(false == bfloat16::islessequal(bfloat16::bfloat16(0.5), bfloat16::bfloat16(0.10)) );
	REQUIRE(true == bfloat16::islessequal(bfloat16::bfloat16(0.10), bfloat16::bfloat16(0.5)) );

	REQUIRE(true == bfloat16::islessequal(bfloat16::bfloat16(10), bfloat16::bfloat16(10)) );
	REQUIRE(false == bfloat16::islessequal(bfloat16::bfloat16(20), bfloat16::bfloat16(10)) );
	REQUIRE(true == bfloat16::islessequal(bfloat16::bfloat16(10), bfloat16::bfloat16(20)) );

	REQUIRE(true == bfloat16::islessequal(bfloat16::bfloat16(10.1), bfloat16::bfloat16(10.1)) );
	REQUIRE(false == bfloat16::islessequal(bfloat16::bfloat16(20.1), bfloat16::bfloat16(10.1)) );
	REQUIRE(true == bfloat16::islessequal(bfloat16::bfloat16(10.1), bfloat16::bfloat16(20.1)) );

	REQUIRE(true == bfloat16::islessequal(bfloat16::bfloat16(10.5), bfloat16::bfloat16(10.5)) );
	REQUIRE(false == bfloat16::islessequal(bfloat16::bfloat16(20.5), bfloat16::bfloat16(10.5)) );
	REQUIRE(true == bfloat16::islessequal(bfloat16::bfloat16(10.5), bfloat16::bfloat16(20.5)) );

	
	//negative
	REQUIRE(true == bfloat16::islessequal(bfloat16::bfloat16(-0.10), bfloat16::bfloat16(-0.10)) );
	REQUIRE(true == bfloat16::islessequal(bfloat16::bfloat16(-0.5), bfloat16::bfloat16(-0.10)) );
	REQUIRE(false == bfloat16::islessequal(bfloat16::bfloat16(-0.10), bfloat16::bfloat16(-0.5)) );

	REQUIRE(true == bfloat16::islessequal(bfloat16::bfloat16(-10), bfloat16::bfloat16(-10)) );
	REQUIRE(true == bfloat16::islessequal(bfloat16::bfloat16(-20), bfloat16::bfloat16(-10)) );
	REQUIRE(false == bfloat16::islessequal(bfloat16::bfloat16(-10), bfloat16::bfloat16(-20)) );

	REQUIRE(true == bfloat16::islessequal(bfloat16::bfloat16(-10.1), bfloat16::bfloat16(-10.1)) );
	REQUIRE(true == bfloat16::islessequal(bfloat16::bfloat16(-20.1), bfloat16::bfloat16(-10.1)) );
	REQUIRE(false == bfloat16::islessequal(bfloat16::bfloat16(-10.1), bfloat16::bfloat16(-20.1)) );

	REQUIRE(true == bfloat16::islessequal(bfloat16::bfloat16(-10.5), bfloat16::bfloat16(-10.5)) );
	REQUIRE(true == bfloat16::islessequal(bfloat16::bfloat16(-20.5), bfloat16::bfloat16(-10.5)) );
	REQUIRE(false == bfloat16::islessequal(bfloat16::bfloat16(-10.5), bfloat16::bfloat16(-20.5)) );

}



TEST_CASE("bfloat16_islessgreater", "[float16]"){

	//works

	//positive
	REQUIRE(false == bfloat16::islessgreater(bfloat16::bfloat16(0.10), bfloat16::bfloat16(0.10)) );
	REQUIRE(true == bfloat16::islessgreater(bfloat16::bfloat16(0.5), bfloat16::bfloat16(0.10)) );
	REQUIRE(true == bfloat16::islessgreater(bfloat16::bfloat16(0.10), bfloat16::bfloat16(0.5)) );

	REQUIRE(false == bfloat16::islessgreater(bfloat16::bfloat16(10), bfloat16::bfloat16(10)) );
	REQUIRE(true == bfloat16::islessgreater(bfloat16::bfloat16(20), bfloat16::bfloat16(10)) );
	REQUIRE(true == bfloat16::islessgreater(bfloat16::bfloat16(10), bfloat16::bfloat16(20)) );

	REQUIRE(false == bfloat16::islessgreater(bfloat16::bfloat16(10.1), bfloat16::bfloat16(10.1)) );
	REQUIRE(true == bfloat16::islessgreater(bfloat16::bfloat16(20.1), bfloat16::bfloat16(10.1)) );
	REQUIRE(true == bfloat16::islessgreater(bfloat16::bfloat16(10.1), bfloat16::bfloat16(20.1)) );

	REQUIRE(false == bfloat16::islessgreater(bfloat16::bfloat16(10.5), bfloat16::bfloat16(10.5)) );
	REQUIRE(true == bfloat16::islessgreater(bfloat16::bfloat16(20.5), bfloat16::bfloat16(10.5)) );
	REQUIRE(true == bfloat16::islessgreater(bfloat16::bfloat16(10.5), bfloat16::bfloat16(20.5)) );


	//negative
	REQUIRE(false == bfloat16::islessgreater(bfloat16::bfloat16(-0.10), bfloat16::bfloat16(-0.10)) );
	REQUIRE(true == bfloat16::islessgreater(bfloat16::bfloat16(-0.5), bfloat16::bfloat16(-0.10)) );
	REQUIRE(true == bfloat16::islessgreater(bfloat16::bfloat16(-0.10), bfloat16::bfloat16(-0.5)) );

	REQUIRE(false == bfloat16::islessgreater(bfloat16::bfloat16(-10), bfloat16::bfloat16(-10)) );
	REQUIRE(true == bfloat16::islessgreater(bfloat16::bfloat16(-20), bfloat16::bfloat16(-10)) );
	REQUIRE(true == bfloat16::islessgreater(bfloat16::bfloat16(-10), bfloat16::bfloat16(-20)) );

	REQUIRE(false == bfloat16::islessgreater(bfloat16::bfloat16(-10.1), bfloat16::bfloat16(-10.1)) );
	REQUIRE(true == bfloat16::islessgreater(bfloat16::bfloat16(-20.1), bfloat16::bfloat16(-10.1)) );
	REQUIRE(true == bfloat16::islessgreater(bfloat16::bfloat16(-10.1), bfloat16::bfloat16(-20.1)) );

	REQUIRE(false == bfloat16::islessgreater(bfloat16::bfloat16(-10.5), bfloat16::bfloat16(-10.5)) );
	REQUIRE(true == bfloat16::islessgreater(bfloat16::bfloat16(-20.5), bfloat16::bfloat16(-10.5)) );
	REQUIRE(true == bfloat16::islessgreater(bfloat16::bfloat16(-10.5), bfloat16::bfloat16(-20.5)) );

}



TEST_CASE("bfloat16_isunordered", "[float16]"){

	//works

	//ERRhandling / numeric Limits
	REQUIRE(true == bfloat16::isunordered(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );
	REQUIRE(true == bfloat16::isunordered(std::numeric_limits<bfloat16::bfloat16>::quiet_NaN(), bfloat16::bfloat16()) );
	REQUIRE(true == bfloat16::isunordered(bfloat16::bfloat16(), std::numeric_limits<bfloat16::bfloat16>::quiet_NaN()) );


	//numeric values
	REQUIRE(false == bfloat16::isunordered(bfloat16::bfloat16(1), bfloat16::bfloat16(5)) );
	REQUIRE(false == bfloat16::isunordered(bfloat16::bfloat16(-1), bfloat16::bfloat16(5)) );
	REQUIRE(false == bfloat16::isunordered(bfloat16::bfloat16(1), bfloat16::bfloat16(-5)) );
	REQUIRE(false == bfloat16::isunordered(bfloat16::bfloat16(-1), bfloat16::bfloat16(-5)) );

	REQUIRE(false == bfloat16::isunordered(bfloat16::bfloat16(0.1), bfloat16::bfloat16(0.5)) );
	REQUIRE(false == bfloat16::isunordered(bfloat16::bfloat16(0.1), bfloat16::bfloat16(-0.5)) );
	REQUIRE(false == bfloat16::isunordered(bfloat16::bfloat16(-0.1), bfloat16::bfloat16(0.5)) );
	REQUIRE(false == bfloat16::isunordered(bfloat16::bfloat16(-0.1), bfloat16::bfloat16(-0.5)) );

}
