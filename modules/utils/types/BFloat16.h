#ifndef BFLOAT_BFLOAT_HPP
#define BFLOAT_BFLOAT_HPP

#define BFLOAT_GCC_VERSION (__GNUC__*100+__GNUC_MINOR__)

#if defined(__INTEL_COMPILER)
        #define BFLOAT_ICC_VERSION __INTEL_COMPILER
#elif defined(__ICC)
        #define BFLOAT_ICC_VERSION __ICC
#elif defined(__ICL)
        #define BFLOAT_ICC_VERSION __ICL
#else
        #define BFLOAT_ICC_VERSION 0
#endif

// check C++11 language features
#if defined(__clang__) // clang
        #if __has_feature(cxx_static_assert) && !defined(BFLOAT_ENABLE_CPP11_STATIC_ASSERT)
                #define BFLOAT_ENABLE_CPP11_STATIC_ASSERT 1
        #endif
        #if __has_feature(cxx_constexpr) && !defined(BFLOAT_ENABLE_CPP11_CONSTEXPR)
                #define BFLOAT_ENABLE_CPP11_CONSTEXPR 1
        #endif
        #if __has_feature(cxx_noexcept) && !defined(BFLOAT_ENABLE_CPP11_NOEXCEPT)
                #define BFLOAT_ENABLE_CPP11_NOEXCEPT 1
        #endif
        #if __has_feature(cxx_user_literals) && !defined(BFLOAT_ENABLE_CPP11_USER_LITERALS)
                #define BFLOAT_ENABLE_CPP11_USER_LITERALS 1
        #endif
        #if __has_feature(cxx_thread_local) && !defined(BFLOAT_ENABLE_CPP11_THREAD_LOCAL)
                #define BFLOAT_ENABLE_CPP11_THREAD_LOCAL 1
        #endif
        #if (defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L) && !defined(BFLOAT_ENABLE_CPP11_LONG_LONG)
                #define BFLOAT_ENABLE_CPP11_LONG_LONG 1
        #endif

#elif BFLOAT_ICC_VERSION && defined(__INTEL_CXX11_MODE__) // Intel C++
        #if BFLOAT_ICC_VERSION >= 1500 && !defined(BFLOAT_ENABLE_CPP11_THREAD_LOCAL)
                #define BFLOAT_ENABLE_CPP11_THREAD_LOCAL 1
        #endif
        #if BFLOAT_ICC_VERSION >= 1500 && !defined(BFLOAT_ENABLE_CPP11_USER_LITERALS)
                #define BFLOAT_ENABLE_CPP11_USER_LITERALS 1
        #endif
        #if BFLOAT_ICC_VERSION >= 1400 && !defined(BFLOAT_ENABLE_CPP11_CONSTEXPR)
                #define BFLOAT_ENABLE_CPP11_CONSTEXPR 1
        #endif
        #if BFLOAT_ICC_VERSION >= 1400 && !defined(BFLOAT_ENABLE_CPP11_NOEXCEPT)
                #define BFLOAT_ENABLE_CPP11_NOEXCEPT 1
        #endif
        #if BFLOAT_ICC_VERSION >= 1110 && !defined(BFLOAT_ENABLE_CPP11_STATIC_ASSERT)
                #define BFLOAT_ENABLE_CPP11_STATIC_ASSERT 1
        #endif
        #if BFLOAT_ICC_VERSION >= 1110 && !defined(BFLOAT_ENABLE_CPP11_LONG_LONG)
                #define BFLOAT_ENABLE_CPP11_LONG_LONG 1
        #endif

#elif defined(__GNUC__) // gcc
        #if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
                #if BFLOAT_GCC_VERSION >= 408 && !defined(BFLOAT_ENABLE_CPP11_THREAD_LOCAL)
                        #define BFLOAT_ENABLE_CPP11_THREAD_LOCAL 1
                #endif
                #if BFLOAT_GCC_VERSION >= 407 && !defined(BFLOAT_ENABLE_CPP11_USER_LITERALS)
                        #define BFLOAT_ENABLE_CPP11_USER_LITERALS 1
                #endif
                #if BFLOAT_GCC_VERSION >= 406 && !defined(BFLOAT_ENABLE_CPP11_CONSTEXPR)
                        #define BFLOAT_ENABLE_CPP11_CONSTEXPR 1
                #endif
                #if BFLOAT_GCC_VERSION >= 406 && !defined(BFLOAT_ENABLE_CPP11_NOEXCEPT)
                        #define BFLOAT_ENABLE_CPP11_NOEXCEPT 1
                #endif
                #if BFLOAT_GCC_VERSION >= 403 && !defined(BFLOAT_ENABLE_CPP11_STATIC_ASSERT)
                        #define BFLOAT_ENABLE_CPP11_STATIC_ASSERT 1
                #endif
                #if !defined(BFLOAT_ENABLE_CPP11_LONG_LONG)
                        #define BFLOAT_ENABLE_CPP11_LONG_LONG 1
                #endif
        #endif
        #define BFLOAT_TWOS_COMPLEMENT_INT 1

#elif defined(_MSC_VER) // Visual C++
        #if _MSC_VER >= 1900 && !defined(BFLOAT_ENABLE_CPP11_THREAD_LOCAL)
                #define BFLOAT_ENABLE_CPP11_THREAD_LOCAL 1
        #endif
        #if _MSC_VER >= 1900 && !defined(BFLOAT_ENABLE_CPP11_USER_LITERALS)
                #define BFLOAT_ENABLE_CPP11_USER_LITERALS 1
        #endif
        #if _MSC_VER >= 1900 && !defined(BFLOAT_ENABLE_CPP11_CONSTEXPR)
                #define BFLOAT_ENABLE_CPP11_CONSTEXPR 1
        #endif
        #if _MSC_VER >= 1900 && !defined(BFLOAT_ENABLE_CPP11_NOEXCEPT)
                #define BFLOAT_ENABLE_CPP11_NOEXCEPT 1
        #endif
        #if _MSC_VER >= 1600 && !defined(BFLOAT_ENABLE_CPP11_STATIC_ASSERT)
                #define BFLOAT_ENABLE_CPP11_STATIC_ASSERT 1
        #endif
        #if _MSC_VER >= 1310 && !defined(BFLOAT_ENABLE_CPP11_LONG_LONG)
                #define BFLOAT_ENABLE_CPP11_LONG_LONG 1
        #endif
        #define BFLOAT_TWOS_COMPLEMENT_INT 1
        #define BFLOAT_POP_WARNINGS 1
        #pragma warning(push)
        #pragma warning(disable : 4099 4127 4146) //struct vs class, constant in if, negative unsigned
#endif

// ckeck C++11 library features
#include <utility>
#if defined(_LIBCPP_VERSION)
        #if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103
                #ifndef BFLOAT_ENABLE_CPP11_TYPE_TRAITS
                        #define BFLOAT_ENABLE_CPP11_TYPE_TRAITS 1
                #endif
                #ifndef BFLOAT_ENABLE_CPP11_CSTDINT
                        #define BFLOAT_ENABLE_CPP11_CSTDINT 1
                #endif
                #ifndef BFLOAT_ENABLE_CPP11_CMATH
                        #define BFLOAT_ENABLE_CPP11_CMATH 1
                #endif
                #ifndef BFLOAT_ENABLE_CPP11_HASH
                        #define BFLOAT_ENABLE_CPP11_HASH 1
                #endif
                #ifndef BFLOAT_ENABLE_CPP11_CFENV
                        #define BFLOAT_ENABLE_CPP11_CFENV 1
                #endif
        #endif

#elif defined(__GLIBCXX__) // libstdc++
        #if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103
                #ifdef __clang__
                        #if __GLIBCXX__ >= 20080606 && !defined(BFLOAT_ENABLE_CPP11_TYPE_TRAITS)
                                #define BFLOAT_ENABLE_CPP11_TYPE_TRAITS 1
                        #endif
                        #if __GLIBCXX__ >= 20080606 && !defined(BFLOAT_ENABLE_CPP11_CSTDINT)
                                #define BFLOAT_ENABLE_CPP11_CSTDINT 1
                        #endif
                        #if __GLIBCXX__ >= 20080606 && !defined(BFLOAT_ENABLE_CPP11_CMATH)
                                #define BFLOAT_ENABLE_CPP11_CMATH 1
                        #endif
                        #if __GLIBCXX__ >= 20080606 && !defined(BFLOAT_ENABLE_CPP11_HASH)
                                #define BFLOAT_ENABLE_CPP11_HASH 1
                        #endif
                        #if __GLIBCXX__ >= 20080606 && !defined(BFLOAT_ENABLE_CPP11_CFENV)
                                #define BFLOAT_ENABLE_CPP11_CFENV 1
                        #endif
                #else
                        #if BFLOAT_GCC_VERSION >= 403 && !defined(BFLOAT_ENABLE_CPP11_TYPE_TRAITS)
                                #define BFLOAT_ENABLE_CPP11_TYPE_TRAITS 1
                        #endif
                        #if BFLOAT_GCC_VERSION >= 403 && !defined(BFLOAT_ENABLE_CPP11_CSTDINT)
                                #define BFLOAT_ENABLE_CPP11_CSTDINT 1
                        #endif
                        #if BFLOAT_GCC_VERSION >= 403 && !defined(BFLOAT_ENABLE_CPP11_CMATH)
                                #define BFLOAT_ENABLE_CPP11_CMATH 1
                        #endif
                        #if BFLOAT_GCC_VERSION >= 403 && !defined(BFLOAT_ENABLE_CPP11_HASH)
                                #define BFLOAT_ENABLE_CPP11_HASH 1
                        #endif
                        #if BFLOAT_GCC_VERSION >= 403 && !defined(BFLOAT_ENABLE_CPP11_CFENV)
                                #define BFLOAT_ENABLE_CPP11_CFENV 1
                        #endif
                #endif
        #endif

#elif defined(_CPPLIB_VER) // Dinkumware/Visual C++
        #if _CPPLIB_VER >= 520 && !defined(BFLOAT_ENABLE_CPP11_TYPE_TRAITS)
                #define BFLOAT_ENABLE_CPP11_TYPE_TRAITS 1
        #endif
        #if _CPPLIB_VER >= 520 && !defined(BFLOAT_ENABLE_CPP11_CSTDINT)
                #define BFLOAT_ENABLE_CPP11_CSTDINT 1
        #endif
        #if _CPPLIB_VER >= 520 && !defined(BFLOAT_ENABLE_CPP11_HASH)
                #define BFLOAT_ENABLE_CPP11_HASH 1
        #endif
        #if _CPPLIB_VER >= 610 && !defined(BFLOAT_ENABLE_CPP11_CMATH)
                #define BFLOAT_ENABLE_CPP11_CMATH 1
        #endif
        #if _CPPLIB_VER >= 610 && !defined(BFLOAT_ENABLE_CPP11_CFENV)
                #define BFLOAT_ENABLE_CPP11_CFENV 1
        #endif
#endif
#undef BFLOAT_GCC_VERSION
#undef BFLOAT_ICC_VERSION

// any error throwing C++ exceptions?
#if defined(BFLOAT_ERRHANDLING_THROW_INVALID)  || defined(BFLOAT_ERRHANDLING_THROW_DIVBYZERO) || defined(BFLOAT_ERRHANDLING_THROW_OVERFLOW) || defined(BFLOAT_ERRHANDLING_TRHOW_UNDERFLOW) || defined(BFLOAT_ERRHANDLING_THROW_INEXACT)
#define BFLOAT_ERRHANDLING_THROWS 1
#endif

// any error handling enabled?
#define BFLOAT_ERRHANDLING        (BFLOAT_ERRHANDLING_FLAGS||BFLOAT_ERRHANDLING_ERRNO|| BFLOAT_ERRHANDLING_FENV||BFLOAT_ERRHANDLING_THROWS)

#if BFLOAT_ERRHANDLING
        #define BFLOAT_UNUSED_NOERR(name) name
#else
        #define BFLOAT_UNUSED_NOERR(name)
#endif

// support constexpr
#if BFLOAT_ENABLE_CPP11_CONSTEXPR
        #define BFLOAT_CONSTEXPR                  constexpr
        #define BFLOAT_CONSTEXPR_CONST            constexpr
        #if BFLOAT_ERRHANDLING
                #define BFLOAT_CONSTEXPR_NOERR
        #else
                #define BFLOAT_CONSTEXPR_NOERR    constexpr
        #endif
#else
        #define BFLOAT_CONSTEXPR
        #define BFLOAT_CONSTEXPR_CONST            const
        #define BFLOAT_CONSTEXPR_NOERR
#endif

// support noexcept
#if BFLOAT_ENABLE_CPP11_NOEXCEPT
        #define BFLOAT_NOEXCEPT     noexcept
        #define BFLOAT_NOTHROW      noexcept
#else
        #define BFLOAT_NOEXCEPT
        #define BFLOAT_NOTHROW      throw()
#endif

// support thread storage
#if BFLOAT_ENABLE_CPP11_THREAD_LOCAL
        #define BFLOAT_THREAD_LOCAL     thread_local
#else
        #define BFLOAT_THREAD_LOCAL     static
#endif

#include <utility>
#include <algorithm>
#include <istream>
#include <ostream>
#include <limits>
#include <stdexcept>
#include <climits>
#include <cmath>
#include <cstring>
#include <cstdlib>
#if BFLOAT_ENABLE_CPP11_TYPE_TRAITS
        #include <type_traits>
#endif
#if BFLOAT_ENABLE_CPP11_CSTDINT
        #include <cstdint>
#endif
#if BFLOAT_ERRHANDLING_ERRNO
        #include <cerrno>
#endif
#if BFLOAT_ENABLE_CPP11_CFENV
        #include <cfenv>
#endif
#if BFLOAT_ENABLE_CPP11_HASH
        #include <functional>
#include <iostream>

#endif


#ifdef BFLOAT_DOXYGEN_ONLY
/// Type for internal floating-point computations.
/// This can be predefined to a built-in floating-point type (`float`, `double` or `long double`) to override the internal 
/// bfloat16-precision implementation to use this type for computing arithmetic operations and mathematical function (if available). 
/// This can result in improved performance for arithmetic operators and mathematical functions but might cause results to 
/// deviate from the specified bfloat16-precision rounding mode and inhibits proper detection of bfloat16-precision exceptions.
#define BFLOAT_ARITHMETIC_TYPE (undefined)

/// Enable internal exception flags.
/// Defining this to 1 causes operations on bfloat16-precision values to raise internal floating-point exception flags according to 
/// the IEEE 754 standard. These can then be cleared and checked with clearexcept(), testexcept().
#define BFLOAT_ERRHANDLING_FLAGS 0

/// Enable exception propagation to `errno`.
/// Defining this to 1 causes operations on bfloat16-precision values to propagate floating-point exceptions to 
/// [errno](https://en.cppreference.com/w/cpp/error/errno) from `<cerrno>`. Specifically this will propagate domain errors as 
/// [EDOM](https://en.cppreference.com/w/cpp/error/errno_macros) and pole, overflow and underflow errors as 
/// [ERANGE](https://en.cppreference.com/w/cpp/error/errno_macros). Inexact errors won't be propagated.
#define BFLOAT_ERRHANDLING_ERRNO 0

/// Enable exception propagation to built-in floating-point platform.
/// Defining this to 1 causes operations on bfloat16-precision values to propagate floating-point exceptions to the built-in 
/// single- and double-precision implementation's exception flags using the 
/// [C++11 floating-point environment control](https://en.cppreference.com/w/cpp/numeric/fenv) from `<cfenv>`. However, this 
/// does not work in reverse and single- or double-precision exceptions will not raise the corresponding bfloat16-precision 
/// exception flags, nor will explicitly clearing flags clear the corresponding built-in flags.
#define BFLOAT_ERRHANDLING_FENV 0

/// Throw C++ exception on domain errors.
/// Defining this to a string literal causes operations on bfloat16-precision values to throw a 
/// [std::domain_error](https://en.cppreference.com/w/cpp/error/domain_error) with the specified message on domain errors.
#define BFLOAT_ERRHANDLING_THROW_INVALID       (undefined)

/// Throw C++ exception on pole errors.
/// Defining this to a string literal causes operations on bfloat16-precision values to throw a 
/// [std::domain_error](https://en.cppreference.com/w/cpp/error/domain_error) with the specified message on pole errors.
#define BFLOAT_ERRHANDLING_THROW_DIVBYZERO     (undefined)

/// Throw C++ exception on overflow errors.
/// Defining this to a string literal causes operations on bfloat16-precision values to throw a 
/// [std::overflow_error](https://en.cppreference.com/w/cpp/error/overflow_error) with the specified message on overflows.
#define BFLOAT_ERRHANDLING_THROW_OVERFLOW      (undefined)

/// Throw C++ exception on underflow errors.
/// Defining this to a string literal causes operations on bfloat16-precision values to throw a 
/// [std::underflow_error](https://en.cppreference.com/w/cpp/error/underflow_error) with the specified message on underflows.
#define BFLOAT_ERRHANDLING_THROW_UNDERFLOW     (undefined)

/// Throw C++ exception on rounding errors.
/// Defining this to 1 causes operations on bfloat16-precision values to throw a 
/// [std::range_error](https://en.cppreference.com/w/cpp/error/range_error) with the specified message on general rounding errors.
#define BFLOAT_ERRHANDLING_THROW_INEXACT       (undefined)
#endif

#ifndef BFLOAT_ERRHANDLING_OVERFLOW_TO_INEXACT
/// Raise INEXACT exception on overflow.
/// Defining this to 1 (default) causes overflow errors to automatically raise inexact exceptions in addition.
/// These will be raised after any possible handling of the underflow exception.
#define BFLOAT_ERRHANDLING_OVERFLOW_TO_INEXACT 1
#endif

#ifndef BFLOAT_ERRHANDLING_UNDERFLOW_TO_INEXACT
/// Raise INEXACT exception on underflow.
/// Defining this to 1 (default) causes underflow errors to automatically raise inexact exceptions in addition.
/// These will be raised after any possible handling of the underflow exception.
///
/// **Note:** This will actually cause underflow (and the accompanying inexact) exceptions to be raised *only* when the result 
/// is inexact, while if disabled bare underflow errors will be raised for *any* (possibly exact) subnormal result.
#define BFLOAT_ERRHANDLING_UNDERFLOW_TO_INEXACT 1
#endif

/// Default rounding mode.
/// This specifies the rounding mode used for all conversions between [bfloat16](\ref bfloat16::bfloat16)s and more precise types 
/// (unless using bfloat16_cast() and specifying the rounding mode directly) as well as in arithmetic operations and mathematical 
/// functions. It can be redefined (before including bfloat16.hpp) to one of the standard rounding modes using their respective 
/// constants or the equivalent values of 
/// [std::float_round_style](https://en.cppreference.com/w/cpp/types/numeric_limits/float_round_style):
///
/// `std::float_round_style`         | value | rounding
/// ---------------------------------|-------|-------------------------
/// `std::round_indeterminate`       | -1    | fastest
/// `std::round_toward_zero`         | 0     | toward zero
/// `std::round_to_nearest`          | 1     | to nearest (default)
/// `std::round_toward_infinity`     | 2     | toward positive infinity
/// `std::round_toward_neg_infinity` | 3     | toward negative infinity
///
/// By default this is set to `1` (`std::round_to_nearest`), which rounds results to the nearest representable value. It can even 
/// be set to [std::numeric_limits<float>::round_style](https://en.cppreference.com/w/cpp/types/numeric_limits/round_style) to synchronize 
/// the rounding mode with that of the built-in single-precision implementation (which is likely `std::round_to_nearest`, though).
#ifndef BFLOAT_ROUND_STYLE
        #define BFLOAT_ROUND_STYLE 1 // = std::round_to_nearest
#endif

/// Value signaling overflow.
/// In correspondence with `HUGE_VAL[F|L]` from `<cmath>` this symbol expands to a positive value signaling the overflow of an 
/// operation, in particular it just evaluates to positive infinity.
///
/// **See also:** Documentation for [HUGE_VAL](https://en.cppreference.com/w/cpp/numeric/math/HUGE_VAL)
#define HUGE_VALH     std::numeric_limits<bfloat16::bfloat16>::infinity()

/// Fast bfloat16-precision fma function.
/// This symbol is defined if the fma() function generally executes as fast as, or faster than, a separate 
/// bfloat16-precision multiplication followed by an addition, which is always the case.
///
/// **See also:** Documentation for [FP_FAST_FMA](https://en.cppreference.com/w/cpp/numeric/math/fma)
#define FP_FAST_FMAH 1

///	Bfloat rounding mode.
/// In correspondence with `FLT_ROUNDS` from `<cfloat>` this symbol expands to the rounding mode used for 
/// bfloat16-precision operations. It is an alias for [BFLOAT_ROUND_STYLE](\ref BFLOAT_ROUND_STYLE).
///
/// **See also:** Documentation for [FLT_ROUNDS](https://en.cppreference.com/w/cpp/types/climits/FLT_ROUNDS)
#define BFL_ROUNDS     BFLOAT_ROUND_STYLE

#ifndef FP_ILOGB0
        #define FP_ILOGB0         INT_MIN
#endif
#ifndef FP_ILOGBNAN
        #define FP_ILOGBNAN       INT_MAX
#endif
#ifndef FP_SUBNORMAL
        #define FP_SUBNORMAL      0
#endif
#ifndef FP_ZERO
        #define FP_ZERO           1
#endif
#ifndef FP_NAN
        #define FP_NAN            2
#endif
#ifndef FP_INFINITE
        #define FP_INFINITE       3
#endif
#ifndef FP_NORMAL
        #define FP_NORMAL         4
#endif

#if !BFLOAT_ENABLE_CPP11_CFENV && !defined(FE_ALL_EXCEPT)
        #define FE_INVALID           0x10
        #define FE_DIVBYZERO         0x08
        #define FE_OVERFLOW          0x04
        #define FE_UNDERFLOW         0x02
        #define FE_INEXACT           0x01
        #define FE_ALL_EXCEPT        (FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW|FE_UNDERFLOW|FE_INEXACT)
#endif


/// Main namespace for bfloat16-precision functionality.
/// This namespace contains all the functionality provided by the library.
namespace bfloat16
{
        class bfloat16;
  
#if BFLOAT_ENABLE_CPP11_USER_LITERALS
        /// Library-defined bfloat16-precision literals.
	      /// Import this namespace to enable bfloat16-precision floating-point literals:
	      /// ~~~~{.cpp}
	      /// using namespace bfloat16::literal;
	      /// bfloat16::bfloat16 = 4.2_h;
	      /// ~~~~
        namespace literal
        {
                bfloat16 operator "" _h(long double);
        }
#endif
  
        /// \internal
        /// \brief Implementation details.
        namespace detail
        {
        #if BFLOAT_ENABLE_CPP11_TYPE_TRAITS
                /// Conditional type.
                template<bool B,typename T,typename F> struct conditional : std::conditional<B,T,F> {};
        
                /// Helper for tag dispatching.
                template<bool B> struct bool_type : std::integral_constant<bool,B> {};
                using std::true_type;
                using std::false_type;
        
                /// Type traits for floating-point types.
                template<typename T> struct is_float : std::is_floating_point<T> {};
        #else
        
                /// Conditional type.
                template<bool,typename T,typename> struct conditional { typedef T type; };
                template<typename T,typename F> struct conditional<false,T,F> { typedef F type; };
        
                /// Helper for tag dispatching.
                template<bool> struct bool_type {};
                typedef bool_type<true> true_type;
                typedef bool_type<false> false_type;
        
                /// Type traits for floating-point types.
                template<typename> struct is_float : false_type {};
                template<typename T> struct is_float<const T> : is_float<T> {};
                template<typename T> struct is_float<volatile T> : is_float<T> {};
                template<typename T> struct is_float<const volatile T> : is_float<T> {};
                template<> struct is_float<float> : true_type {};
                template<> struct is_float<double> :  true_type {};
                template<> struct is_float<long double> : true_type {};
        #endif
        
                /// Type traits for floating-point bits.
                template<typename T> struct bits { typedef unsigned char type; };
                template<typename T> struct bits<const T> : bits<T> {};
                template<typename T> struct bits<volatile T> : bits<T> {};
                template<typename T> struct bits<const volatile T> : bits <T> {};
        
        #if BFLOAT_ENABLE_CPP11_CSTDINT
                /// Unsigned integer of (at least) 16 bits width.
                typedef std::uint_least16_t uint16;
        
                /// Fastest unsigned integer of (at least) 32 bits width.
                typedef std::uint_fast32_t uint32;
        
                /// Fastest signed integer of (at least) 32 bits width.
                typedef std::int_fast32_t int32;
        
                /// Unsigned integer of (at least) 32 bits width.
                template<> struct bits<float> { typedef std::uint_least32_t type; };
        
                /// Unsigned integer of (at least) 64 bits width.
                template<> struct bits<double> { typedef std::uint_least64_t type; };
        #else
                /// Unsigned integer of (at least) 16 bits width.
                typedef unsigned short uint16;
        
                /// Fastest unsigned integer of (at least) 32 bits width.
                typedef unsigned long uint32;
        
                /// Fastest signed integer of (at least) 32 bits width.
                typedef long int32;
        
                /// Unsigned integer of (at least) 32 bits width.
                template<> struct bits<float> : conditional<std::numeric_limits<unsigned int>::digits>=32,unsigned int, unsigned long> {};
        
                #if BFLOAT_ENABLE_CPP11_LONG_LONG
                        /// Unsigned Integer of (at least) 64 bits width.
                        template<> struct bits<double> : conditional<std::numeric_limits<unsigned long>::digits>=64,unsigned long,unsigned long long> {};
                #else
                        /// Unsigned integer of (at least) 64 bits width.
                        template<> struct bits<double> { typedef unsigned long type };
                #endif
        #endif
        
        #ifdef BFLOAT_ARITHMETIC_TYPE
                /// Type to use for arithmetic computations and matematic functions internally.
                typedef BFLOAT_ARITHMETIC_TYPE internal_t;
        #endif
        
                /// Tag type for binary construction.
                struct binary_t {};
        
                /// Tag for binary construction.
                BFLOAT_CONSTEXPR_CONST binary_t binary = binary_t();
        
                /// \name Implementation defined classification and arithmetic
                /// \{

                /// Check for infinity.
                /// \tparam T argument type (builtin floating-point type)
                /// \param arg value to query
                /// \retval true if infinity
                /// \retval false else
                template<typename T> bool builtin_isinf(T arg)
                {

                        return arg == std::numeric_limits<T>::infinity() || arg == -std::numeric_limits<T>::infinity();


                }
        
                /// Check for NaN.
                /// \tparam T argument type (builtin floating-point type)
                /// \param arg value to query
                /// \retval true if not a number
                /// \retval false else
                template<typename T> bool builtin_isnan(T arg)
                {
//                #if BFLOAT_ENABLE_CPP11_CMATH
//                        return std::isnan(arg); // does not work with bfloat16
//                #else

                        return arg != arg;
//                #endif
                }

                /// Check sign.
                /// \tparam T argument type (builtin floating-point type)
                /// \param arg value to query
                /// \retval true if signbit set
                /// \retval false else
                template<typename T> bool builtin_signbit(T arg)
                {
//                #if BFLOAT_ENABLE_CPP11_CMATH
//                        return std::signbit(arg); // does not work with bfloat16
//                #else

                        return arg < T() || (arg == T() && T(1)/arg < T());
//                #endif
                }

                /// Platform-independent sign mask.
                /// \param arg integer value in two's complement
                /// \retval -1 if \a arg negative
                /// \retval 0 if \a arg positive
                inline uint32 sign_mask(uint32 arg)
                {
                        static const int N = std::numeric_limits<uint32>::digits - 1;

                #if BFLOAT_TWOS_COMPLEMENT_INT
                        return static_cast<int32>(arg) >> N;
                #else

                        return -((arg>>N)&1);
                #endif
                }
        
                /// Platform-independent arithmetic right shift.
                /// \param arg integer value in two's complement
                /// \param i shift amount (at most 31)
                /// \return \a arg right shifted for \a i bits with possible sign extension
                inline uint32 arithmetic_shift(uint32 arg, int i)
                {

                #if BFLOAT_TWOS_COMPLEMENT_INT
                        return static_cast<int32>(arg) >> i;
                #else

                        return static_cast<int32>(arg)/(static_cast<int32>(1)<<i) -
                          ((arg>>(std::numeric_limits<uint32>::digits-1))&1);
                #endif
                }
        
                /// \}
                /// \name Error handling
                /// \{

                /// Internal exception flags.
                /// \return reference to global exception flags
                inline int& errflags() { BFLOAT_THREAD_LOCAL int flags = 0; return flags; }

                /// Raise floating-point exception.
                /// \param flags exceptions to raise
                /// \param cond condition to raise exceptions for
                inline void raise(int BFLOAT_UNUSED_NOERR(flags), bool BFLOAT_UNUSED_NOERR(cond) = true)
                {
                #if BFLOAT_ERRHANDLING
                        if(!cond)
                                return;
                #if BFLOAT_ERHHANDLING_FLAGS
                        errflags() |= flags;
                #endif
                #if BFLOAT_ERRHANDLING_ERRNO
                        if(flags & FE_INVALID)
                                errno = EDOM;
                        else if(flags & (FE_DIVBYZERO|FE_OVERFLOW|FE_UNDERFLOW))
                                errno = ERANGE;
                #endif
                #if BFLOAT_ERRHANDLING_FENV && BFLOAT_ENABLE_CPP11_CFENV
                        std::feraiseexcept(flags);
                #endif
                #ifdef BFLOAT_ERRHANDLING_THROW_INVALID
                        if(flags & FE_INVALID)
                                throw std::domain_error(BFLOAT_ERRHANDLING_THROW_INVALID);
                #endif
                #ifdef BFLOAT_ERRHANDLING_THROW_DIVBYZERO
                        if(flags & FE_DIVBYZERO)
                                throw std::domain_error(BFLOAT_ERRHANDLING_THROW_DIVBYZERO);
                #endif
                #ifdef BFLOAT_ERRHANDLING_THROW_OVERFLOW
                        if(flags & FE_OVERFLOW)
                                throw std::overflow_error(BFLOAT_ERRHANDLING_THROW_OVERFLOW);
                #endif
                #ifdef BFLOAT_ERRHANDLING_THROW_UNDERFLOW
                        if(flags & FE_UNDERFLOW)
                                throw std::underflow_error(BFLOAT_ERRHANDLING_THROW_UNDERFLOW);
                #endif
                #ifdef BFLOAT_ERRHANDLING_THROW_INEXACT
                        if(flags & FE_INEXACT)
                                throw std::range_error(BFLOAT_ERRHANDLINIG_THROW_INEXACT);
                #endif
                #if BFLOAT_ERRHANDLING_UNDERFLOW_TO_INEXACT
                        if((flags & FE_UNDERFLOW) && !(flags & FE_INEXACT))
                                raise(FE_INEXACT);
                #endif
                #if BFLOAT_ERRHANDLING_OVERFLOW_TO_INEXACT
                        if((flags & FE_OVERFLOW) && !(flags & FE_INEXACT))
                                raise(FE_INEXACT);
                #endif
                #endif
                }
          
                /// Check and signal for any NaN.
                /// \param x first bfloat16-precision value to check
                /// \param y second bfloat16-precision value to check
                /// \retval true if either \a x or \a y is NaN
                /// \retval false else
                /// \exception FE_INVALID if \a x or \a y is NaN
                inline BFLOAT_CONSTEXPR_NOERR bool compsignal(unsigned int x, unsigned int y)
                {

                #if BFLOAT_ERRHANDLING
                        raise(FE_INVALID, (x&0x7FFF)>0x7F80 || (y&0x7FFF)>0x7F80);

                #endif
                        return (x&0x7FFF) > 0x7F80 || (y&0x7FFF) > 0x7F80;
                }
        
                /// Signal and silence signaling NaN.
                /// \param nan bfloat16-precision NaN value
                /// \return quiet NaN
                /// \exception FE_INVALID if \a nan is signaling NaN
                inline BFLOAT_CONSTEXPR_NOERR unsigned int signal(unsigned int nan)
                {

                #if BFLOAT_ERRHANDLING
                        raise(FE_INVALID, !(nan&0x40));

                #endif
                        return nan | 0x7FFF;
                }
        
                /// Signal and silence signaling NaNs.
                /// \param x first bfloat16-precision value to check
                /// \param y second bfloat16-precision value to check
                /// \return quiet NaN
                /// \exception FE_INVALID if \a x or \a y is signaling NaN
                inline BFLOAT_CONSTEXPR_NOERR unsigned int signal(unsigned int x, unsigned int y)
                {
                #if BFLOAT_ERRHANDLING
                        raise(FE_INVALID, ((x&0x7FFF)>0x7F80 && !(x&0x40)) || ((y&0x7FFF)>0x7F80 && !(y&0x40)));
                #endif
                        return ((x&0x7FFF)>0x7F80) ? (x|0x40) : (y|0x40);
                }
        
                /// Signal and silence signaling NaNs.
                /// \param x first bfloat16-precision value to check
                /// \param y second bfloat16-precision value to check
                /// \param z third bfloat16-precision value to check
                /// \return quiet NaN
                /// \exception FE_INVALID if \a x, \a y or \a z is signaling NaN
                inline BFLOAT_CONSTEXPR_NOERR unsigned int signal(unsigned int x, unsigned int y, unsigned int z)
                {
                #if BFLOAT_ERRHANDLING
                        raise(FE_INVALID, ((x&0x7FFF)>0x7F80 && !(x&0x40)) || ((y&0x7FFF)>0x7F80 && !(y&0x40)) || ((z&0x7FFF)>0x7F80 && !(z&0x40)));
                #endif
                        return ((x&0x7FFF)>0x7F80) ? (x|0x40) : ((y&0x7FFF)>0x7F80) ? (y|0x40) : (z|0x40);
                }
        
                /// Select value or signaling NaN.
                /// \param x preferred bfloat16-precision value
                /// \param y ignored bfloat16-precision value except for signaling NaN
                /// \return \a y if signaling NaN, \a x otherwise
                /// \exception FE_INVALID if \a y is signaling NaN
                inline BFLOAT_CONSTEXPR_NOERR unsigned int select(unsigned int x, unsigned BFLOAT_UNUSED_NOERR(y))
                {
                #if BFLOAT_ERRHANDLING
                        return (((y&0x7FFF)>0x7F80) && !(y&0x40)) ? signal(y) : x;
                #else
                        return x;
                #endif
                }
        
                /// Raise domain error and return NaN.
                /// return quiet NaN
                /// \exception FE_INVALID
                inline BFLOAT_CONSTEXPR_NOERR unsigned int invalid()
                {
                #if BFLOAT_ERRHANDLING
                        raise(FE_INVALID);
                #endif
                        return 0x7FFF;
                }
        
                /// Raise pole error and return infinity.
                /// \param sign bfloat16-precision value with sign bit only
                /// \return bfloat16-precision infinity with sign of \a sign
                /// \exception FE_DIVBYZERO
                inline BFLOAT_CONSTEXPR_NOERR unsigned int pole(unsigned int sign = 0)
                {
                #if BFLOAT_ERRHANDLING
                        raise(FE_DIVBYZERO);
                #endif
                        return sign | 0x7F80;
                }
        
                /// Check value for underflow.
                /// \param arg non-zero bfloat16-precision value to check
                /// \return \a arg
                /// \exception FE_UNDERFLOW if arg is subnormal
                inline BFLOAT_CONSTEXPR_NOERR unsigned int check_underflow(unsigned int arg)
                {
                #if BFLOAT_ERRHANDLING && !BFLOAT_ERRHANDLING_UNDERFLOW_TO_INEXACT
                        raise(FE_UNDERFLOW, !(arg&0x7F80));
                #endif
                        return arg;
                }
        
                /// \}
                /// \name Conversion and rounding
                /// \{

                /// Bfloat16-precision overflow.
                /// \tparam R rounding mode to use
                /// \param sign bfloat16-precision value with sign bit only
                /// \return rounded overflowing bfloat16-precision value
                /// \exception FE_OVERFLOW
                template<std::float_round_style R> BFLOAT_CONSTEXPR_NOERR unsigned int overflow(unsigned int sign = 0)
                {
                #if BFLOAT_ERRHANDLING
                        raise(FE_OVERFLOW);
                #endif
                        return (R==std::round_toward_infinity) ? (sign+0x7F80-(sign>>15)) :
                          (R==std::round_toward_neg_infinity) ? (sign+0x7F7F+(sign>>15)) :
                          (R==std::round_toward_zero) ? (sign|0x7F7F) :
                          (sign|0x7F80);
                }
          
                /// Bfloat16-precision underflow.
                /// \tparam R rounding mode to use
                /// \param sign bfloat16-precision value with sign bit only
                /// \return rounded underflowing bfloat16-precision value
                /// \exception FE_UNDERFLOW
                template<std::float_round_style R> BFLOAT_CONSTEXPR_NOERR unsigned int underflow(unsigned int sign = 0)
                {
                #if BFLOAT_ERRHANDLING
                        raise(FE_UNDERFLOW);
                #endif
                        return (R==std::round_toward_infinity) ? (sign+1-(sign>>15)) :
                          (R==std::round_toward_neg_infinity) ? (sign+(sign>>15)) :
                          sign;
                }
        
                /// Round bfloat16-precision number.
                /// \tparam R rounding mode to use
                /// \tparam I `true` to always raise INEXACT exception, `false` to raise only for rounded results
                /// \param value finite bfloat16-precision number to round
                /// \param g guard bit (most significant discarded bit)
                /// \param s sticky bit (or of all but the most significant discarded bits)
                /// \return rounded bfloat16-precision value
                /// \exception FE_OVERFLOW on overflows
                /// \exception FE_UNDERFLOW on underflows
                /// \exception FE_INEXACT if value had to be rounded or \a I is `true`
                template<std::float_round_style R,bool I> BFLOAT_CONSTEXPR_NOERR unsigned int rounded(unsigned int value, int g, int s)
                {
                #if BFLOAT_ERRHANDLING
                        value += (R==std::round_to_nearest) ? (g&(s|value)) :
                          (R==std::round_toward_infinity) ? (~(value>>15)&(g|s)) :
                          (R==std::round_toward_neg_infinity) ? ((value>>15)&(g|s)) : 0;
                        if((value&0x7F80) == 0x7F80)
                                raise(FE_OVERFLOW);
                        else if(value & 0x7F80)
                                raise(FE_INEXACT, I || (g|s)!=0);
                        else
                                raise(FE_UNDERFLOW, !(BFLOAT_ERRHANDLING_UNDERFLOW_TO_INEXACT) || I || (g|s)!=0);
                        return value;
                #else
                        return (R==std::round_to_nearest) ? (value+(g&(s|value))) :
                          (R==std::round_toward_infinity) ? (value+(~(value>>15)&(g|s))) :
                          (R==std::round_toward_neg_infinity) ? (value+((value>>15)&(g|s))) : value;
                #endif
                }
        
                /// Round bfloat16-precision number to nearest integer value.
                /// \tparam R rounding mode to use
                /// \tparam E `true` for round to even, `false` for round away from zero
                /// \tparam I `true` to raise INEXACT exception (if inexact), `false` to never raise it
                /// \param value bfloat16-precision value to round
                /// \return bfloat16-precision bits for nearest integral value
                /// \exception FE_INVALID for signaling NaN
                /// \exception FE_INEXACT if value had to be rounded and \a I is `true`
                template<std::float_round_style R,bool E,bool I> unsigned int integral(unsigned int value)
                {
                        unsigned int abs = value & 0x7FFF;
                        if(abs < 0x3F80)
                        {
                                raise(FE_INEXACT, I);
                                return ((R==std::round_to_nearest) ? (0x3F80&-static_cast<unsigned>(abs>=(0x3F00+E))) :
                                  (R==std::round_toward_infinity) ? (0x3F80&-(~(value>>15)&(abs!=0))) :
                                  (R==std::round_toward_neg_infinity) ? (0x3F80&-static_cast<unsigned>(value>0x8000)) :
                                   0) | (value&0x8000);
                        }
                        if(abs >= 0x6080)
                                return (abs>0x7F80) ? signal(value) : value;
                        unsigned int exp = 70 - (abs>>7), mask = (1<<exp) - 1 ;
                        raise(FE_INEXACT, I && (value&mask));
			//std::cout<<std::endl; //without it, for some reason it does not calculate it
                        return ~((( (R==std::round_to_nearest) ? ((1<<(exp-1))-(~(value>>exp)&E)) :
                          (R==std::round_toward_neg_infinity) ? (mask&((value>>15)-1)) :
                          (R==std::round_toward_infinity) ? (mask&-(value>>15)) :
                          0) - value) & ~mask) + 1;
                }
        
                /// Convert fixed point to bfloat16-precision floating-point.
                /// \tparam R rounding mode to use
                /// \tparam F number of fractional bits (at least 11)
                /// \tparam S `true` for signed, `false` for unsigned
                /// \tparam N `true` for additional normalization step, `false` if already normalized to 1.F
                /// \tparam I `true` to always raise INEXACT exception, `false` to raise only for rounded results
                /// \param m mantissa in Q1.F fixed point format
                /// \param exp exponent
                /// \param sign bfloat16-precision value with sign bit only
                /// \param s sticky bit (or of all but the most significant already discarded bits)
                /// \return value converted to bfloat16-precision
                /// \exception FE_OVERFLOW on overflows
                /// \exception FE_UNDERFLOW on underflows
                /// \exception FE_INEXACT if value had to be rounded or \a I is `true`
                template<std::float_round_style R,unsigned int F,bool S,bool N,bool I> unsigned int fixed2bfloat16(uint32 m, int exp = 14, unsigned int sign = 0, int s = 0)
                {
                        if(S)
                        {
                                uint32 msign = sign_mask(m);
                                m = (m^msign) - msign;
                                sign = msign & 0x8000;
                        }
                        if(N)
                                for(; m<(static_cast<uint32>(1)<<F) && exp; m<<=1, --exp);
                        else if(exp < 0)
                                return rounded<R,I>(sign+(m>>(F-7-exp)), (m>>(F-8-exp))&1,
                                       s|((m&((static_cast<uint32>(1)<<(F-8-exp))-1))!=0));
                        return rounded<R,I>(sign+(exp<<7)+(m>>(F-7)), (m>>(F-8))&1,
                               s|((m&((static_cast<uint32>(1)<<(F-8))-1))!=0));
                }
        
                /// Convert IEEE single-precision to bfloat16-precision.
                /// \tparam R rounding mode to use
                /// \param value single-precision value to convert
                /// \return rounded float16-precision value
                /// \exception FE_OVERFLOW on overflows
                /// \exception FE_UNDERFLOW on underflows
                /// \exception FE_INEXACT if value had to be rounded
                template<std::float_round_style R> unsigned int float2bfloat16_impl(float value, true_type)
                {
                        bits<float>::type fbits;
                        std::memcpy(&fbits, &value, sizeof(float));
                        unsigned int sign = (fbits>>16) & 0x8000;
                        fbits &= 0x7FFFFFFF;
                        if(fbits >= 0x7F800000)
                                return sign | 0x7F80 | ((fbits>0x7F800000) ? (0x40|((fbits>>13)&0x7F)) : 0);
                        if(fbits >= 0x47800000)
                                return overflow<R>(sign);
			if(fbits >= 0x38800000){
                               return ((fbits>>16)|sign);}
                        if(fbits >= 0x33000000)
                        {
                                int i = 125 - (fbits>>23);
                                fbits = (fbits&0x7FFFFF) | 0x800000;
                                return rounded<R,false>(sign|(fbits>>(i+1)), (fbits>>i)&1, (fbits&((static_cast<uint32>(1)<<i)-1))!=0);
                        }
                        if(fbits != 0)
                                return underflow<R>(sign);
                        return sign;
                  
                }
          
                /// Convert IEEE double-precision to bfloat16-precision.
                /// \tparam R rounding mode to use
                /// \param value double-precision value to convert
                /// \return rounded bfloat16-precision value
                /// \exception FE_OVERFLOW on overflows
                /// \exception FE_UNDERFLOW on underflows
                /// \exception FE_INEXACT if value had to be rounded
                template<std::float_round_style R> unsigned int float2bfloat16_impl(double value, true_type)
                {

                        bits<double>::type dbits;
                        std::memcpy(&dbits, &value, sizeof(double));
                        uint32 hi = dbits >> 32, lo = dbits & 0xFFFFFFFF;
                        unsigned int sign = (hi>>16) & 0x8000;
                        hi &= 0x7FFFFFFF;
                        if(hi >= 0x7FF00000)
                                return sign | 0x7F80 | ((dbits&0xFFFFFFFFFFFFF) ? (0x40|((hi>>10)&0x7F)) : 0);
                        if(hi >= 0x40F00000)
                                return overflow<R>(sign);
                        if(hi >= 0x3F100000)
			
                                return rounded<R,false>(sign|(((hi>>20)-1008)<<10)|((hi>>10)&0x7F), (hi>>9)&1, ((hi&0x3F)|lo)!=0);
                        if(hi >= 0x3E600000)
                        {
                                int i = 1018 - (hi>>20);
                                hi = (hi&0xFFFFF) | 0x100000;
                                return rounded<R,false>(sign|(hi>>(i+1)), (hi>>i)&1,((hi&((static_cast<uint32>(1)<<i)-1))|lo)!=0);
                        }
                        if((hi|lo) != 0)
                                return underflow<R>(sign);
                        return sign;
                }

                /// Convert non-IEEE floating-point to bfloat16-precision.
                /// \tparam R rounding mode to use
                /// \tparam T source type (builtin floating-point type)
                /// \param value floating-point value to convert
                /// \return rounded bfloat16-precision value
                /// \exception FE_OVERFLOW on overflows
                /// \exception FE_UNDERFLOW on underflows
                /// \exception FE_INEXACT if value had to be rounded
                template<std::float_round_style R,typename T> unsigned int float2bfloat16_impl(T value, ...)
                {
                        unsigned int hbits = static_cast<unsigned>(builtin_signbit(value)) << 15;
                        if(value == T())
                                return hbits;
                        if(builtin_isnan(value))
                                return hbits | 0x7FFF;
                        if(builtin_isinf(value))
                                return hbits | 0x7F80;
                        int exp;
                        std::frexp(value, &exp);
                        if( exp > 128)
                                return overflow<R>(hbits);
                        if(exp < -125)
                                value = std::ldexp(value, 25);
                        else
                        {
                                value = std::ldexp(value, 124-exp);
                                hbits |= ((exp+125)<<7);
                        }
                        T ival, frac = std::modf(value, &ival);
                        int m = std::abs(static_cast<int>(ival));
                        return rounded<R,false>(hbits+(m>>1), m&1, frac!=T());
                }
        
                /// Convert floating-point to bfloat16-precision.
                /// \tparam R rounding mode to use
                /// \tparam T source type (builtin floating-point type)
                /// \param value floating-point value to convert
                /// \return rounded bfloat16-precision value
                /// \exception FE_OVERFLOW on overflows
                /// \exception FE_UNDERFLOW on underflows
                /// \exception FE_INEXACT if value had to be rounded
                template<std::float_round_style R,typename T> unsigned int float2bfloat16(T value)
                {
                        return float2bfloat16_impl<R>(value, bool_type<std::numeric_limits<T>::is_iec559&&sizeof(typename bits<T>::type)==sizeof(T)>());
                }
        
                /// Convert integer to bfloat16-precision floating-point.
                /// \tparam R rounding mode to use
                /// \tparam T type to convert (builtin integer type)
                /// \param value integral value to convert
                /// \return rounded bfloat16-precision value
                /// \exception FE_OVERFLOW on overflows
                /// \exception FE_INEXACT if value had to be rounded
                template<std::float_round_style R,typename T> unsigned int int2bfloat16(T value)
                {
                        unsigned int bits = static_cast<unsigned>(value<0) << 15;
                        if(!value)
                                return bits;
                        if(bits)
                                value = -value;
                        if(value > 0xFFFF)
                                return overflow<R>(bits);
                        unsigned int m = static_cast<unsigned int>(value), exp = 133;
                        for(; m<0x80; m<<=1,--exp) ;
                        for(; m>0xFF; m>>=1,++exp) ;
                        bits |= (exp<<7) + m;
                        return (exp>133) ? rounded<R,false>(bits, (value>>(exp-134))&1, (((1<<(exp-134))-1)&value)!=0) : bits;
                }
        

		/// temp Function for conversion, because compiler converts bfloat intern for some reason, so sometimes it converts bfloat16 -> float32 -> float32, and because of bitshift it destroys the value
		// \param value bfloat16-precision value to convert
		// \return single-precision value
		// bfloat16 -> float32.
		inline float bfloat162float_temp(unsigned int value)
		{

			bits<float>::type fbits = static_cast<bits<float>::type>(value) << 16;
			float out;
			std::memcpy(&out, &fbits, sizeof(float));
			return out;

		}





                /// Convert bfloat16-precision to IEEE single-precision.
                /// \param value bfloat16-precision value to convert
                /// \return single-precision value
                inline float bfloat162float_impl(unsigned int value, float, true_type)
                {
			//bits<float>::type fbits = static_cast<bits<float>::type>(value) <<16;
			bits<float>::type out;
			//float out;
			//std::memcpy(&out, &fbits, sizeof(float));
		        std::memcpy(&out, &value, sizeof(float));
		        return out;
                }
        
                /// Convert bfloat16-precision to IEEE double-precision.
                /// \param value bfloat16-precision value to convert
                /// \return double-precision value
                inline double bfloat162float_impl(unsigned int value, double, true_type)
                {
                        uint32 hi = static_cast<uint32>(value&0x8000) << 16;
                        unsigned int abs = value & 0x7FFF;
                        if(abs)
                        {
                                hi |= 0x3F000000 << static_cast<unsigned>(abs>=0x7C00);
                                for(; abs<0x80; abs<<=1,hi-=0x100000) ;
                                hi += static_cast<uint32>(abs) << 10;
                        }
                        bits<double>::type dbits = static_cast<bits<double>::type>(hi) << 32;
                        double out;
                        std::memcpy(&out, &dbits, sizeof(double));
                        return out;
                }
        
                /// Convert bfloat16-precision to non-IEEE floating-point.
                /// \tparam T type to convert to (builtin integer type)
                /// \param value bfloat16-precision value to convert
                /// \return floating-point value
                template<typename T> T bfloat162float_impl(unsigned int value, T, ...)
                {
                  T out;
                  unsigned int abs = value & 0x7FFF;
                  if(abs > 0x7F80)
                          out = (std::numeric_limits<T>::has_signaling_NaN && !(abs&0x40)) ? std::numeric_limits<T>::signaling_NaN() :
                              std::numeric_limits<T>::has_quit_NaN ? std::numeric_limits<T>::quit_NaN() : T();
                  else if(abs == 0x7F80)
                          out = std::numeric_limits<T>::has_infinity ? std::numeric_limits<T>::infinity() : std::numeric_limits<T>::max();
                  else if(abs >0x7F)
                          out = std::ldexp(static_cast<T>((abs&0x7F)|0x80), (abs>>10)-25);
                  else
                          out = std::ldexp(static_cast<T>(abs), -24);
                  return (value&0x8000) ? -out : out;
                }
        
                /// Convert bfloat16-precision to floating-point.
                /// \tparam T type to convert to (builtin integer type)
                /// \param value bfloat16-precision value to convert
                /// \return floating-point value
                template<typename T> T bfloat162float(unsigned int value)
                {
                        return bfloat162float_impl(value, T(), bool_type<std::numeric_limits<T>::is_iec559&&sizeof(typename bits<T>::type)==sizeof(T)>());
                }

                /// Convert bfloat16-precision floating-point to integer.
                /// \tparam R rounding mode to use
                /// \tparam E `true` for round to even, `false` for round away from zero
                /// \tparam I `true` to raise INEXACT exception (if inexact), `false` to never raise it
                /// \tparam T type to convert to (buitlin integer type with at least 16 bits precision, excluding any implicit sign bits)
                /// \param value bfloat16-precision value to convert
                /// \return rounded integer value
                /// \exception FE_INVALID if value is not representable in type \a T
                /// \exception FE_INEXACT if value had to be rounded and \a I is `true`
                template<std::float_round_style R,bool E,bool I,typename T> T bfloat162int(unsigned int value)
                {
                        unsigned int abs = value & 0x7FFF;
                        if(abs >= 0x7F80)
                        {
                                raise(FE_INVALID);
                                return (value&0x8000) ? std::numeric_limits<T>::min() : std::numeric_limits<T>::max();
                        }
                        if(abs < 0x3F00)
                        {
                                raise(FE_INEXACT, I);
                                return (R==std::round_toward_infinity) ? T(~(value>>15)&(abs!=0)) :
                                       (R==std::round_toward_neg_infinity) ? - T(value>0x8000) : T();
                        }
                        int exp = 134 - (abs>>7);
                        unsigned int m = (value&0x7F) | 0x80;
                        int32 i = static_cast<int32>((exp<=0) ? (m<<-exp) : ((m+(
                          (R==std::round_to_nearest) ? ((1<<(exp-1))-(~(m>>exp)&E)) :
                          (R==std::round_toward_infinity) ? (((1<<exp)-1)&((value>>15)-1)) :
                          (R==std::round_toward_neg_infinity) ? ((( 1<<exp)-1)&-(value>>15)) : 0))>>exp));
                        if((!std::numeric_limits<T>::is_signed && (value&0x8000)) ||
                                   (std::numeric_limits<T>::digits<16 &&
                                   ((value&0x8000) ? (-i<std::numeric_limits<T>::min()) :                                                (i>std::numeric_limits<T>::max()))))
                                raise(FE_INVALID);
                        else if(I && exp > 0 && (m&((1<<exp)-1)))
                                raise(FE_INEXACT);
                        return static_cast<T>((value&0x8000) ? -i : i);
                }
        
        
                /// \}
                /// \name Mathematics
                /// \{

                /// upper part of 64-bit multiplication.
                /// \tparam R rounding mode to use
                /// \param x first factor
                /// \param y second factor
                /// \return upper 32 bit of \a x * \a y
                template<std::float_round_style R> uint32 mulhi(uint32 x, uint32 y)
                {
                        uint32 xy = (x>>16) * (y&0xFFFF), yx = (x&0xFFFF) * (y>>16), c= (xy&0xFFFF) + (yx&0xFFFF) + (((x&0xFFFF)*(y&0xFFFF))>>16);
                        return (x>>16)*(y>>16) + (xy>>16) + (yx>>16) + (c>>16) +
                          ((R==std::round_to_nearest) ? ((c>>15)&1) :
                           (R==std::round_toward_infinity) ? ((c&0xFFFF)!=0) : 0);
                }
        
                /// 64-bit multiplication.
                /// \param x first factor
                /// \param y second factor
                /// \return upper 32 bit of \a x * \a y rounded to nearest
                inline uint32 multiply64(uint32 x, uint32 y)
                {
                #if BFLOAT_ENABLE_CPP11_LONG_LONG
                        return static_cast<uint32>((static_cast<unsigned long long>(x)*static_cast<unsigned long long>(y)+0x80000000)>>32);
                #else

                        return mulhi<std::round_to_nearest>(x,y);
                #endif
                }
        
                /// 64-bit division.
                /// \param x upper 32 bit of dividend
                /// \param y divisor
                /// \param s variable to store sticky bit for rounding
                /// \return (\a x << 32) / \a y
                inline uint32 divide64(uint32 x, uint32 y, int &s)
                {

                #if BFLOAT_ENABLE_CPP11_LONG_LONG
                        unsigned long long xx = static_cast<unsigned long long>(x) << 32;
                        return s = (xx%y!=0), static_cast<uint32>(xx/y);
                #else

                        y >>= 1;
                        uint32 rem = x, div = 0;
                        for(unsigned int i=0; i <32; ++i)
                        {
                                div <<= 1;
                                if(rem >= y)
                                {
                                        rem -= y;
                                        div |= 1;
                                }
                                rem <<= 1;
                        }
                        return s = rem > 1, div;
                #endif
                }
        
                /// Bfloat16 precision positive modulus.
                /// \tparam Q `true` to compute full quotient, `false` else
                /// \tparam R `true` to compute signed remainder, `false` for positive remainder
                /// \param x first operand as positive finite bfloat16-precision value
                /// \param y second operand as positive finite bfloat16-precision value
                /// \param quo adress to store quotient at, `nullptr` if \a Q `false`
                /// \return modulus of \a x / \a y
                template<bool Q,bool R> unsigned int mod(unsigned int x, unsigned int y, int *quo = NULL)
                {
                        unsigned int q = 0;
                        if(x > y)
                        {
                                int absx = x, absy = y, expx = 0, expy = 0;
                                for(; absx<0x80; absx<<=1,--expx) ;
                                for(; absy<0x80; absy<<=1,--expy) ;
                                expx += absx >> 7;
                                expy += absy >> 7;
                                int mx = (absx&0x7F) | 0x80, my = (absy&0x7F) | 0x80;
                                for(int d=expx-expy; d; --d)
                                {
                                        if(!Q && mx == my)
                                                return 0;
                                        if(mx >= my)
                                        {
                                                mx -= my;
                                                q += Q;
                                        }
                                        mx <<= 1;
                                        q <<= static_cast<int>(Q);
                                }
                                if(!Q && mx == my)
                                        return 0;
                                if(mx >= my)
                                {
                                        mx -= my;
                                        ++q;
                                }
                                if(Q)
                                {
                                        q &= (1<<(std::numeric_limits<int>::digits-1)) - 1;
                                        if(!mx)
                                                return *quo = q, 0;
                                }
                                for(; mx<0x80; mx<<=1,--expy) ;
                                x = (expy>0) ? ((expy<<7)|(mx&0x7F)) : (mx>>(1-expy));
                        }
                        if(R)
                        {
                                unsigned int a, b;
                                if(y < 0x100)
                                {
                                        a = (x<0x80) ? (x<<1) : (x+0x80);
                                        b = y;
                                }
                                else
                                {
                                        a = x;
                                        b = y - 0x80;
                                }
                                if(a > b || (a == b && (q&1)))
                                {
                                        int exp = (y>>7) + (y<=0x7F), d = exp - (x>>7) - (x<=0x7F);
                                        int m = (((y&0x7F)|((y>0x7F)<<7))<<1) - (((x&0x7F)|((x>0x7F)<<7))<<(1-d));
                                        for(; m<0x100 && exp>1; m<<=1,--exp) ;
                                        x = 0x8000 + ((exp-1)<<7) + (m>>1);
                                        q += Q;
                                }
                        }
                        if(Q)
                                *quo = q;
                        return x;
                }
        
                /// Fixed point square root.
                /// \tparam F number of fractional bits
                /// \param r radicand in Q1.F fixed point format
                /// \param exp exponent
                /// \return square root as Q1.F/2
                template<unsigned int F> uint32 sqrt(uint32 &r, int &exp)
                {
                        int i = exp & 1;
                        r <<= i;
                        exp = (exp-i) / 2;
                        uint32 m = 0;
                        for(uint32 bit=static_cast<uint32>(1)<<F; bit; bit>>=2)
                        {
                                if(r < m+bit)
                                        m >>= 1;
                                else
                                {
                                        r -= m + bit;
                                        m = (m>>1) + bit;
                                }
                        }
                        return m;
                }

                /// Fixed point binary exponential.
                /// This uses the BKM algorithm in E-mode.
                /// \param m exponent in [0,1) as Q0.31
                /// \param n number of iterations (at most 32)
                /// \return 2 ^ \a m as Q1.31
                inline uint32 exp2(uint32 m, unsigned int n = 32)
                {
                        static const uint32 logs[] = {
                                     0x80000000, 0x4AE00D1D, 0x2934F098, 0x15C01A3A, 0x0B31FB7D, 0x05AEB4DD, 0x02DCF2D1, 0x016FE50B,
                                     0x00B84E23, 0x005C3E10, 0x002E24CA, 0x001713D6, 0x000B8A47, 0x0005C53B, 0x0002E2A3, 0x00017153,
                                     0x0000B8AA, 0x00005C55, 0x00002E2B, 0x00001715, 0x00000B8B, 0x000005C5, 0x000002E3, 0x00000171,
                                     0x000000B9, 0x0000005C, 0x0000002E, 0x00000017, 0x0000000C, 0x00000006, 0x00000003, 0x00000001 };
                        if(!m)
                                return 0x80000000;
                        uint32 mx = 0x80000000, my = 0;
                        for(unsigned int i=1; i<n; ++i)
                        {
                                uint32 mz = my + logs[i];
                                if(mz <= m)
                                {
                                        my = mz;
                                        mx += mx >> i;
                                }
                        }
                        return mx;
                }
        
                /// Fixed point binary logarithm.
                /// This uses the BKM algorithm in L-mode.
                /// \param m mantissa in [1,2) as Q1.30
                /// \param n number of iterations (at most 32)
                /// \return log2(\a m) as Q0.31
                inline uint32 log2(uint32 m, unsigned int n = 32)
                {
                        static const uint32 logs[] = {
                                   0x80000000, 0x4AE00D1D, 0x2934F098, 0x15C01A3A, 0x0B31FB7D, 0x05AEB4DD, 0x02DCF2D1, 0x016FE50B,
                                   0x00B84E23, 0x005C3E10, 0x002E24CA, 0x001713D6, 0x000B8A47, 0x0005C53B, 0x0002E2A3, 0x00017153,
                                   0x0000B8AA, 0x00005C55, 0x00002E2B, 0x00001715, 0x00000B8B, 0x000005C5, 0x000002E3, 0x00000171,
                                   0x000000B9, 0x0000005C, 0x0000002E, 0x00000017, 0x0000000C, 0x00000006, 0x00000003, 0x00000001 };
                        if(m == 0x40000000)
                                return 0;
                        uint32 mx = 0x40000000, my = 0;
                        for(unsigned int i=1; i<n; ++i)
                        {
                                uint32 mz = mx + (mx>>i);
                                if(mz <= m)
                                {
                                        mx = mz;
                                        my += logs[i];
                                }
                        }
                        return my;
                }
        
                /// Fixed point sine and cosine.
                /// This uses the CORDIC algorithm in rotation mode.
                /// \param mz angle in [-pi/2,pi/2] as Q1.30
                /// \param n number of iterations (at most 31)
                /// \return sine and cosine of \a mz as Q1.30
                inline std::pair<uint32,uint32> sincos(uint32 mz, unsigned int n = 31)
                {
                        static const uint32 angles[] = {
                                     0x3243F6A9, 0x1DAC6705, 0x0FADBAFD, 0x07F56EA7, 0x03FEAB77, 0x01FFD55C, 0x00FFFAAB, 0x007FFF55,
                                     0x003FFFEB, 0x001FFFFD, 0x00100000, 0x00080000, 0x00040000, 0x00020000, 0x00010000, 0x00008000,
                                     0x00004000, 0x00002000, 0x00001000, 0x00000800, 0x00000400, 0x00000200, 0x00000100, 0x00000080,
                                     0x00000040, 0x00000020, 0x00000010, 0x00000008, 0x00000004, 0x00000002, 0x00000001 };
                        uint32 mx = 0x26DD3B6A, my = 0;
                        for(unsigned int i=0; i<n; ++i)
                        {
                                uint32 sign = sign_mask(mz);
                                uint32 tx = mx - (arithmetic_shift(my, i)^sign) + sign;
                                uint32 ty = my - (arithmetic_shift(mx, i)^sign) - sign;
                                mx = tx; my = ty; mz -= (angles[i]^sign) - sign;
                        }
                        return std::make_pair(my, mx);
                }
        
                /// Fixed point arc tangent.
                /// This uses the CORDIC algorithm in vectoring mode.
                /// \param my y coordinate as Q0.30
                /// \param mx x coordinate as Q0.30
                /// \param n number of iterations (at most 31)
                /// \return arc tangent of \a my / \a mx as Q1.30
                inline uint32 atan2(uint32 my, uint32 mx, unsigned int n = 31)
                {
                        static const uint32 angles[] = {
                                     0x3243F6A9, 0x1DAC6705, 0x0FADBAFD, 0x07F56EA7, 0x03FEAB77, 0x01FFD55C, 0x00FFFAAB, 0x007FFF55,
                                     0x003FFFEB, 0x001FFFFD, 0x00100000, 0x00080000, 0x00040000, 0x00020000, 0x00010000, 0x00008000,
                                     0x00004000, 0x00002000, 0x00001000, 0x00000800, 0x00000400, 0x00000200, 0x00000100, 0x00000080,
                                     0x00000040, 0x00000020, 0x00000010, 0x00000008, 0x00000004, 0x00000002, 0x00000001 };
                        uint32 mz = 0;
                        for(unsigned int i=0; i<n;  ++i)
                        {
                          uint32 sign = sign_mask(my);
                          uint32 tx = mx + (arithmetic_shift(my, i)^sign) - sign;
                          uint32 ty = my - (arithmetic_shift(mx, i)^sign) + sign;
                          mx = tx; my = ty; mz += (angles[i]^sign) - sign;
                        }
                        return mz;
                }
        
                /// Reduce argument for trigonometric functions.
                /// \param abs bfloat16-precision floating-point value
                /// \param k value to take quarter period
                /// \return \a abs reduced to [-pi/4,pi/4] as Q0.30
                inline uint32 angle_arg(unsigned int abs, int &k)
                {
                        uint32 m = (abs&0x3FF) | ((abs>0x3FF)<<10);
                        int exp = (abs>>10) + (abs<=0x3FF) - 15;
                        if(abs < 0x3A48)
                                return k = 0, m << (exp+20);
                #if BFLOAT_ENABLE_CPP11_LONG_LONG
                        unsigned long long y = m * 0xA2F9836E4E442, mask = (1ULL<<(62-exp)) - 1, yi = (y+(mask>>1)) & ~mask, f = y - yi;
                        uint32 sign = -static_cast<uint32>(f>>63);
                        k = static_cast<int>(yi>>(62-exp));
                        return (multiply64(static_cast<uint32>((sign ? -f : f)>>(31-exp)),0xC90FDAA2)^sign)-sign;
                #else
                        uint32 yh = m*0xA2F98 + mulhi<std::round_toward_zero>(m, 0x36E4E442), yl = (m*0x36E4E442) & 0xFFFFFFFF;
                        uint32 mask = (static_cast<uint32>(1)<<(30-exp)) - 1, yi = (yh+(mask>>1)) & ~mask, sign = -static_cast<uint32>(yi>yh);
                        k = static_cast<int>(yi>>(30-exp));
                        uint32 fh = (yh^sign) + (yi^~sign) - ~sign, fl = (yl^sign) - sign;
                        return (multiply64((exp>-1) ? (((fh<<(1+exp))&0xFFFFFFFF)|((fl&0xFFFFFFFF)>>(31-exp))) : fh, 0xC90FDAA2)^sign) - sign;
                #endif
                }
        
                /// Get arguments for atan2 function.
                /// \param abs bfloat16-precision floating-point value
                /// \return \a abs and sqrt(1 - \a abs^2) as Q0.30
                inline std::pair<uint32,uint32> atan2_arg(unsigned int abs)
                {
                        int exp = -15;
                        for(; abs<0x80; abs<<=1,--exp) ;
                        exp += abs >> 10;
                        uint32 my = ((abs&0x7F)|0x80) << 5, r = my * my;
                        int rexp = 2 * exp;
                        r = 0x40000000 - ((rexp>-31) ? ((r>>-rexp) |((r&((static_cast<uint32>(1)<<-rexp)-1))!=0)) : 1);
                        for(rexp=0; r<0x40000000; r<<=1,--rexp) ;
                        uint32 mx = sqrt<30>(r, rexp);
                        int d = exp - rexp;
                        if(d < 0)
                                  return std::make_pair((d<-14) ? ((my>>(-d-14))+((my>>(-d-15))&1)) :
                                                (my<<(14+d)), (mx<<14)+(r<<13)/mx);
                        if(d > 0)
                                  return std::make_pair(my<<14, (d>14) ? ((mx>>(d-14))+((mx>>(d-15))&1)) :
                                                ((d==14) ? mx : ((mx<<(14-d))+(r<<(13-d))/mx)));
                        return std::make_pair(my<<13, (mx<<13)+(r<<12)/mx);
                }

                /// Get exponentials for hyperbolic computation
                /// \param abs bfloat16-precision floating-point value
                /// \param exp variable to take unbiased exponent of larger result
                /// \param n number of BKM iterations (at most 32)
                /// \return exp(abs) and exp(-\a abs) as Q1.31 with same exponent
                inline std::pair<uint32,uint32> hyperbolic_args(unsigned int abs, int &exp, unsigned int n = 32)
                {
                        uint32 mx = detail::multiply64(static_cast<uint32>((abs&0x3FF)+((abs>0x7F)<<10))<<21, 0xB8AA3B29), my;
                        int e = (abs>>10) + (abs<=0x7F);
                        if(e < 14)
                        {
                                exp = 0;
                                mx >>= 14 -e;
                        }
                        else
                        {
                                exp = mx >>(45-e);
                                mx = (mx<<(e-14)) & 0x7FFFFFFF;
                        }
                        mx = exp2(mx, n);
                        int d = exp << 1, s;
                        if(mx > 0x80000000)
                        {
                                my = divide64(0x80000000, mx, s);
                                my |= s;
                                ++d;
                        }
                        else
                                my = mx;
                        return std::make_pair(mx, (d<31) ? ((my>>d)|((my&((static_cast<uint32>(1)<<d)-1))!=0)) : 1);
                }
        
                /// Postprocessing for binary exponential.
                /// \tparam R rounding mode to use
                /// \tparam I `true` to always raise INEXACT exception, `false` to raise only for rounded results
                /// \param m mantissa as Q1.31
                /// \param exp absolute value of unbiased exponent
                /// \param esign sign of actual exponent
                /// \param sign sign bit of result
                /// \return value converted to bfloat16-precision
                /// \exception FE_OVERFLOW on overflows
                /// \exception FE_UNDERFLOW on underflows
                /// \exception FE_INEXACT if value had to be rounded or \a I is `true`
                template<std::float_round_style R,bool I> unsigned int exp2_post(uint32 m, int exp, bool esign, unsigned int sign = 0)
                {
                        int s = 0;
                        if(esign)
                        {
                                if(m > 0x80000000)
                                {
                                        m = divide64(0x80000000, m, s);
                                        ++exp;
                                }
                                if(exp > 25)
                                        return underflow<R>(sign);
                                else if(exp == 25)
                                        return rounded<R,I>(sign, 1, (m&0x7FFFFFFF)!=0);
                                exp = -exp;
                        }
                        else if(exp > 127)//15
                                return overflow<R>(sign);
                        return fixed2bfloat16<R,31,false,false,I>(m, exp+14, sign, s);
                }
        
                /// Postprocessing for binary logarithm.
                /// \tparam R rounding mode to use
                /// \tparam L logarithm for base transformation as Q1.31
                /// \param m fractional part of logarithm as Q0.31
                /// \param ilog signed integer part of logarithm
                /// \param exp biased exponent of result
                /// \param sign sign bit of result
                /// \return value base-transformed and converted to bfloat16-precision
                /// \exception FE_OVERFLOW on overflows
                /// \exception FE_UNDERFLOW on underflows
                /// \exception FE_INEXACT if no other exception occurred
                template<std::float_round_style R, uint32 L> unsigned int log2_post(uint32 m, int ilog, int exp, unsigned int sign = 0)
                {
                        uint32 msign = sign_mask(ilog);
                        m = (((static_cast<uint32>(ilog)<<27)+(m>>4))^msign) - msign;
                        if(!m)
                                return 0;
                        for(; m<0x80000000; m<<=1,--exp);
                        int i = m >= L, s;
                        exp += i;
                        m >>= 1 + i;
                        sign ^= msign & 0x8000;
                        if(exp < -11)
                                return underflow<R>(sign);
                        m = divide64(m, L, s);
                        return fixed2bfloat16<R,30,false,false,true>(m, exp, sign, 1);
                }
        
                /// Hypotenuse square root and postprocessing.
                /// \tparam R rounding mode to use
                /// \param r mantissa as Q2.30
                /// \param exp unbiased exponent
                /// \return square root converted to bfloat16-precision
                /// \exception FE_OVERFLOW on overflows
                /// \exception FE_UNDERFLOW on underflows
                /// \exception FE_INEXACT if value had to be rounded
                template<std::float_round_style R> unsigned int hypot_post(uint32 r, int exp)
                {
                        int i = r >> 31;
                        if((exp+=i) > 46)
                                return overflow<R>();
                        if(exp < -34)
                                return underflow<R>();
                        r = (r>>i) | (r&i);
                        uint32 m = sqrt<30>(r, exp+=15);
                        return fixed2bfloat16<R,15,false,false,false>(m, exp-1, 0, r!=0);
                }
        
                /// Division and postprocessing for tangents.
                /// \tparam R rounding mode to use
                /// \param my dividend as Q1.31
                /// \param mx divisor as Q1.31
                /// \param exp biased exponent of result
                /// \param sign sign bit of result
                /// \return quotient converted to bfloat16-precision
                /// \exception FE_OVERFLOW on overflows
                /// \exception FE_UNDERFLOW on underflows
                /// \exception FE_INEXACT if no other exception occurred
                template<std::float_round_style R> unsigned int tangent_post(uint32 my, uint32 mx, int exp, unsigned int sign = 0)
                {
                        int i = my >= mx, s;
                        exp += i;
                        if(exp > 29)
                                return overflow<R>(sign);
                        if(exp < -11)
                                return underflow<R>(sign);
                        uint32 m = divide64(my>>(i+1), mx, s);
                        return fixed2bfloat16<R,30,false,false,true>(m, exp, sign, s);
                }
        
                /// Area function and postprocessing.
                /// This computes the value directly in Q2.30 using the representation `asinh|acosh(x) = log(x+sqrt(x^2+|-1))`.
                /// \tparam R rounding mode to use
                /// \tparam S `true` for asinh, `false` for acosh
                /// \param arg bfloat16-precision argument
                /// \return asinh|acosh(\a arg) converted to bfloat16-precision
                /// \exception FE_OVERFLOW on overflows
                /// \exception FE_UNDERFLOW on underflows
                /// \exception FE_INEXACT if no other exception occurred
                template<std::float_round_style R,bool S> unsigned int area(unsigned int arg)
                {
                        int abs  = arg & 0x7FFF, expx = (abs>>10) + (abs<=0x7F) - 15, expy = -15, ilog, i;
                        uint32 mx = static_cast<uint32>((abs&0x7F)|((abs>0x7F)<<10)) << 20, my, r;
                        for(; abs<0x80; abs<<=1,--expy) ;
                        expy += abs >> 10;
                        r = ((abs&0x7F)|0x80) << 5;
                        r *=r;
                        i =r >> 31;
                        expy = 2*expy + i;
                        r >>=i;
                        if(S)
                        {
                                if(expy < 0)
                                {
                                        r = 0x40000000 + ((expy>-30) ? ((r>>-expy)| ((r&((static_cast<uint32>(1)<<-expy)-1))!=0)) : 1);
                                        expy = 0;
                                }
                                else
                                {
                                        r += 0x40000000 >> expy;
                                        i = r >> 31;
                                        r = (r>>i) | (r&i);
                                        expy += i;
                                }
                        }
                        else
                        {
                                r -= 0x40000000 >> expy;
                                for(; r<0x40000000; i<<=1,--expy) ;
                        }
                        my = sqrt<30>(r, expy);
                        my = (my<<15) + (r<<14)/my;
                        if(S)
                        {
                                mx >>= expy - expx;
                                ilog = expy;
                        }
                        else
                        {
                                my >>= expx - expy;
                                ilog = expx;
                        }
                        my += mx;
                        i = my >> 31;
                        static const int G = S && (R==std::round_to_nearest);
                        return log2_post<R,0xB8AA3B2A>(log2(my>>i, 26+S+G)+(G<<3), ilog+i, 17, arg&(static_cast<unsigned>(S)<<15));
                }
        
                /// Class for 1.31 unsigned floating-point computation
                struct f31
                {
                        /// Constructor.
                        /// \param mant mantissa as 1.31
                        /// \param e exponent
                        BFLOAT_CONSTEXPR f31(uint32 mant, int e) : m(mant), exp(e) {}

                        /// Constructor.
                        /// \param abs unsigned bfloat16-precision value
                        f31(unsigned int abs) : exp(-15)
                        {
                                for(; abs<0x80; abs<<=1,--exp);
                                m = static_cast<uint32>((abs&0x7F)|0x80) << 24;
                                exp += (abs>>7);
                        }

                        /// Addition operator.
                        /// \param a first operand
                        /// \param b second operand
                        /// \return \a a + \a b
                        friend f31 operator+(f31 a, f31 b)
                        {
                                if(b.exp > a.exp)
                                        std::swap(a, b);
                                int d = a.exp - b.exp;
                                uint32 m = a.m + ((d<32) ? (b.m>>d) : 0);
                                int i = (m&0xFFFFFFFF) < a.m;
                                return f31(((m+i)>>i)|0x80000000, a.exp+i);
                        }

                        /// Subtraction operator.
                        /// \param a first operand
                        /// \param b second operand
                        /// \return \a a - \a b
                        friend f31 operator-(f31 a, f31 b)
                        {
                                int d = a.exp - b.exp, exp = a.exp;
                                uint32 m = a.m - ((d<32) ? (b.m>>d) : 0);
                                if(!m)
                                        return f31(0, -32);
                                for(; m<0x80000000; m<<=1,--exp) ;
                                return f31(m, exp);
                        }

                        /// Multiplication operator.
                        /// \param a first operand
                        /// \param b second operand
                        /// \return \a a * \a b
                        friend f31 operator*(f31 a, f31 b)
                        {
                                uint32 m = multiply64(a.m, b.m);
                                int i = m >> 31;
                                return f31(m<<(1-i), a.exp + b.exp + i);
                        }

                        /// Division operator.
                        /// \param a first operand
                        /// \param b second operand
                        /// \return \a a / \a b
                        friend f31 operator/(f31 a, f31 b)
                        {
                                int i = a.m >= b.m, s;
                                uint32 m = divide64((a.m+i)>>i, b.m,s);
                                return f31(m, a.exp - b.exp + i - 1);
                        }

                        uint32 m;             ///< mantissa as 1.31.
                        int exp;              ///< exponent.
                };
        
                /// \}
                /// \name Mathematics
                /// \{





                /// Error function and postprocessing.
                /// This computes the value directly in Q1.31 using the approximations given 
                /// [here](https://en.wikipedia.org/wiki/Error_function#Approximation_with_elementary_functions).
                /// \tparam R rounding mode to use
                /// \tparam C `true` for comlementary error function, `false` else
                /// \param arg bfloat16-precision function argument
                /// \return approximated value of error function in bfloat16-precision
                /// \exception FE_OVERFLOW on overflows
                /// \exception FE_UNDERFLOW on underflows
                /// \exception FE_INEXACT if no other exception occurred
                template<std::float_round_style R,bool C> unsigned int erf(unsigned int arg)
                {
                        unsigned int abs = arg & 0x7FFF, sign = arg & 0x8000;
                        f31 x(abs), x2 = x * x * f31(0xB8AA3B29, 0), t= f31(0x80000000, 0) / (f31(0x80000000, 0) +f31(0xA7BA054A, -2)*x), t2 = t * t;
                        f31 e = ((f31(0x87DC2213, 0)*t2+f31(0xB5F0E2AE, 0))*t2+f31(0x82790637, -2)- (f31(0xBA00E2B8, 0)*t2+f31(0x91A98E62, -2))*t) * t / ((x2.exp<0) ? f31(exp2((x2.exp>-32) ? (x2.m>>-x2.exp) : 0, 30), 0) : f31(exp2((x2.m<<x2.exp)&0x7FFFFFFFF, 22), x2.m>>(31-x2.exp)));
                        return (!C || sign) ? fixed2bfloat16<R,31,false,true,true>(0x80000000-(e.m>>(C-e.exp)), 14+C, sign&(C-1U)) :
                        (e.exp<-25) ? underflow<R>() : fixed2bfloat16<R,30,false,false,true>(e.m>>1, e.exp+14, 0, e.m&1);
                }
        
                /// Gamma function and postprocessing.
                /// This approximates the value of either the gamma function or its logarithm directly in Q1.31.
                /// \tparam R rounding mode to use
                /// \tparam L `true` for lograithm of gamma function, `false` for gamma function
                /// \param arg bfloat16-precision floating-point value
                /// \return lgamma/tgamma(\a arg) in bfloat16-precision
                /// \exception FE_OVERFLOW on overflows
                /// \exception FE_UNDERFLOW on underflows
                /// \exception FE_INEXACT if \a arg is not a positive integer
                template<std::float_round_style R,bool L> unsigned int gamma(unsigned int arg)
                {
                  /*
                  static const double p[] ={ 2.50662827563479526904, 225.525584619175212544, -268.295973841304927459, 80.9030806934622512966, -5.00757863970517583837, 0.0114684895434781459556 };
            double t = arg + 4.65, s = p[0];
            for(unsigned int i=0; i<5; ++i)
              s += p[i+1] / (arg+i);
            return std::log(s) + (arg-0.5)*std::log(t) - t;
                  */
                  static const f31 pi(0xC90FDAA2, 1), lbe(0xB8AA3B29, 0);
                  unsigned int abs = arg & 0x7FFF, sign = arg & 0x8000;
                  bool bsign = sign != 0;
                  f31 z(abs), x = sign ? (z+f31(0x80000000, 0)) : z, t = x + f31(0x94CCCCCD, 2),
                  s = f31(0xA06C9901, 1) + f31(0xBBE654E2, -7)/(x+f31(0x80000000, 2)) + f31(0xA1CE6098, 6)/
                    (x+f31(0x80000000, 1)) + f31(0xE1868CB7, 7)/x - f31(0x8625E279, 8)/
                    (x+f31(0x80000000, 0)) - f31(0xA03E158F, 2)/(x+f31(0xC0000000, 1));
                  int i = (s.exp>=2) +(s.exp>=4) + (s.exp>=8) + (s.exp>=16);
                  s = f31((static_cast<uint32>(s.exp)<<(31-i))+(log2(s.m>>1, 28)>>i), i) / lbe;
                  if(x.exp != -1 || x.m != 0x80000000)
                  {
                          i = (t.exp>=2) + (t.exp>=4) + (t.exp>=8);
                          f31 l = f31((static_cast<uint32>(t.exp)<<(31-i))+(log2(t.m>>1, 30)>>i), i) /lbe;
                          s = (x.exp<-1) ? (s-(f31(0x80000000, -1)-x)*l) : (s+(x-f31(0x80000000, -1))*l);
                  }
                  s = x.exp ? (s-t) : (t-s);
                  if(bsign)
                  {
                          if(z.exp >= 0)
                          {
                                  sign &= (L|((z.m>>(31-z.exp))&1)) - 1;
                                  for(z=f31((z.m<<(1+z.exp))&0xFFFFFFFF, -1); z.m<0x80000000; z.m<<=1,--z.exp) ;
                          }
                          if(z.exp == -1)
                                  z = f31(0x80000000, 0) - z;
                          if(z.exp < -1)
                          {
                                  z = z * pi;
                                  z.m = sincos(z.m>>(1-z.exp), 30).first;
                                  for(z.exp=1; z.m<0x80000000; z.m<<=1,--z.exp) ;
                          }
                          else
                                  z = f31(0x80000000, 0);
                  }
                  if(L)
                  {
                          if(bsign)
                          {
                                  f31 l(0x92868247, 0);
                                  if(z.exp < 0)
                                  {
                                          uint32 m = log2((z.m+1)>>1, 27);
                                          z = f31(-((static_cast<uint32>(z.exp)<<26)+(m>>5)), 5);
                                          for(; z.m<0x80000000; z.m<<=1,--z.exp) ;
                                          l = l + z / lbe;
                                  }
                                  sign = static_cast<unsigned>(x.exp&&(l.exp<s.exp||(l.exp==s.exp&&l.m<s.m))) << 15;
                                  s = sign ? (s-l) : x.exp ? (l-s) : (l+s);
                          }
                          else
                          {
                                  sign = static_cast<unsigned>(x.exp==0) << 15;
                                  if(s.exp < -24)
                                          return underflow<R>(sign);
                                  if(s.exp > 15)
                                          return overflow<R>(sign);
                          }
                  }
                  else
                  {
                          s = s * lbe;
                          uint32 m;
                          if(s.exp < 0)
                          {
                                  m = s.m >> -s.exp;
                                  s.exp = 0;
                          }
                          else
                          {
                                  m = (s.m<<s.exp) & 0x7FFFFFFF;
                                  s.exp = (s.m>>(31-s.exp));
                          }
                          s.m = exp2(m, 27);
                          if(!x.exp)
                                  s = f31(0x80000000, 0) / s;
                          if(bsign)
                          {
                                  if(z.exp < 0)
                                          s = s * z;
                                  s = pi / s;
                                  if(s.exp < -24)
                                          return underflow<R>(sign);
                          }
                          else if(z.exp > 0 && !(z.m&((1<<(31-z.exp))-1)))
                                  return ((s.exp+14)<<10) + (s.m>>21);
                          if(s.exp > 15)
                                  return overflow<R>(sign);
                  }
                  return fixed2bfloat16<R,31,false,false,true>(s.m, s.exp+14, sign);
                }
                /// \}

                template<typename,typename,std::float_round_style> struct bfloat16_caster;
        }
        /// BFloat-precision floating-point type.
        /// This class implements an IEEE-conformant bfloat16-precision floating-point type with the usual arithmetic 
        /// operators and conversions. It is implicitly convertible to single-precision floating-point, which makes artihmetic 
        /// expressions and functions with mixed-type operands to be of the most precise operand type.
        ///
        /// According to the C++98/03 definition, the float16 type is not a POD type. But according to C++11's less strict and 
        /// extended definitions it is both a standard layout type and a trivially copyable type (even if not a POD type), which 
        /// means it can be standard-conformantly copied using raw binary copies. But in this context some more words about the 
        /// actual size of the type. Although the bfloat16 is representing an IEEE 16-bit type, it does not neccessarily have to be of 
        /// exactly 16-bits size. But on any reasonable implementation the actual binary representation of this type will most 
        /// probably not involve any additional "magic" or padding beyond the simple binary representation of the underlying 16-bit 
        /// IEEE number, even if not strictly guaranteed by the standard. But even then it only has an actual size of 16 bits if 
        /// your C++ implementation supports an unsigned integer type of exactly 16 bits width. But this should be the case on 
        /// nearly any reasonable platform.
        ///
        /// So if your C++ implementation is not totally exotic or imposes special alignment requirements, it is a reasonable 
        /// assumption that the data of a bfloat16 is just comprised of the 2 bytes of the underlying IEEE representation.
        class bfloat16
        {
        public:
                /// \name Construction and assignment
                /// \{

                /// Default constructor.
                /// This initializes the bfloat16 to 0. Although this does not match the builtin types' default-initialization semantics 
                /// and may be less efficient than no initialization, it is needed to provide proper value-initialization semantics.
                BFLOAT_CONSTEXPR bfloat16() BFLOAT_NOEXCEPT : data_() {}

                /// Conversion constructor.
                /// \param rhs float to convert
                /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
                explicit bfloat16(float rhs) :
                data_(static_cast<detail::uint16>(detail::float2bfloat16<round_style>(rhs))) {}

                /// Conversion to single-precision.
                /// \return single precision value representing expression value
                operator float() const { return detail::bfloat162float<float>(data_); }

                /// Assignment operator.
                /// \param rhs single-precision value to copy from
                /// \return reference to this bfloat16
                /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
                bfloat16& operator=(float rhs) { data_ =
               static_cast<detail::uint16>(detail::float2bfloat16<round_style>(rhs)); return *this; }

                /// \}
                /// \name Arithmetic updates
                /// \{

                /// Arithmetic assignment.
                /// \tparam T type of concrete bfloat16 expression
                /// \param rhs bfloat16 expression to add
                /// \return reference to this bfloat16
                /// \exception FE_... according to operator+(float16,float16)
                bfloat16& operator+=(bfloat16 rhs) { return *this = * this + rhs; }
      
                /// Arithmetic assignment.
                /// \tparam T type of concrete bfloat16 expression
                /// \param rhs bfloat16 expression to subtract
                /// \return reference to this bfloat16
                /// \exception FE_... according to operator-(float16,float16)
                bfloat16& operator-=(bfloat16 rhs) { return *this = *this - rhs; }
      
                /// Arithmetic assignment.
                /// \tparam T type of concrete bfloat16 expression
                /// \param rhs bfloat16 expression to multiply with
                /// \return reference to this bfloat16
                /// \exception FE_... according to operator*(float16,float16)
                bfloat16& operator*=(bfloat16 rhs) { return *this = *this * rhs; }
      
                /// Arithmetic assignment.
                /// \tparam T type of concrete bfloat16 expression
                /// \param rhs bfloat16 expression to divide by
                /// \return reference to this bfloat16
                /// \exception FE_... according to operator/(float16,float16)
                bfloat16& operator/=(bfloat16 rhs) { return *this = *this / rhs; }
      
                /// Arithmetic assignment.
                /// \param rhs single-precision value to add
                /// \return reference to this bfloat16
                /// \exception FE_... according to operator=()
                bfloat16& operator+=(float rhs) { return *this = *this + rhs; }
      
                /// Arithmetic assignment.
                /// \param rhs single-precision value to subtract
                /// \return reference to this bfloat16
                /// \exception FE_... according to operator=()
                bfloat16& operator-=(float rhs) { return *this = *this - rhs; }
      
                /// Arithmetic assignment.
                /// \param rhs single-precision value to multiply with
                /// \return reference to this bfloat16
                /// \exception FE_... according to operator=()
                bfloat16& operator*=(float rhs) { return *this = *this * rhs; }
      
                /// Arithmetic assignment.
                /// \param rhs single-precision value to divide by
                /// \return reference to this bfloat16
                /// \exception FE_... according to operator=()
                bfloat16& operator/=(float rhs) { return *this = *this / rhs; }
      
                /// \}
                /// \name Increment and decrement
                /// \{

                /// Prefix increment.
                /// \return incremented bfloat16 value
                /// \exception FE_... according to operator+(bfloat16,bfloat16)
                bfloat16& operator++() { return *this = *this + bfloat16(detail::binary, 0x3F80); }
      
                /// Prefix decrement.
                /// \return decremented bfloat16 value
                /// \exception FE_... according to operator-(bfloat16,bfloat16)
                bfloat16& operator--() { return *this = *this + bfloat16(detail::binary, 0xBF80); }
      
                /// Postfix increment.
                /// \return non-incremented bfloat16 value
                /// \exception FE_... according to operator+(bfloat16,bfloat16)
                bfloat16 operator++(int) { bfloat16 out(*this); ++*this; return out; }
      
                /// Postfix decrement.
                /// \return non-decremented bfloat16 value
                /// \exception FE_... according to operator-(bfloat16,bfloat16)
                bfloat16 operator--(int) { bfloat16 out(*this); --*this; return out; }
                /// \}
      
      
        private:
                /// Rounding mode to use
                static const std::float_round_style round_style = (std::float_round_style)(BFLOAT_ROUND_STYLE);
      
                /// Constructor.
                /// \param bits binary representation to set bfloat16 to
                BFLOAT_CONSTEXPR bfloat16(detail::binary_t, unsigned int bits) BFLOAT_NOEXCEPT : data_(static_cast<detail::uint16>(bits)) {}

                /// Internal binary representation
                detail::uint16 data_;
      
        #ifndef BFLOAT_DOXYGEN_ONLY
                friend BFLOAT_CONSTEXPR_NOERR bool operator==(bfloat16, bfloat16);
                friend BFLOAT_CONSTEXPR_NOERR bool operator!=(bfloat16, bfloat16);
                friend BFLOAT_CONSTEXPR_NOERR bool operator<(bfloat16, bfloat16);
                friend BFLOAT_CONSTEXPR_NOERR bool operator>(bfloat16, bfloat16);
                friend BFLOAT_CONSTEXPR_NOERR bool operator<=(bfloat16, bfloat16);
                friend BFLOAT_CONSTEXPR_NOERR bool operator>=(bfloat16, bfloat16);
                friend BFLOAT_CONSTEXPR bfloat16 operator-(bfloat16);
                friend bfloat16 operator+(bfloat16, bfloat16);
                friend bfloat16 operator-(bfloat16, bfloat16);
                friend bfloat16 operator*(bfloat16, bfloat16);
                friend bfloat16 operator/(bfloat16, bfloat16);
                template<typename charT, typename traits> friend std::basic_ostream<charT,traits>&operator<<(std::basic_ostream<charT,traits>&, bfloat16);
                template<typename charT, typename traits> friend std::basic_istream<charT,traits>&operator>>(std::basic_istream<charT,traits>&, bfloat16&);
                friend BFLOAT_CONSTEXPR bfloat16 fabs(bfloat16);
                friend bfloat16 fmod(bfloat16, bfloat16);
                friend bfloat16 remainder(bfloat16, bfloat16);
                friend bfloat16 remquo(bfloat16, bfloat16, int*);
                friend bfloat16 fma(bfloat16, bfloat16, bfloat16);
                friend BFLOAT_CONSTEXPR_NOERR bfloat16 fmax(bfloat16, bfloat16);
                friend BFLOAT_CONSTEXPR_NOERR bfloat16 fmin(bfloat16, bfloat16);
                friend bfloat16 fdim(bfloat16, bfloat16);
                friend bfloat16 nanh(const char*);
                friend bfloat16 exp(bfloat16);
                friend bfloat16 exp2(bfloat16);
                friend bfloat16 expm1(bfloat16);
                friend bfloat16 log(bfloat16);
                friend bfloat16 log10(bfloat16);
                friend bfloat16 log2(bfloat16);
                friend bfloat16 log1p(bfloat16);
                friend bfloat16 sqrt(bfloat16);
                friend bfloat16 cbrt(bfloat16);
                friend bfloat16 hypot(bfloat16, bfloat16);
                friend bfloat16 hypot(bfloat16, bfloat16, bfloat16);
                friend bfloat16 pow(bfloat16, bfloat16);
                friend void sincos(bfloat16, bfloat16*, bfloat16*);
                friend bfloat16 sin(bfloat16);
                friend bfloat16 cos(bfloat16);
                friend bfloat16 tan(bfloat16);
                friend bfloat16 asin(bfloat16);
                friend bfloat16 acos(bfloat16);
                friend bfloat16 atan(bfloat16);
                friend bfloat16 atan2(bfloat16, bfloat16);
                friend bfloat16 sinh(bfloat16);
                friend bfloat16 cosh(bfloat16);
                friend bfloat16 tanh(bfloat16);
                friend bfloat16 asinh(bfloat16);
                friend bfloat16 acosh(bfloat16);
                friend bfloat16 atanh(bfloat16);
                friend bfloat16 erf(bfloat16);
                friend bfloat16 erfc(bfloat16);
                friend bfloat16 lgamma(bfloat16);
                friend bfloat16 tgamma(bfloat16);
                friend bfloat16 ceil(bfloat16);
                friend bfloat16 floor(bfloat16);
                friend bfloat16 trunc(bfloat16);
                friend bfloat16 round(bfloat16);
                friend long lround(bfloat16);
                friend bfloat16 rint(bfloat16);
                friend long lrint(bfloat16);
                friend bfloat16 nearbyint(bfloat16);
        #ifdef BFLOAT_ENABLE_CPP11_LONG_LONG
                friend long long llround(bfloat16);
                friend long long llrint(bfloat16);
        #endif
                friend bfloat16 frexp(bfloat16, int*);
                friend bfloat16 scalbln(bfloat16, long);
                friend bfloat16 modf(bfloat16, bfloat16*);
                friend int ilogb(bfloat16);
                friend bfloat16 logb(bfloat16);
                friend bfloat16 nextafter(bfloat16, bfloat16);
                friend bfloat16 nexttoward(bfloat16, long double);
                friend BFLOAT_CONSTEXPR bfloat16 copysign(bfloat16, bfloat16);
                friend BFLOAT_CONSTEXPR int fpclassify(bfloat16);
                friend BFLOAT_CONSTEXPR bool isfinite(bfloat16);
                friend BFLOAT_CONSTEXPR bool isinf(bfloat16);
                friend BFLOAT_CONSTEXPR bool isnan(bfloat16);
                friend BFLOAT_CONSTEXPR bool isnormal(bfloat16);
                friend BFLOAT_CONSTEXPR bool signbit(bfloat16);
                friend BFLOAT_CONSTEXPR bool isgreater(bfloat16, bfloat16);
                friend BFLOAT_CONSTEXPR bool isgreaterequal(bfloat16, bfloat16);
                friend BFLOAT_CONSTEXPR bool isless(bfloat16, bfloat16);
                friend BFLOAT_CONSTEXPR bool islessequal(bfloat16, bfloat16);
                friend BFLOAT_CONSTEXPR bool islessgreater(bfloat16, bfloat16);
                template<typename,typename,std::float_round_style> friend struct detail::bfloat16_caster;
                friend class std::numeric_limits<bfloat16>;
        #if BFLOAT_ENABLE_CPP11_HASH
                friend struct std::hash<bfloat16>;
        #endif
        #if BFLOAT_ENABLE_CPP11_USER_LITERALS
                friend bfloat16 literal::operator "" _h(long double);
        #endif
        #endif
        };
    
#if BFLOAT_ENABLE_CPP11_USER_LITERALS
        namespace literal
        {
                /// Bfloat16 literal.
                /// While this returns a properly rounded bfloat16-precision value, bfloat16 literals can unfortunately not be constant 
                /// expressions due to rather involved conversions. So don't expect this to be a literal literal without involving 
                /// conversion operations at runtime. It is a convenience feature, not a performance optimization.
                /// \param value literal value
                /// \return bfloat16 with of given value (possibly rounded)
                /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
                inline bfloat16 operator "" _h(long double value) { return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(value)); }
        }
#endif
  
        namespace detail
        {
                /// Helper class for bfloat16 casts.
                /// This class template has to be specialized for all valid cast arguments to define an appropriate static 
                /// `cast` member function and a corresponding `type` member denoting its return type.
                /// \tparam T destination type
                /// \tparam U source type
                /// \tparam R rounding mode to use
                template<typename T,typename U,std::float_round_style R=(std::float_round_style) (BFLOAT_ROUND_STYLE)> struct bfloat16_caster {};
                template<typename U,std::float_round_style R> struct bfloat16_caster<bfloat16,U,R>
                {
                #if BFLOAT_ENABLE_CPP11_STATIC_ASSERT && BFLOAT_ENABLE_CPP11_TYPE_TRAITS
                        static_assert(std::is_arithmetic<U>::value, "bfloat16_cast from non-arithmetic type unsupported");
                #endif
                        static bfloat16 cast(U arg) { return cast_impl(arg, is_float<U>()); };

                private:
                        static bfloat16 cast_impl(U arg, true_type) { return bfloat16(binary, float2bfloat16<R>(arg)); }
                        static bfloat16 cast_impl(U arg, false_type) { return bfloat16(binary, int2bfloat16<R>(arg)); }
                };
                template<typename T,std::float_round_style R> struct bfloat16_caster<T,bfloat16,R>
                {
                #if BFLOAT_ENABLE_CPP11_STATIC_ASSERT & BFLOAT_ENABLE_CPP11_TYPE_TRAITS
                        static_assert(std::is_arithmetic<T>::value, "bfloat16_cast to non-arithmetic type unsupported");
                #endif
                        static T cast(bfloat16 arg) { return cast_impl(arg, is_float<T>()); }
                private:
                        static T cast_impl(bfloat16 arg, true_type) { return bfloat162float<T>(arg.data_); }
                        static T cast_impl(bfloat16 arg, false_type) { return bfloat162int<R,true,true,T>(arg.data_); }
                };
                template<std::float_round_style R> struct bfloat16_caster<bfloat16,bfloat16,R>
                {
                        static bfloat16 cast(bfloat16 arg) { return arg; }
                };
        }
}

/// Extensions to the C++ standard library.
namespace std
{
        /// Numeric limits for bfloat16-precision floats.
        /// **See also:** Documentation for [std::numeric_limits](https://en.cppreference.com/w/cpp/types/numeric_limits)
        template<> class numeric_limits<bfloat16::bfloat16>
        {
        public:
                /// Is template specialization.
                static BFLOAT_CONSTEXPR_CONST bool is_specialized = true;
    
                /// Supports signed values.
                static BFLOAT_CONSTEXPR_CONST bool is_signed = true;

                /// Is not an integer type.
                static BFLOAT_CONSTEXPR_CONST bool is_integer = false;

                /// Is not exact.
                static BFLOAT_CONSTEXPR_CONST bool is_exact = false;

                /// Doesn't provide modulo arithmetic.
                static BFLOAT_CONSTEXPR_CONST bool is_modulo = false;

                /// Has a finite set of values.
                static BFLOAT_CONSTEXPR_CONST bool is_bounded = true;

                /// IEEE conformant.
                static BFLOAT_CONSTEXPR_CONST bool is_iec559 = true;

                /// Supports infinity.
                static BFLOAT_CONSTEXPR_CONST bool has_infinity = true;

                /// Supports quit NaNs
                static BFLOAT_CONSTEXPR_CONST bool has_quiet_NaN = true;

                /// Supports signaling NaNs
                static BFLOAT_CONSTEXPR_CONST bool has_signaling_NaN = true;

                /// Supports subnormal values.
                static BFLOAT_CONSTEXPR_CONST float_denorm_style has_denorm = denorm_present;

                /// Supports no denormalization detection.
                static BFLOAT_CONSTEXPR_CONST bool has_denorm_loss = false;

        #if BFLOAT_ERRHANDLING_THROWS
                static BFLOAT_CONSTEXPR_CONST bool traps = true;
        #else
                /// Traps only if [BFLOAT_ERRHANDLING_THROW_...](\ref BFLOAT_ERRHANDLINIG_THROW_INVALID) is activated.
                static BFLOAT_CONSTEXPR_CONST bool traps = false;
        #endif
    
                /// Does not support no pre-rounding underflow detection.
                static BFLOAT_CONSTEXPR_CONST bool tinyness_before = false;

                // Rounding mode.
                static BFLOAT_CONSTEXPR_CONST float_round_style round_style = bfloat16::bfloat16::round_style;

                /// Significant digits.
                static BFLOAT_CONSTEXPR_CONST int digits = 8;

                /// Significant decimal digits.
                static BFLOAT_CONSTEXPR_CONST int digits10 = 2;

                /// Required decimal digits to represent all possible values.
                static BFLOAT_CONSTEXPR_CONST int max_digits10 = 39;  // should be right

                /// Number base.
                static BFLOAT_CONSTEXPR_CONST int radix = 2;

                /// One more than smallest exponent.
                static BFLOAT_CONSTEXPR_CONST int min_exponent = -125; // smallest = -126

                /// Smallest normalized representable power of 10.
                static BFLOAT_CONSTEXPR_CONST int min_exponent10 = -38;

                /// One more than largest exponent.
                static BFLOAT_CONSTEXPR_CONST int max_exponent = 128;

                /// Largest finitely representable power of 10.
                static BFLOAT_CONSTEXPR_CONST int max_exponent10 = 38;

                /// Smallest positive normal value.
                static BFLOAT_CONSTEXPR bfloat16::bfloat16 min() BFLOAT_NOTHROW { return bfloat16::bfloat16(bfloat16::detail::binary, 0x0080); }

                /// Smallest finite value.
                static BFLOAT_CONSTEXPR bfloat16::bfloat16 lowest() BFLOAT_NOTHROW { return bfloat16::bfloat16(bfloat16::detail::binary, 0xFF7F); }

                /// Largest finite value.
                static BFLOAT_CONSTEXPR bfloat16::bfloat16 max() BFLOAT_NOTHROW { return bfloat16::bfloat16(bfloat16::detail::binary, 0x7F7F); }

                /// Difference between 1 and next represantable value.
                static BFLOAT_CONSTEXPR bfloat16::bfloat16 epsilon() BFLOAT_NOTHROW { return bfloat16::bfloat16(bfloat16::detail::binary, 0x0280); } // could be wrong

                /// Maximum rounding error in ULP (units in the last place).
                static BFLOAT_CONSTEXPR bfloat16::bfloat16 round_error() BFLOAT_NOTHROW 
                { return bfloat16::bfloat16(bfloat16::detail::binary, (round_style==std::round_to_nearest) ? 0x3F00 : 0x3F80); }

                /// Positive infinity.
                static BFLOAT_CONSTEXPR bfloat16::bfloat16 infinity() BFLOAT_NOTHROW { return bfloat16::bfloat16(bfloat16::detail::binary, 0x7F80); }

                /// Quiet NaN.
                static BFLOAT_CONSTEXPR bfloat16::bfloat16 quiet_NaN() BFLOAT_NOTHROW { return bfloat16::bfloat16(bfloat16::detail::binary, 0x7FFF); }

                /// Signaling NaN
                static BFLOAT_CONSTEXPR bfloat16::bfloat16 signaling_NaN() BFLOAT_NOTHROW { return bfloat16::bfloat16(bfloat16::detail::binary, 0x7FBF); }

                /// Smallest positive subnormal value.
                static BFLOAT_CONSTEXPR bfloat16::bfloat16 denorm_min() BFLOAT_NOTHROW { return bfloat16::bfloat16(bfloat16::detail::binary, 0x0001); }
              };
#if BFLOAT_ENABLE_CPP11_HASH
        /// Hash function for bfloat16-precision floats.
        /// This is only defined if C++11 `std::hash` is supported and enabled.
        ///
        /// **See also:** Documentation for [std::hash](https://en.cppreference.com/w/cpp/utility/hash)
        template<> struct hash<bfloat16::bfloat16>
        {
                /// Type of function argument.
                typedef bfloat16::bfloat16 argument_type;

                /// Function return type.
                typedef size_t result_type;

                /// Compute hash function.
                /// \param arg bfloat16 to hash
                /// \return hash value
                result_type operator()(argument_type arg) const { return hash<bfloat16::detail::uint16>()(arg.data_&-static_cast<unsigned>(arg.data_!=0x8000)); }
        };
#endif
}

namespace bfloat16
{
        /// \anchor compop
        /// \name Comparison operators
        /// \{

        /// Comparison for equality.
        /// \param x first operand
        /// \param y second operand
        /// \retval true if operands equal
        /// \retval false else
        /// \exception FE_INVALID if \a x or \a y is NaN
        inline BFLOAT_CONSTEXPR_NOERR bool operator==(bfloat16 x, bfloat16 y)
        {
                return !detail::compsignal(x.data_, y.data_) && (x.data_==y.data_ || !((x.data_|y.data_) &0x7FFF));
        }
  
        /// Comparison for inequality.
        /// \param x first operand
        /// \param y second operand
        /// \retval true if operands not equal
        /// \retval false else
        /// \exception FE_INVALID if \a x or \a y is NaN
        inline BFLOAT_CONSTEXPR_NOERR bool operator!=(bfloat16 x, bfloat16 y)
        {
                return detail::compsignal(x.data_, y.data_) || (x.data_!=y.data_ && ((x.data_|y.data_)&0x7FFF));
        }
  
        /// Comparison for less than.
        /// \param x first operand
        /// \param y second operand
        /// \retval true if \a x less than \a y
        /// \retval false else
        /// \exception FE_INVALID if \a x or \a y is NaN
        inline BFLOAT_CONSTEXPR_NOERR bool operator<(bfloat16 x, bfloat16 y)
        {
          return !detail::compsignal(x.data_, y.data_) && ((x.data_^(0x8000|(0x8000-(x.data_>>15))))+(x.data_>>15)) < ((y.data_^(0x8000|(0x8000-(y.data_>>15))))+(y.data_>>15));
        }
  
        /// Comparison for greater than.
        /// \param x first operand
        /// \param y second operand
        /// \retval true if \a x greater than \a y
        /// \retval false else
        /// \exception FE_INVALID if \a x or \a y is NaN
        inline BFLOAT_CONSTEXPR_NOERR bool operator>(bfloat16 x, bfloat16 y)
        {
                return !detail::compsignal(x.data_, y.data_) && ((x.data_^(0x8000|(0x8000-(x.data_>>15))))+(x.data_>>15)) > ((y.data_^(0x8000|(0x8000-(y.data_>>15))))+(y.data_>>15));
        }
  
        /// Comparison for less equal.
        /// \param x first operand
        /// \param y second operand
        /// \retval true if \a x less equal \a y
        /// \retval false else
        /// \exception FE_INVALID if \a x or \a y is NaN
        inline  BFLOAT_CONSTEXPR_NOERR bool operator<=(bfloat16 x, bfloat16 y)
        {
                return !detail::compsignal(x.data_, y.data_) && ((x.data_^(0x8000|(0x8000-(x.data_>>15))))+(x.data_>>15)) <= ((y.data_^(0x8000|(0x8000-(y.data_>>15))))+(y.data_>>15));
        }
  
        /// Comparison for greater equal.
        /// \param x first operand
        /// \param y second operand
        /// \retval true if \a x greater equal \a y
        /// \retval false else
        /// \exception FE_INVALID if \a x or \a y is NaN
        inline BFLOAT_CONSTEXPR_NOERR bool operator>=(bfloat16 x, bfloat16 y)
        {
                return !detail::compsignal(x.data_, y.data_) && ((x.data_^(0x8000|(0x8000-(x.data_>>15))))+(x.data_>>15)) >= ((y.data_^(0x8000|(0x8000-(y.data_>>15))))-(y.data_>>15));
        }
  
  
        /// \}
        /// \anchor arithmetics
        /// \name Arithmetic operators
        /// \{

        /// Identity.
        /// \param arg operand
        /// \return unchanged operand

        inline BFLOAT_CONSTEXPR bfloat16 operator+(bfloat16 arg) { return arg; }
  
        /// Negation.
        /// \param arg operand
        /// \return negated operand
        inline BFLOAT_CONSTEXPR bfloat16 operator-(bfloat16 arg) { return bfloat16(detail::binary, arg.data_^0x8000); }
  
        /// Addition.
        /// This operation is exact to rounding for all rounding modes.
        /// \param x left operand
        /// \param y right operand
        /// \return sum of bfloat16 expressions
        /// \exception FE_INVALID if \a x and \a y are infinities with different signs or signaling NaNs
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 operator+(bfloat16 x, bfloat16 y)
        {

        #ifdef BFLOAT_ARITHMETIC_TYPE
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(detail::bfloat162float<detail::internal_t>(x.data_) +detail::bfloat162float<detail::internal_t>(y.data_)));




	//temp
	#else
		return bfloat16( detail::bfloat162float_temp(x) + detail::bfloat162float_temp(y));

	#endif





        #if 0 //else
                int absx = x.data_ & 0x7FFF, absy = y.data_ & 0x7FFF;
                bool sub = ((x.data_^y.data_)&0x8000) != 0;
                if(absx >= 0x7F80 || absy >= 0x7F80)
                        return bfloat16(detail::binary, (absx>0x7F80 || absy>0x7F80) ? detail::signal(x.data_, y.data_) : (absy!=0x7F80) ? x.data_ : (sub && absx==0x7F80) ? detail::invalid() : y.data_);
                if(!absx)
                        return absy ? y : bfloat16(detail::binary, (bfloat16::round_style==std::round_toward_neg_infinity) ? (x.data_|y.data_) : (x.data_&y.data_));
                if(!absy)
                        return x;
                unsigned int sign = ((sub && absy>absx) ? y.data_ : x.data_) & 0x8000;
                if(absy > absx)
                        std::swap(absx, absy);
                int exp = (absx>>7) + (absx<=0x7F), d = exp - (absy>>7) - (absy<=0x7F), mx = ((absx&0x7F)|((absx>0x7F)<<7))<<3, my;
                if(d < 125)
                {
                        my = ((absy&0x7F)|((absy>0x7F)<<7))<<3;
                        my = (my>>d) | ((my&((1<<d)-1))!=0);
                }
		
                else
                        my = 1;
                if(sub)
                {
                        if(!(mx-=my))
                                return bfloat16(detail::binary, static_cast<unsigned>(bfloat16::round_style==std::round_toward_neg_infinity)<<15); // hier
                        for(; mx<0x2000 && exp>1; mx<<=1,--exp) ; 
			return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,false>(sign+((exp-7)<<7)+(mx>>3), (mx>>2)&1, (mx&0x3)!=0));
                }
                else
                {
                        mx += my;
                        int i = mx >> 11; 
                        if((exp+=i)> 254) 
                                return bfloat16(detail::binary, detail::overflow<bfloat16::round_style>(sign));
                        mx = (mx>>i) | (mx&i);
			return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,false>(sign+((exp-1)<<7)+(mx>>3), (mx>>2)&1, (mx&0x3)!=0));
                }		               
        #endif
        }
  
        /// Subtraction.
        /// This operation is exact to rounding for all rounding modes.
        /// \param x left operand
        /// \param y right operand
        /// \return difference of bfloat16 expressions
        /// \exception FE_INVALID if \a x and \a y are infinities with equal signs or signaling NaNs
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 operator-(bfloat16 x, bfloat16 y)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(detail::bfloat162float<detail::internal_t>(x.data_)-detail::bfloat162float<detail::internal_t>(y.data_)));
        #else
                return x + -y;
        #endif
        }
  
        /// Multiplication.
        /// This operation is exact to rounding for all rounding modes.
        /// \param x left operand
        /// \param y right operand
        /// \return product of bfloat16 expressions
        /// \exception FE_INVALID if multiplying 0 with infinity or if \a x or \a y is signaling NaN
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 operator*(bfloat16 x, bfloat16 y)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(detail::bfloat162float<detail::internal_t>(x.data_)*detail::bfloat162float<detail::internal_t>(y.data_)));


	//temp
	#else

		return bfloat16( detail::bfloat162float_temp(x) * detail::bfloat162float_temp(y));


	#endif




        #if 0
                int absx = x.data_ & 0x7FFF, absy = y.data_ & 0x7FFF, exp = -128;
                unsigned int sign = (x.data_^y.data_) & 0x8000;
                if(absx >= 0x7F80 || absy >= 0x7F80)
                        return bfloat16(detail::binary, (absx>0x7F80 || absy>0x7F80) ? detail::signal(x.data_, y.data_) : ((absx==0x7F80 && ! absy) || (absy==0x7F80 && ! absx)) ? detail::invalid() : (sign|0x7F80));
                if(!absx || !absy)
                        return bfloat16(detail::binary, sign);
                for(; absx<0x80; absx<<=1,--exp) ;
                for(; absy<0x80; absy<<=1,--exp) ;
                detail::uint32 m = static_cast<detail::uint32>((absx&0x7F)|0x80) * static_cast<detail::uint32>((absy&0x7F)|0x80);
                int i = m >> 21, s = m & i; // 21 vlt ändern
                exp += (absx>>7) + (absy>>7) +i;
                if(exp > 253) // 29 vlt ändern
                        return bfloat16(detail::binary, detail::overflow<bfloat16::round_style>(sign));
                else if (exp < -123) // -11 vlt ändern
                        return bfloat16(detail::binary, detail::underflow<bfloat16::round_style>(sign));
                return bfloat16(detail::binary, detail::fixed2bfloat16<bfloat16::round_style, 11,false,false,false>(m>>i, exp, sign, s)); //vlt 20 ändern
        #endif
        }
  
        /// Division.
        /// This operation is exact to rounding for all rounding modes.
        /// \param x left operand
        /// \param y right operand
        /// \return quotient of bfloat16 expressions
        /// \exception FE_INVALID if dividing 0s or infinities with each other or if \a x or \a y is signaling NaN
        /// \exception FE_DIVBYZERO if dividing finite value by 0
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 operator/(bfloat16 x, bfloat16 y)
        {
        #ifdef BFLOAT_ARITHEMTIC_TYPE
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(detail::bfloat162float<detail::internal_t>(x.data_)/detail::bfloat162float<detail::internal_t>(y.data_)));



	//temp
	#else
	
		return bfloat16( detail::bfloat162float_temp(x) / detail::bfloat162float_temp(y));

	#endif





        #if 0
                int absx = x.data_ & 0x7FFF, absy = y.data_ & 0x7FFF, exp = 126; //14vlt ändern
                unsigned int sign = (x.data_^y.data_) & 0x8000;
                if(absx >= 0x7F80 || absy >= 0x7F80)
                        return bfloat16(detail::binary, (absx>0x7F80 || absy>0x7F80) ?
                                        detail::signal(x.data_, y.data_) :(absx==absy) ?
                                        detail::invalid() : (sign|((absx==0x7F80) ? 0x7F80 : 0)));
                if(!absx)
                        return bfloat16(detail::binary, absy ? sign : detail::invalid());
                if(!absy)
                        return bfloat16(detail::binary, detail::pole(sign));
                for(; absx<0x80; absx<<=1,--exp) ;
                for(; absy<0x80; absy<<=1,++exp) ;
                detail::uint32 mx = (absx&0x7F) | 0x80, my = (absy&0x7F) | 0x80;
                int i = mx < my;
                exp += (absx>>7) - (absy>>7) - i;
                if(exp > 253) // 29 vlt ändern
                        return bfloat16(detail::binary, detail::overflow<bfloat16::round_style>(sign));
                else if(exp < -123) // vlt ändern
                        return bfloat16(detail::binary, detail::underflow<bfloat16::round_style>(sign));
                my <<= 12 + i;
                my <<= 1;
                return bfloat16(detail::binary, detail::fixed2bfloat16<bfloat16::round_style, 11, false,false,false>(mx/my, exp, sign, mx%my!=0));
        #endif
        }
  
        /// \}
        /// \anchor streaming
        /// \name Input and output
        /// \{

        /// Output operator.
        ///	This uses the built-in functionality for streaming out floating-point numbers.
        /// \param out output stream to write into
        /// \param arg bfloat16 expression to write
        /// \return reference to output stream
        template<typename charT,typename traits> std::basic_ostream<charT,traits>&operator<<(std::basic_ostream<charT,traits> &out, bfloat16 arg)
        {

        #ifdef BFLOAT_ARITHMETIC_TYPE
                return out << detail::bfloat162float<detail::internal_t>(arg.data_);
        #else
                //return out << detail::bfloat162float<float>(arg.data_);
		return out << detail::bfloat162float_temp(arg.data_);
        #endif
        }
  
        /// Input operator.
        ///	This uses the built-in functionality for streaming in floating-point numbers, specifically double precision floating 
        /// point numbers (unless overridden with [BFLOAT_ARITHMETIC_TYPE](\ref BFLOAT_ARITHMETIC_TYPE)). So the input string is first 
        /// rounded to double precision using the underlying platform's current floating-point rounding mode before being rounded 
        /// to bfloat16-precision using the library's bfloat16-precision rounding mode.
        /// \param in input stream to read from
        /// \param arg bfloat16 to read into
        /// \return reference to input stream
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding

        template<typename charT,typename traits> std::basic_istream<charT,traits>& operator>>(std::basic_istream<charT,traits> &in, bfloat16 &arg)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                detail::internal_t f;
        #else
                double f;
        #endif
                if(in >> f)
			arg.data_ = detail::float2bfloat16<bfloat16::round_style>(f);
                return in;
        }

        /// \}
        /// \anchor basic
        /// \name Basic mathematical operations
        /// \{

        /// Absolute value.
        /// **See also:** Documentation for [std::fabs](https://en.cppreference.com/w/cpp/numeric/math/fabs).
        /// \param arg operand
        /// \return absolute value of \a arg
        inline BFLOAT_CONSTEXPR bfloat16 fabs(bfloat16 arg) { return bfloat16(detail::binary, arg.data_&0x7FFF); }
  
        /// Absolute value.
        /// **See also:** Documentation for [std::abs](https://en.cppreference.com/w/cpp/numeric/math/fabs).
        /// \param arg operand
        /// \return absolute value of \a arg
        inline BFLOAT_CONSTEXPR bfloat16 abs(bfloat16 arg) { return fabs(arg); }
  
        /// Remainder of division.
        /// **See also:** Documentation for [std::fmod](https://en.cppreference.com/w/cpp/numeric/math/fmod).
        /// \param x first operand
        /// \param y second operand
        /// \return remainder of floating-point division.
        /// \exception FE_INVALID if \a x is infinite or \a y is 0 or if \a x or \a y is signaling NaN
        inline bfloat16 fmod(bfloat16 x, bfloat16 y)
        {
          unsigned int absx = x.data_ & 0x7FFF, absy = y.data_ & 0x7FFF, sign = x.data_ & 0x8000;
          if(absx >= 0x7F80 || absy >= 0x7F80)
                  return bfloat16(detail::binary, (absx>0x7F80 || absy>0x7F80) ? detail::signal(x.data_, y.data_) : (absx==0x7F80) ? detail::invalid() : x.data_);
          if(!absy)
                  return bfloat16(detail::binary, detail::invalid());
          if(!absx)
                  return x;
          if(absx == absy)
                  return bfloat16(detail::binary, sign);
          return bfloat16(detail::binary, sign|detail::mod<false,false>(absx, absy));
        }

        /// Remainder of division.
        /// **See also:** Documentation for [std::remainder](https://en.cppreference.com/w/cpp/numeric/math/remainder).
        /// \param x first operand
        /// \param y second operand
        /// \return remainder of floating-point division.
        /// \exception FE_INVALID if \a x is infinite or \a y is 0 or if \a x or \a y is signaling NaN
        inline bfloat16 remainder(bfloat16 x, bfloat16 y)
        {
                unsigned int absx = x.data_ & 0x7FFF, absy = y.data_ & 0x7FFF, sign = x.data_ & 0x8000;
                if(absx >= 0x7F80 || absy >= 0x7F80)
                        return bfloat16(detail::binary, (absx>0x7F80 || absy>0x7F80) ? detail::signal(x.data_, y.data_) : (absx==0x7F80) ? detail::invalid() : x.data_);
                if(!absy)
                        return bfloat16(detail::binary, detail::invalid());
                if(absx == absy)
                        return bfloat16(detail::binary, sign);
                return bfloat16(detail::binary, sign^detail::mod<false,true>(absx, absy));
        }
  
        /// Remainder of division.
        /// **See also:** Documentation for [std::remquo](https://en.cppreference.com/w/cpp/numeric/math/remquo).
        /// \param x first operand
        /// \param y second operand
        /// \param quo address to store some bits of quotient at
        /// \return remainder of floating-point division.
        /// \exception FE_INVALID if \a x is infinite or \a y is 0 or if \a x or \a y is signaling NaN
        inline bfloat16 remquo(bfloat16 x, bfloat16 y, int *quo)
        {
                unsigned int absx = x.data_ & 0x7FFF, absy = y.data_ & 0x7FFF, value = x.data_ & 0x8000;

                if(absx >= 0x7F80 || absy >= 0x7F80)
                        return bfloat16(detail::binary, (absx>0x7F80 || absy>0x7F80) ? detail::signal(x.data_, y.data_) : (absx==0x7F80) ? detail::invalid() : (*quo = 0, x.data_));
                if(!absy)
                        return bfloat16(detail::binary, detail::invalid());
                bool qsign = ((value^y.data_)&0x8000) !=0;
                int q = 1;

                if(absx != absy)
                        value^= detail::mod<true,true>(absx, absy, &q);
                return *quo = qsign ? -q : q, bfloat16(detail::binary, value);
        }
  
        /// Fused multiply add.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::fma](https://en.cppreference.com/w/cpp/numeric/math/fma).
        /// \param x first operand
        /// \param y second operand
        /// \param z third operand
        /// \return ( \a x * \a y ) + \a z rounded as one operation.
        /// \exception FE_INVALID according to operator*() and operator+() unless any argument is a quiet NaN and no argument is a signaling NaN
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding the final addition
        inline bfloat16 fma(bfloat16 x, bfloat16 y, bfloat16 z)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                detail::internal_t fx = detail::bfloat162float<detail::internal_t>(x.data_), fy = detail::bfloat162float<detail::internal_t>(y.data_), fz = detail::bfloat162float<detail::internal_t>(z.data_);
                #if BFLOAT_ENABLE_CPP11_CMATH && FP_FAST_FMA
                        return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::fma(fx, fy, fz)));
                #else
                        return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(fx*fy+fz));
                #endif



	//temp
	#else
		
		return bfloat16(std::fma( detail::bfloat162float_temp(x), detail::bfloat162float_temp(y), detail::bfloat162float_temp(z)));

	#endif



        #if 0
                int absx = x.data_ & 0x7FFF, absy = y.data_ & 0x7FFF, absz = z.data_ & 0x7FFF, exp = -15;
                unsigned int sign = (x.data_^y.data_ )& 0x8000;
                bool sub = ((sign^z.data_)&0x8000) !=0;
                if(absx >= 0x7F80 || absy >= 0x7F80 || absz >= 0x7F80)
                        return (absx>0x7F80 || absy>0x7F80 || absz>0x7F80) ?
                        bfloat16(detail::binary, detail::signal(x.data_, y.data_, z.data_)) : (absx==0x7F80) ?
                        bfloat16(detail::binary, (!absy || (sub && absz==0x7F80)) ?
                                 detail::invalid() : (sign|0x7F80)) : (absy==0x7F80) ?
                        bfloat16(detail::binary, (!absx || (sub && absz==0x7F80)) ? detail::invalid() : (sign|0x7F80)) : z;
                if(!absx || !absy)
                        return absz ? z : bfloat16(detail::binary, (bfloat16::round_style==std::round_toward_neg_infinity) ? (z.data_|sign) : (z.data_&sign));
                for(; absx<0x80; absx<<=1,--exp) ;
                for(; absy<0x80; absy<<=1,--exp) ;
                detail::uint32 m = static_cast<detail::uint32>((absx&0x7F)|0x80) * static_cast<detail::uint32>((absy&0x7F)|0x80);
                int i = m >> 21; //vlt ändern
                exp += (absx>>7) + (absy>>7) + i;
                m <<= 2 - 1;
                if(absz)
                {
                        int expz = 0;
                        for(; absz<0x80; absz<<=1,--expz) ;
                        expz += absz >> 7;
                        detail::uint32 mz = static_cast<detail::uint32>((absz&0x7F)|0x80) << 13; //13 vlt ändern
                        if(expz > exp || (expz == exp && mz > m))
                        {
                                std::swap(m, mz);
                                std::swap(exp, expz);
                                if(sub)
                                        sign = z.data_ & 0x8000;
                        }
                        int d = exp - expz;
                        mz = (d<23) ? ((mz>>d)|((mz&((static_cast<detail::uint32>(1)<<d)-1))!=0)) : 1; // 23
                        if(sub)
                        {
                                m = m -mz;
                                if(!m)
                                        return bfloat16(detail::binary, static_cast<unsigned>(bfloat16::round_style==std::round_toward_neg_infinity)<<15);
                                for(; m<0x800000; m<<=1,--exp) ;
                        }
                        else
                        {
                                m += mz;
                                i = m >> 24;
                                m = (m>>i) | (m&i);
                                exp += i;
                        }
                }
                if(exp > 30)
                        return bfloat16(detail::binary, detail::overflow<bfloat16::round_style>(sign));
                else if(exp < -10)
                        return bfloat16(detail::binary, detail::underflow<bfloat16::round_style>(sign));
                return bfloat16(detail::binary, detail::fixed2bfloat16<bfloat16::round_style, 23,false,false,false>(m, exp-1, sign));
        #endif
        }
  
        /// Maximum of bfloat16 expressions.
        /// **See also:** Documentation for [std::fmax](https://en.cppreference.com/w/cpp/numeric/math/fmax).
        /// \param x first operand
        /// \param y second operand
        /// \return maximum of operands, ignoring quiet NaNs
        /// \exception FE_INVALID if \a x or \a y is signaling NaN
        inline BFLOAT_CONSTEXPR_NOERR bfloat16 fmax(bfloat16 x, bfloat16 y)
        {
                return bfloat16(detail::binary, (!isnan(y) && (isnan(x) || (x.data_^(0x8000|(0x8000-(x.data_>>15)))) < (y.data_^(0x8000|(0x8000-(y.data_>>15)))))) ? detail::select(y.data_, x.data_) : detail::select(x.data_, y.data_));
        }
  
        /// Minimum of bfloat16 expressions.
        /// **See also:** Documentation for [std::fmin](https://en.cppreference.com/w/cpp/numeric/math/fmin).
        /// \param x first operand
        /// \param y second operand
        /// \return minimum of operands, ignoring quiet NaNs
        /// \exception FE_INVALID if \a x or \a y is signaling NaN
        inline BFLOAT_CONSTEXPR_NOERR bfloat16 fmin(bfloat16 x, bfloat16 y)
        {
                return bfloat16(detail::binary, (!isnan(y) && (isnan(x) || (x.data_^(0x8000|(0x8000-(x.data_>>15)))) > (y.data_^(0x8000|(0x8000-(y.data_>>15)))))) ? detail::select(y.data_, x.data_) : detail::select(x.data_, y.data_));
        }
  
        /// Positive difference.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::fdim](https://en.cppreference.com/w/cpp/numeric/math/fdim).
        /// \param x first operand
        /// \param y second operand
        /// \return \a x - \a y or 0 if difference negative
        /// \exception FE_... according to operator-(bfloat16,bfloat16)
        inline bfloat16 fdim(bfloat16 x, bfloat16 y)
        {
                if(isnan(x) || isnan(y))
                        return bfloat16(detail::binary, detail::signal(x.data_, y.data_));
                return (x.data_^(0x8000|(0x8000-(x.data_>>15)))) <= (y.data_^(0x8000|(0x8000-(y.data_>>15)))) ? bfloat16(detail::binary, 0) : (x-y); // has problems with negative values
		//return bfloat16(std::fdim(detail::bfloat162float_temp(x), detail::bfloat162float_temp(y)));
        }
  
        /// Get NaN value.
        /// **See also:** Documentation for [std::nan](https://en.cppreference.com/w/cpp/numeric/math/nan).
        /// \param arg string code
        /// \return quiet NaN
        inline bfloat16 nanh(const char *arg)
        {
                unsigned int value = 0x7FFF;
                while(*arg)
                        value ^= static_cast<unsigned>(*arg++) & 0xFF;
                return bfloat16(detail::binary, value);
        }
  
        /// \}
        /// \anchor exponential
        /// \name Exponential functions
        /// \{

        /// Exponential function.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::exp](https://en.cppreference.com/w/cpp/numeric/math/exp).
        /// \param arg function argument
        /// \return e raised to \a arg
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 exp(bfloat16 arg)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::exp(detail::bfloat162float<detail::internal_t>(arg.data_))));



	//temp
	#else


		return bfloat16(std::exp(detail::bfloat162float_temp(arg)));

	#endif


        #if 0
                int abs = arg.data_ & 0x7FFF;
                if(!abs)
                        return bfloat16(detail::binary, 0x3F80);
                if(abs >= 0x7F80)
                        return bfloat16(detail::binary, (abs==0x7F80) ? (0x7F80&((arg.data_>>15)-1U)) : detail::signal(arg.data_));
                if(abs >= 0x4C80) // kan falsch sein
                        return bfloat16(detail::binary, (arg.data_&0x8000) ? detail::underflow<bfloat16::round_style>() : detail::overflow<bfloat16::round_style>());
                detail::uint32 m = detail::multiply64(static_cast<detail::uint32>((abs&0x7F)+((abs>0x7F)<<7))<<24, 0xB8AA3B29);
                int e = (abs>>7) + (abs<=0x7F), exp;
                if(e < 14)
                {
                        exp = 0;
                        m >>= 14 - e;
                }
                else
                {
                        exp = m >> (408-e);//45
                        m = (m<<(e-126)) & 0x7FFFFFFF;//14
                }

                return bfloat16(detail::binary, detail::exp2_post<bfloat16::round_style, true>(detail::exp2(m, 23), exp, (arg.data_&0x8000)!=0));//26
        #endif
        }
  
        /// Binary exponential.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::exp2](https://en.cppreference.com/w/cpp/numeric/math/exp2).
        /// \param arg function argument
        /// \return 2 raised to \a arg
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 exp2(bfloat16 arg)
        {
        #if defined(BFLOAT_ARITHMETIC_TYPE) && BFLOAT_ENABLE_CPP11_CMATH
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::exp2(detail::bfloat162float<detail::internal_t>(arg.data_))));



	//temp
	#else

		return bfloat16(std::exp2(detail::bfloat162float_temp(arg)));

	#endif





        #if 0
                int abs = arg.data_ & 0x7FFF;
                if(!abs)
                        return bfloat16(detail::binary, 0x3F80);
                if(abs >= 0x7F80)
                        return bfloat16(detail::binary, (abs==0x7F80) ? (0x7F80&((arg.data_>>15)-1U)) : detail::signal(arg.data_));
                if(abs >= 0x4E40) // kann falsch sein
                        return bfloat16(detail::binary, (arg.data_&0x8000) ? detail::underflow<bfloat16::round_style>() : detail::overflow<bfloat16::round_style>());
                int e = (abs>>7) + (abs<=0x7F), exp = (abs&0x7F) + ((abs>0x7F)<<7);
                detail::uint32 m = detail::exp2((static_cast<detail::uint32>(exp)<<(6+e))&0x7FFFFFFF, 28);
                exp >>= 25 -e;
                if(m == 0x800000000)
                {
                        if(arg.data_&0x8000)
                                exp = -exp;
                        else if(exp > 15)
                                return bfloat16(detail::binary, detail::overflow<bfloat16::round_style>());
                        return bfloat16(detail::binary, detail::fixed2bfloat16<bfloat16::round_style, 31,false,false,false>(m, exp+14));
                }
                return bfloat16(detail::binary, detail::exp2_post<bfloat16::round_style,true>(m, exp, (arg.data_&0x8000)!=0));
        #endif
        }
  
        /// Exponential minus one.
        /// This function may be 1 ULP off the correctly rounded exact result in <0.05% of inputs for `std::round_to_nearest` 
        /// and in <1% of inputs for any other rounding mode.
        ///
        /// **See also:** Documentation for [std::expm1](https://en.cppreference.com/w/cpp/numeric/math/expm1).
        /// \param arg function argument
        /// \return e raised to \a arg and subtracted by 1
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 expm1(bfloat16 arg)
        {
        #if defined(BFLOAT_ARITHMETIC_TYPE) && BFLOAT_ENABLE_CPP11_CMATH
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::expm1(detail::bfloat162float<detail::internal_t>(arg.data_))));




	//temp
	#else

		return bfloat16(std::expm1(detail::bfloat162float_temp(arg)));
	
	#endif



        #if 0
                unsigned int abs = arg.data_ & 0x7FFF, sign = arg.data_ & 0x8000;
                if(!abs)
                        return arg;
                if(abs>= 0x7F80)
                        return bfloat16(detail::binary, (abs==0x7F80) ? (0x7F80+(sign>>1)) : detail::signal(arg.data_));
                if(abs >= 0x4A00)
                        return bfloat16(detail::binary, (arg.data_&0x8000) ? detail::rounded<bfloat16::round_style,true>(0xBF7F, 1, 1) : detail::overflow<bfloat16::round_style>());
                detail::uint32 m = detail::multiply64(static_cast<detail::uint32>((abs&0x7F) + ((abs>0x7F)<<7))<<24, 0xB8AA3B29);
                int e = (abs>>7) + (abs<=0x7F), exp;
                if(e < 14)
                {
                        exp = 0;
                        m >>= 14 -e;
                }
                else
                {
                        exp = m >> (45-e);
                        m = (m<<(e-14)) & 0x7FFFFFFF;
                }
                m = detail::exp2(m);
                if(sign)
                {
                        int s = 0;
                        if(m > 0x8000000000)
                        {
                                ++exp;
                                m = detail::divide64(0x800000000, m, s);
                        }
                        m = 0x800000000 - ((m>>exp)|((m&((static_cast<detail::uint32>(1)<<exp)-1))!=0)|s);
                        exp = 0;
                }
                else
                        m -= (exp<31) ? (0x800000000>>exp) : 1;
                for(exp+=14; m<0x800000000 && exp; m<<=1,--exp) ;
                if(exp > 29)
                        return bfloat16(detail::binary, detail::overflow<bfloat16::round_style>());
                return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(sign+ (exp<<7)+(m>>24), (m>>23)&1, (m&0xFFFFF)!=0));
        #endif
        }
  
        /// Natural logarithm.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::log](https://en.cppreference.com/w/cpp/numeric/math/log).
        /// \param arg function argument
        /// \return logarithm of \a arg to base e
        /// \exception FE_INVALID for signaling NaN or negative argument
        /// \exception FE_DIVBYZERO for 0
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 log(bfloat16 arg)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::log(detail::bfloat162float<detail::internal_t>(arg.data_))));


	//temp
	#else

		return bfloat16(std::log(detail::bfloat162float_temp(arg)));

	#endif

        #if 0
                int abs = arg.data_ & 0x7FFF, exp = -15;
                if(!abs)
                        return bfloat16(detail::binary, detail::pole(0x8000));
                if(arg.data_ & 0x8000)
                        return bfloat16(detail::binary, (arg.data_<=0xFF80) ? detail::invalid() : detail::signal(arg.data_));
                if(abs >= 0x7F80)
                        return (abs==0x7F80) ? arg : bfloat16(detail::binary, detail::signal(arg.data_));
                for(; abs<0x80; abs<<=1,--exp) ;
                exp += abs >> 7;
                return bfloat16(detail::binary, detail::log2_post<bfloat16::round_style, 0xB8AA3B2A>(detail::log2(static_cast<detail::uint32>((abs&0x7F)|0x80)<<20, 27)+8, exp, 17));
        #endif
        }
  
        /// Common logarithm.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::log10](https://en.cppreference.com/w/cpp/numeric/math/log10).
        /// \param arg function argument
        /// \return logarithm of \a arg to base 10
        /// \exception FE_INVALID for signaling NaN or negative argument
        /// \exception FE_DIVBYZERO for 0
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 log10(bfloat16 arg)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::log10(detail::bfloat162float<detail::internal_t>(arg.data_))));



	//temp
	#else

		return bfloat16(std::log10(detail::bfloat162float_temp(arg)));
	
	#endif

        #if 0
                int abs = arg.data_ & 0x7FFF, exp = -15;
                if(!abs)
                        return bfloat16(detail::binary, detail::pole(0x8000));
                if(arg.data_ & 0x8000)
                        return bfloat16(detail::binary, (arg.data_<=0xFF80) ? detail::invalid() : detail::signal(arg.data_));
                if(abs >= 0x7F80)
                        return (abs==0x7F80) ? arg : bfloat16(detail::binary, detail::signal(arg.data_));
                switch(abs)
                {
                        case 0x4900: return bfloat16(detail::binary, 0x3F80);
                        case 0x5640: return bfloat16(detail::binary, 0x4000);
                        case 0x63D0: return bfloat16(detail::binary, 0x4040);
                        case 0x70E2: return bfloat16(detail::binary, 0x4080);
                }
                for(; abs<0x80; abs<<=1,--exp) ;
                exp += abs >> 10;
                return bfloat16(detail::binary, detail::log2_post<bfloat16::round_style, 0xD49A784C>(detail::log2(static_cast<detail::uint32>((abs&0x7F)|0x80)<<20, 27)+8, exp, 16));
        #endif
        }
  
        /// Binary logarithm.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::log2](https://en.cppreference.com/w/cpp/numeric/math/log2).
        /// \param arg function argument
        /// \return logarithm of \a arg to base 2
        /// \exception FE_INVALID for signaling NaN or negative argument
        /// \exception FE_DIVBYZERO for 0
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 log2(bfloat16 arg)
        {
        #if defined(BFLOAT_ARITHMETIC_TYPE) && BFLOAT_ENABLE_CPP11_CMATH
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::log2(detail::bfloat162float<detail::internal_t>(arg.data_))));


	//temp
	#else

		return bfloat16(std::log2(detail::bfloat162float_temp(arg)));

	#endif

        #if 0
                int abs = arg.data_ & 0x7FFF, exp = -15, s = 0;
                if(!abs)
                        return bfloat16(detail::binary, detail::pole(0x8000));
                if(arg.data_ & 0x8000)
                        return bfloat16(detail::binary, (arg.data_<=0xFF80) ? detail::invalid() : detail::signal(arg.data_));
                if(abs == 0x7F80)
                        return (abs==0x7F80) ? arg : bfloat16(detail::binary, detail::signal(arg.data_));
                if(abs == 0x3F80)
                        return bfloat16(detail::binary, 0);
                for(; abs<0x80; abs<<=1,--exp) ;
                exp += (abs>>10);
                if(!(abs&0x7F))
                {
                        unsigned int value = static_cast<unsigned>(exp<0) << 15, m = std::abs(exp) << 6;
                        for(exp=18; m<0x80; m<<=1,--exp) ;
                        return bfloat16(detail::binary, value+(exp<<10)+m);
                }
                detail::uint32 ilog = exp, sign = detail::sign_mask(ilog), m = (((ilog<<27)+(detail::log2(static_cast<detail::uint32>((abs&0x7F)|0x80)<<20, 28)>>4))^sign) - sign;
                if(!m)
                        return bfloat16(detail::binary, 0);
                for(exp=14; m<0x8000000 && exp; m<<=1,--exp) ;
                for(; m>0xFFFFFFF; m>>=1,++exp)
                        s |= m & 1;
                return bfloat16(detail::binary, detail::fixed2bfloat16<bfloat16::round_style, 27,false,false,true>(m, exp, sign&0x8000, s));
        #endif
        }
  
        /// Natural logarithm plus one.
        /// This function may be 1 ULP off the correctly rounded exact result in <0.05% of inputs for `std::round_to_nearest` 
        /// and in ~1% of inputs for any other rounding mode.
        ///
        /// **See also:** Documentation for [std::log1p](https://en.cppreference.com/w/cpp/numeric/math/log1p).
        /// \param arg function argument
        /// \return logarithm of \a arg plus 1 to base e
        /// \exception FE_INVALID for signaling NaN or argument <-1
        /// \exception FE_DIVBYZERO for -1
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 log1p(bfloat16 arg)
        {
        #if defined(BFLOAT_ARITHMETIC_TYPE) && BFLOAT_ENABLE_CPP11_CMATH
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::log1p(detail::bfloat162float<detail::internal_t>(arg.data_))));




	//temp
	#else

		return bfloat16(std::log1p(detail::bfloat162float_temp(arg)));

	#endif

        #if 0
                if(arg.data_ >= 0xBF80)
                        return bfloat16(detail::binary, (arg.data_==0xBF80) ? detail::pole(0x8000) : (arg.data_<=0xFF80) ? detail::invalid() : detail::signal(arg.data_));
                int abs = arg.data_ & 0x7FFF, exp = -15;
                if(!abs || abs >= 0x7F80)
                        return (abs>0x7F80) ? bfloat16(detail::binary, detail::signal(arg.data_)) : arg;
                for(; abs<0x80; abs<<=1,--exp) ;
                exp += abs >> 10;
                detail::uint32 m = static_cast<detail::uint32>((abs&0x7F)|0x80) << 20;
                if(arg.data_ & 0x8000)
                {
                        m = 0x40000000 - (m>>-exp);
                        for(exp=0; m<0x40000000; m<<=1,--exp) ;
                }
                else
                {
                        if(exp < 0)
                        {
                                m = 0x40000000 + (m>>-exp);
                                exp = 0;
                        }
                        else
                        {
                                m += 0x40000000 >> exp;
                                int i = m >> 31;
                                m >>= i;
                                exp += i;
                        }
                }
                return bfloat16(detail::binary, detail::log2_post<bfloat16::round_style, 0xB8AA3B2A>(detail::log2(m), exp, 17));
        #endif
        }
  
        /// \}
        /// \anchor power
        /// \name Power functions
        /// \{

        /// Square root.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::sqrt](https://en.cppreference.com/w/cpp/numeric/math/sqrt).
        /// \param arg function argument
        /// \return square root of \a arg
        /// \exception FE_INVALID for signaling NaN and negative arguments
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 sqrt(bfloat16 arg)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::sqrt(detail::bfloat162float<detail::internal_t>(arg.data_))));




	//temp
	#else

		return bfloat16(std::sqrt(detail::bfloat162float_temp(arg)));

	#endif

        #if 0
                int abs = arg.data_ & 0x7FFF, exp = 127;
                if(!abs || arg.data_ >= 0x7F80)
                        return bfloat16(detail::binary, (abs>0x7F80) ? detail::signal(arg.data_) : (arg.data_>0x8000) ? detail::invalid() : arg.data_);
                for(; abs<0x80; abs<<=1,--exp) ;
                detail::uint32 r = static_cast<detail::uint32>((abs&0x7F)|0x80) << 7, m = detail::sqrt<20>(r, exp+=abs>>7);
                return bfloat16(detail::binary, detail::rounded<bfloat16::round_style, false>((exp<<7)+(m&0x7F), r>m, r!=0));
        #endif
        }
  
        /// Cubic root.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::cbrt](https://en.cppreference.com/w/cpp/numeric/math/cbrt).
        /// \param arg function argument
        /// \return cubic root of \a arg
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 cbrt(bfloat16 arg)
        {
        #if defined(BFLOAT_ARITHMETIC_TYPE) && BFLOAT_ENABLE_CPP11_CMATH
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::cbrt(detail::bfloat162float<detail::internalt>(arg.data_))));



	//temp
	#else

		return bfloat16(std::cbrt(detail::bfloat162float_temp(arg)));

	#endif

        #if 0
                int abs = arg.data_ & 0x7FFF, exp = -127;
                if(!abs || abs == 0x3F80 || abs >= 0x7F80)
                        return (abs>0x7F80) ? bfloat16(detail::binary, detail::signal(arg.data_)) : arg;
                for(; abs<0x80; abs<<=1,--exp) ;
                detail::uint32 ilog = exp +(abs>>7), sign = detail::sign_mask(ilog), f, m = (((ilog<<24)+(detail::log2(static_cast<detail::uint32>((abs&0x7F)|0x80)<<20, 24)>>4))^sign) - sign;
                for(exp=2; m<0x80000000; m<<=1,--exp) ;
                m = detail::multiply64(m, 0xAAAAAAAB);
                int i = m >>31, s;
                exp += i;
                m <<= 1 -i;
                if(exp < 0)
                {
                        f = m >> -exp;
                        exp = 0;
                }
                else
                {
                        f = (m<<exp) & 0x7FFFFFFF;
                        exp = m >> (31-exp);
                }
                m = detail::exp2(f, (bfloat16::round_style==std::round_to_nearest) ? 29 : 26);
                if(sign)
                {
                        if(m > 0x80000000)
                        {
                                m = detail::divide64(0x80000000, m, s);
                                ++exp;
                        }
                        exp = -exp;
                }
                return bfloat16(detail::binary, (bfloat16::round_style==std::round_to_nearest) ? detail::fixed2bfloat16<bfloat16::round_style,31,false,false,false>(m, exp+14, arg.data_&0x8000) : detail::fixed2bfloat16<bfloat16::round_style,23,false,false,false>((m+0x10)>>8, exp+14, arg.data_&0x8000));
        #endif
        }
  
        /// Hypotenuse function.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::hypot](https://en.cppreference.com/w/cpp/numeric/math/hypot).
        /// \param x first argument
        /// \param y second argument
        /// \return square root of sum of squares without internal over- or underflows
        /// \exception FE_INVALID if \a x or \a y is signaling NaN
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding of the final square root
        inline bfloat16 hypot(bfloat16 x, bfloat16 y)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                detail::internal_t fx = detail::bfloat162float<detail::internal_t>(x.data_), fy = detail::bfloat162float<detail::internal_t>(y.data_);
                #if BFLOAT_ENABLE_CPP11_CMATH
                        return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::hypot(fx, fy)));
                #else
                        return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::sqrt(fx*fx+fy*fy)));
                #endif



	//temp
	#else
	
		return bfloat16(std::hypot(detail::bfloat162float_temp(x), detail::bfloat162float_temp(y)));
	
	#endif


        #if 0 
                int absx = x.data_ & 0x7FFF, absy = y.data_ & 0x7FFF, expx = 0, expy = 0;
                if(absx >= 0x7F80 || absy >= 0x7F80)
                        return bfloat16(detail::binary, (absx==0x7F80) ? detail::select(0x7F80, y.data_) : (absy==0x7F80) ? detail::select(0x7F80, x.data_) : detail::signal(x.data_, y.data_));
                if(!absx) return bfloat16(detail::binary, absy ? detail::check_underflow(absy) : 0);
                if(!absy) return bfloat16(detail::binary, detail::check_underflow(absx));
                if(absy > absx)
                        std::swap(absx, absy);
                for(; absx<0x80; absx<<=1,--expx) ;
                for(; absy<0x80; absy<<=1,--expy) ;
                detail::uint32 mx = (absx&0x7F) | 0x80, my = (absy&0x7F) | 0x80;
                mx *= mx;
                my *= my;
                int ix = mx >> 24, iy = my >> 24;
                expx = 2*(expx+(absx>>7)) - 127 + ix;
                expy = 2*(expy+(absy>>7)) - 127 + iy;
                mx <<= 7 - ix;
                my <<= 7 - iy;
                int d = expx - expy;
                my = (d<30) ? ((my>>d)|((my&((static_cast<detail::uint32>(1)<<d)-1))!=0)) : 1;
                return bfloat16(detail::binary, detail::hypot_post<bfloat16::round_style>(mx+my, expy));
        #endif
        }
  
        /// Hypotenuse function.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::hypot](https://en.cppreference.com/w/cpp/numeric/math/hypot).
        /// \param x first argument
        /// \param y second argument
        /// \param z third argument
        /// \return square root of sum of squares without internal over- or underflows
        /// \exception FE_INVALID if \a x, \a y or \a z is signaling NaN
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding of the final square root
        inline bfloat16 hypot(bfloat16 x, bfloat16 y, bfloat16 z)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                detail::internal_t fx = detail::bfloat162float<detail::internal_t>(x.data_), fy =
                  detail::bfloat162float<detail::internal_t>(y.data_), fz =
                  detail::bfloat162float<detail::internal_t>(z.data_);
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::sqrt(fx*fx+fy*fy+fz*fz)));




	//temp
	#else

		return bfloat16(std::hypot(detail::bfloat162float_temp(x), detail::bfloat162float_temp(y), detail::bfloat162float_temp(z)));

	#endif




        #if 0
                int absx = x.data_ & 0x7FFF, absy = y.data_ & 0x7FFF, absz = z.data_ & 0x7FFF, expx = 0, expy = 0, expz = 0;
                if(!absx)
                        return hypot(y, z);
                if(!absy)
                        return hypot(x, z);
                if(!absz)
                        return hypot(x, y);
                if(absx >= 0x7F80 || absy >= 0x7F80 || absz >= 0x7F80)
                        return bfloat16(detail::binary, (absx==0x7F80) ? detail::select(0x7F80, detail::select(y.data_, z.data_)) : (absy==0x7F80) ? detail::select(0x7F80, detail::select(x.data_, z.data_)) : (absz==0x7F80) ? detail::select(0x7F80, detail::select(x.data_ ,y.data_)) : detail::signal(x.data_, y.data_, z.data_));
                if(absz > absy)
                        std::swap(absy, absz);
                if(absy > absx)
                        std::swap(absx, absy);
                if(absz > absy)
                        std::swap(absy, absz);
                for(; absx<0x80; absx<<=1,--expx) ;
                for(; absy<0x80; absy<<=1,--expy) ;
                for(; absz<0x80; absz<<=1,--expz) ;
                detail::uint32 mx = (absx&0x7F) | 0x80, my = (absy&0x7F) | 0x80, mz = (absz&0x7F) | 0x80;
                mx *= mx;
                my *= my;
                mz *= mz;
                int ix = mx >> 21, iy = my >> 21, iz = mz >> 21;
                expx = 2*(expx+(absx>>10)) - 15 + ix;
                expy = 2*(expy+(absy>>10)) - 15 + iy;
                expz = 2*(expz+(absz>>10)) - 15 + iz;
                mx <<= 10 - ix;
                my <<= 10 - iy;
                mz <<= 10 - iz;
                int d = expy - expz;
                mz = (d<30) ? ((mz>>d)|((mz&((static_cast<detail::uint32>(1)<<d)-1))!=0)) : 1;
                my += mz;
                if(my & 0x80000000)
                {
                        my = (my>>1) | (my&1);
                        if(++expy > expx)
                        {
                                std::swap(mx, my);
                                std::swap(expx, expy);
                        }
                }
                d = expx - expy;
                my = (d<30) ? ((my>>d)|((my&((static_cast<detail::uint32>(1)<<d)-1))!=0)) : 1;
                return bfloat16(detail::binary, detail::hypot_post<bfloat16::round_style>(mx+my, expx));
        #endif
        }
  
        /// Power function.
        /// This function may be 1 ULP off the correctly rounded exact result for any rounding mode in ~0.00025% of inputs.
        ///
        /// **See also:** Documentation for [std::pow](https://en.cppreference.com/w/cpp/numeric/math/pow).
        /// \param x base
        /// \param y exponent
        /// \return \a x raised to \a y
        /// \exception FE_INVALID if \a x or \a y is signaling NaN or if \a x is finite an negative and \a y is finite and not integral
        /// \exception FE_DIVBYZERO if \a x is 0 and \a y is negative
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 pow(bfloat16 x, bfloat16 y)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::pow(detail::bfloat162float<detail::internal_t>(x.data_), detail::bfloat162float<detail::internal_t>(y.data_))));





	//temp
	#else

		return bfloat16(std::pow(detail::bfloat162float_temp(x), detail::bfloat162float_temp(y)));
	
	#endif


        #if 0
                int absx = x.data_  & 0x7FFF, absy = y.data_ & 0x7FFF, exp = -15;
                if(!absy || x.data_ == 0x3F80)
                        return bfloat16(detail::binary, detail::select(0x3F80, (x.data_==0x3F80) ? y.data_ : x.data_));
                bool is_int = absy >= 0x6080 || (absy>=0x3F80 && !(absy&((1<<(25-(absy>>10)))-1)));
                unsigned int sign = x.data_ & (static_cast<unsigned>((absy<0x6100)&&is_int&&((absy>>(25-(absy>>10)))&1))<<15);
                if(absx >= 0x7F80 || absy >= 0x7F80)
                        return bfloat16(detail::binary, (absx>0x7F80 || absy>0x7F80) ? detail::signal(x.data_, y.data_) : (absy==0x7F80) ? ((absx==0x3F80) ? 0x3F80 : (!absx && y.data_==0xFF80) ? detail::pole() : (0x7F80&-((y.data_>>15)^(absx>0x3F80)))) : (sign|(0x7F80&((y.data_>>15)-1U))));
                if(!absx)
                        return bfloat16(detail::binary, (y.data_&0x8000) ? detail::pole(sign) : sign);
                if((x.data_&0x8000) && !is_int)
                        return bfloat16(detail::binary, detail::invalid());
                if(x.data_==0xBF80)
                        return bfloat16(detail::binary, sign|0x3F80);
                if(y.data_==0x3F00)
                        return sqrt(x);
                if(y.data_==0x3F80)
                        return bfloat16(detail::binary, detail::check_underflow(x.data_));
                if(y.data_==0x4000)
                        return x * x;
                for(; absx<0x80; absx<<=1,--exp) ;
                detail::uint32 ilog = exp + (absx>>10), msign = detail::sign_mask(ilog), f, m = (((ilog>>27)+((detail::log2(static_cast<detail::uint32>((absx&0x7F)|0x80)<<20)+8)>>4))^msign) - msign;
                for(exp=-11; m<0x80000000; m<<=1,--exp) ;
                for(; absy<0x80; absy<<=1,--exp) ;
                m = detail::multiply64(m, static_cast<detail::uint32>((absy&0x7F)|0x80)<<21);
                int i = m >> 31;
                exp += (absy>>10) + i;
                m <<= 1 - i;
                if(exp < 0)
                {
                        f = m >> -exp;
                        exp = 0;
                }
                else
                {
                        f = (m<<exp) & 0x7FFFFFFF;
                        exp = m >> (31-exp);
                }
                return bfloat16(detail::binary, detail::exp2_post<bfloat16::round_style,false>(detail::exp2(f), exp, ((msign&1)^(y.data_>>15))!=0, sign));
        #endif
        }
  
        /// \}
        /// \anchor trigonometric
        /// \name Trigonometric functions
        /// \{

        /// Compute sine and cosine simultaneously.
        ///	This returns the same results as sin() and cos() but is faster than calling each function individually.
        ///
        /// This function is exact to rounding for all rounding modes.
        /// \param arg function argument
        /// \param sin variable to take sine of \a arg
        /// \param cos variable to take cosine of \a arg
        /// \exception FE_INVALID for signaling NaN or infinity
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline void sincos(bfloat16 arg, bfloat16 *sin, bfloat16 *cos)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                detail::internal_t f = detail::bfloat162float<detail::internal_t>(arg.data_);
                *sin = bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::sin(f)));
                *cos = bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::cos(f)));




	//temp
	#else

		*sin = bfloat16(std::sin(detail::bfloat162float_temp(arg)));
		*cos = bfloat16(std::cos(detail::bfloat162float_temp(arg)));

	#endif


        #if 0
                int abs = arg.data_ & 0x7FFF, sign = arg.data_ >> 15, k;
                if(abs >= 0x7F80)
                        *sin = *cos = bfloat16(detail::binary, (abs==0x7F80) ? detail::invalid() : detail::signal(arg.data_));
                else if(!abs)
                {
                        *sin = arg;
                        *cos = bfloat16(detail::binary, 0x3F80);
                }
                else if(abs < 0x20A0)
                {
                        *sin = bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(arg.data_-1, 1, 1));
                        *cos = bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(0x3F7F, 1, 1));
                }
                else
                {
                        if(bfloat16::round_style != std::round_to_nearest)
                        {
                                switch(abs)
                                {
                                        case 0x48B7:
                                          *sin = bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>((~arg.data_&0x8000)|0x1D07, 1, 1));
                                          *cos = bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(0xBBFF, 1, 1));
                                          return;
                                        case 0x598C:
                                          *sin = bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>((arg.data_&0x8000)|0x3BFF, 1, 1));
                                          *cos = bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(0x80FC, 1, 1));
                                          return;
                                        case 0x6A64:
                                          *sin = bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>((~arg.data_&0x8000)|0x3BFE, 1, 1));
                                          *cos = bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(0x27FF, 1, 1));
                                          return;
                                        case 0x6D8C:
                                          *sin = bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>((arg.data_&0x8000)|0x0FE6, 1, 1));
                                          *cos = bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(0x3BFF, 1, 1));
                                          return;
                                }
                        }
                }
                std::pair<detail::uint32, detail::uint32> sc = detail::sincos(detail::angle_arg(abs, k), 28);
                switch(k & 3)
                {
                        case 1: sc = std::make_pair(sc.second, -sc.first); break;
                        case 2: sc = std::make_pair(-sc.first, -sc.second); break;
                        case 3: sc = std::make_pair(-sc.second, sc.first); break;
                }
                *sin = bfloat16(detail::binary, detail::fixed2bfloat16<bfloat16::round_style, 30,true,true,true>((sc.first^-static_cast<detail::uint32>(sign))+sign));
                *cos = bfloat16(detail::binary, detail::fixed2bfloat16<bfloat16::round_style, 30,true,true,true>(sc.second));
        #endif
        }
  
        /// Sine function.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::sin](https://en.cppreference.com/w/cpp/numeric/math/sin).
        /// \param arg function argument
        /// \return sine value of \a arg
        /// \exception FE_INVALID for signaling NaN or infinity
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 sin(bfloat16 arg)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::sin(detail::bfloat162float<detail::internal_t>(arg.data_))));




	//temp
	#else

		return bfloat16(std::sin(detail::bfloat162float_temp(arg)));

	#endif




        #if 0 
                int abs = arg.data_ & 0x7FFF, k;
                if(!abs)
                        return arg;
                if(abs >= 0x7F80)
                        return bfloat16(detail::binary, (abs==0x7F80) ? detail::invalid() : detail::signal(arg.data_));
                if(abs < 2120)
                        return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(arg.data_-1, 1, 1));
                if(bfloat16::round_style != std::round_to_nearest)
                        switch(abs)
                        {
                                case 0x48B7: return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>((~arg.data_&0x8000)|0x1D07, 1, 1));
                                case 0x6A64: return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>((~arg.data_&0x8000)|0x3BFE, 1, 1));
                                case 0x6D8C: return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>((arg.data_&0x8000)|0x0FE6, 1, 1));
                        }
                std::pair<detail::uint32,detail::uint32> sc = detail::sincos(detail::angle_arg(abs,k),28);
                detail::uint32 sign = -static_cast<detail::uint32>(((k>>1)&1)^(arg.data_>>15));
                return bfloat16(detail::binary, detail::fixed2bfloat16<bfloat16::round_style, 30,true,true,true>((((k&1) ? sc.second : sc.first)^sign)- sign));
        #endif
        }
  
        /// Cosine function.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::cos](https://en.cppreference.com/w/cpp/numeric/math/cos).
        /// \param arg function argument
        /// \return cosine value of \a arg
        /// \exception FE_INVALID for signaling NaN or infinity
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 cos(bfloat16 arg)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::cos(detail::bfloat162float<detail::internal_t>(arg.data_))));




	//temp
	#else
	
		return bfloat16(std::cos(detail::bfloat162float_temp(arg)));

	#endif


        #if 0
                int abs = arg.data_ & 0x7FFF, k;
                if(!abs)
                        return bfloat16(detail::binary, 0x3F80);
                if(abs >= 0x7F80)
                        return bfloat16(detail::binary, (abs==0x7F80) ? detail::invalid() : detail::signal(arg.data_));
                if(abs >= 0x20A0)
                        return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(0x3BFF, 1, 1));
                if(bfloat16::round_style != std::round_to_nearest && abs == 0x598C)
                        return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(0x80FC, 1, 1));
                std::pair<detail::uint32, detail::uint32> sc = detail::sincos(detail::angle_arg(abs, k), 28);
                detail::uint32 sign = -static_cast<detail::uint32>(((k>>1)^k)&1);
                return bfloat16(detail::binary, detail::fixed2bfloat16<bfloat16::round_style, 30,true,true,true>((((k&1) ? sc.first : sc.second)^sign) - sign));
        #endif
        }
  
        /// Tangent function.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::tan](https://en.cppreference.com/w/cpp/numeric/math/tan).
        /// \param arg function argument
        /// \return tangent value of \a arg
        /// \exception FE_INVALID for signaling NaN or infinity
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 tan(bfloat16 arg)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::tan(detail::bfloat162float<detail::internal_t>(arg.data_))));





	//temp 
	#else

		return bfloat16(std::tan(detail::bfloat162float_temp(arg)));

	#endif


        #if 0
                int abs = arg.data_ & 0x7FFF, exp = 13, k;
                if(!abs)
                        return arg;
                if(abs >= 0x7F80)
                        return bfloat16(detail::binary, (abs==0x7F80) ? detail::invalid() : detail::signal(arg.data_));
                if(abs < 0x2700)
                        return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(arg.data_, 0, 1));
                if(bfloat16::round_style != std::round_to_nearest)
                        switch(abs)
                        {
                                case 0x658C: return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>((arg.data_&0x8000)|0x07E6, 0, 1));
                                case 0x7330: return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>((~arg.data_&0x8000)|0x4B62, 1, 1));
                        }
                std::pair<detail::uint32,detail::uint32> sc = detail::sincos(detail::angle_arg(abs,k),30);
                if(k & 1)
                        sc = std::make_pair(-sc.second, sc.first);
                detail::uint32 signy = detail::sign_mask(sc.first), signx = detail::sign_mask(sc.second);
                detail::uint32 my = (sc.first^signy) - signy, mx = (sc.second^signx) - signx;
                for(; my<0x80000000; my<<=1,--exp) ;
                for(; mx<0x80000000; mx<<=1,++exp) ;
                return bfloat16(detail::binary, detail::tangent_post<bfloat16::round_style>(my, mx, exp, (signy^signx^arg.data_)&0x8000));
        #endif
        }
  
        /// Arc sine.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::asin](https://en.cppreference.com/w/cpp/numeric/math/asin).
        /// \param arg function argument
        /// \return arc sine value of \a arg
        /// \exception FE_INVALID for signaling NaN or if abs(\a arg) > 1
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 asin(bfloat16 arg)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::asin(detail::bfloat162float<detail::internal_t>(arg.data_))));




	//temp
	#else

		return bfloat16(std::asin(detail::bfloat162float_temp(arg)));

	#endif


        #if 0
                unsigned int abs = arg.data_ & 0x7FFF, sign = arg.data_ & 0x8000;
                if(!abs)
                        return arg;
                if(abs >= 0x3F80)
                        return bfloat16(detail::binary, (abs>=0x7F80) ? detail::signal(arg.data_) : (abs>0x3F80) ? detail::invalid() : detail::rounded<bfloat16::round_style,true>(sign|0x3E48, 0, 1));
                if(abs < 0x2900)
                        return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(arg.data_, 0, 1));
                if(bfloat16::round_style != std::round_to_nearest && (abs == 0x2B44 || abs == 0x2DC3))
                        return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(arg.data_+1, 1, 1));
                std::pair<detail::uint32,detail::uint32> sc = detail::atan2_arg(abs);
                detail::uint32 m = detail::atan2(sc.first, sc.second, (bfloat16::round_style==std::round_to_nearest) ? 27 : 26);
                return bfloat16(detail::binary, detail::fixed2bfloat16<bfloat16::round_style, 30,false,true,true>(m, 14, sign));
        #endif
        }
  
        /// Arc cosine function.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::acos](https://en.cppreference.com/w/cpp/numeric/math/acos).
        /// \param arg function argument
        /// \return arc cosine value of \a arg
        /// \exception FE_INVALID for signaling NaN or if abs(\a arg) > 1
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 acos(bfloat16 arg)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::acos(detail::bfloat162float<detail::internal_t>(arg.data_))));



	//temp
	#else

		return bfloat16(std::acos(detail::bfloat162float_temp(arg)));

	#endif



        #if 0
                unsigned int abs = arg.data_ & 0x7FFF, sign = arg.data_ >> 15;
                if(!abs)
                        return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(0x3E48, 0, 1));
                if(abs >= 0x3F80)
                        return bfloat16(detail::binary, (abs>0x7F80) ? detail::signal(arg.data_) : (abs>0x3F80) ? detail::invalid() : sign ? detail::rounded<bfloat16::round_style,true>(0x4248, 0, 1) : 0);
                std::pair<detail::uint32,detail::uint32> cs = detail::atan2_arg(abs);
                detail::uint32 m = detail::atan2(cs.second, cs.first, 28);
                return bfloat16(detail::binary, detail::fixed2bfloat16<bfloat16::round_style, 31,false,true,true>(sign ? (0xC90FDAA2-m) : m, 15, 0, sign));
        #endif
        }
  
        /// Arc tangent function.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::atan](https://en.cppreference.com/w/cpp/numeric/math/atan).
        /// \param arg function argument
        /// \return arc tangent value of \a arg
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 atan(bfloat16 arg)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::atan(detail::bfloat162float<detail::internal_t>(arg.data_))));




	//temp
	#else

		return bfloat16(std::atan(detail::bfloat162float_temp(arg)));

	#endif


        #if 0
                unsigned int abs = arg.data_ & 0x7FFF, sign = arg.data_ & 0x8000;
                if(!abs)
                        return arg;
                if(abs >= 0x7F80)
                        return bfloat16(detail::binary, (abs==0x7F80) ? detail::rounded<bfloat16::round_style,true>(sign|0x3E48, 0, 1) : detail::signal(arg.data_));
                if(abs <= 0x2700)
                        return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(arg.data_-1, 1, 1));
                int exp = (abs>>10) + (abs<=0x7F);
                detail::uint32 my = (abs&0x7F) | ((abs>0x7F)<<10);
                detail::uint32 m = (exp>15) ? detail::atan2(my<<19, 0x20000000>>(exp-15), (bfloat16::round_style==std::round_to_nearest) ? 26 : 24) : detail::atan2(my<<(exp+4), 0x20000000, (bfloat16::round_style==std::round_to_nearest) ? 30 : 28);
                return bfloat16(detail::binary, detail::fixed2bfloat16<bfloat16::round_style, 30,false,true,true>(m, 14, sign));
        #endif
        }
  
        /// Arc tangent function.
        /// This function may be 1 ULP off the correctly rounded exact result in ~0.005% of inputs for `std::round_to_nearest`, 
        /// in ~0.1% of inputs for `std::round_toward_zero` and in ~0.02% of inputs for any other rounding mode.
        ///
        /// **See also:** Documentation for [std::atan2](https://en.cppreference.com/w/cpp/numeric/math/atan2).
        /// \param y numerator
        /// \param x denominator
        /// \return arc tangent value
        /// \exception FE_INVALID if \a x or \a y is signaling NaN
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 atan2(bfloat16 y, bfloat16 x)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::atan2(detail::bfloat162float<detail::internalt_t>(y.data_), detail::bfloat162float<detail::internal_t>(x.data_))));



	//temp
	#else

		return bfloat16(std::atan2(detail::bfloat162float_temp(y), detail::bfloat162float_temp(x)));
	
	#endif



        #if 0
                unsigned int absx = x.data_ & 0x7FFF, absy = y.data_ & 0x7FFF, signx = x.data_ >> 15, signy = y.data_ & 0x8000;
                if(absx >= 0x7F80 || absy >= 0x7F80)
                {
                        if(absx > 0x7F80 || absy > 0x7F80)
                                return bfloat16(detail::binary, detail::signal(x.data_, y.data_));
                        if(absy == 0x7F80)
                                return bfloat16(detail::binary, (absx<0x7F80) ? detail::rounded<bfloat16::round_style,true>(signy|0x3E48, 0, 1) : signx ? detail::rounded<bfloat16::round_style,true>(signy|0x40B5, 0, 1) : detail::rounded<bfloat16::round_style,true>(signy|0x3A48, 0, 1));
                        return (x.data_==0x7F80) ? bfloat16(detail::binary, signy) : bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(signy|0x4248, 0, 1));
                }
                if(!absy)
                        return signx ? bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(signy|0x4248, 0, 1)) : y;
                if(!absx)
                        return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(signy|0x3E48, 0, 1));
                int d = (absy>>10) + (absy<=0x7F) - (absx>>10) - (absx<=0x7F);
                if(d > (signx ? 18 : 12))
                        return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(signy|0x3E48, 0, 1));
                if(signx && d < -11)
                        return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(signy|0x4248, 0, 1));
                if(!signx && d < ((bfloat16::round_style==std::round_toward_zero) ? -15 : -9))
                {
                        for(; absy<0x80; absy<<=1,--d) ;
                        detail::uint32 mx = ((absx<<1)&0xFF) | 0x100, my = ((absy<<1)&0xFF) | 0x100;
                        int i = my < mx;
                        d -= i;
                        if(d < -25)
                                return bfloat16(detail::binary, detail::underflow<bfloat16::round_style>(signy));
                        my <<= 11 + i;
                        return bfloat16(detail::binary, detail::fixed2bfloat16<bfloat16::round_style, 11,false,false,true>(my/mx, d+14, signy, my%mx!=0));
                }
                detail::uint32 m = detail::atan2(((absy&0x7F)|((absy>0x7F)<<10))<<(19+((d<0) ? d : (d>0) ? 0 : -1)), ((absx&0x7F)|((absx>0x7F)<<10))<<(19-((d>0) ? d : (d<0) ? 0 : 1)));
                return bfloat16(detail::binary, detail::fixed2bfloat16<bfloat16::round_style, 31,false,true,true>(signx ? (0xC90FDAA2-m) : m, 15, signy, signx));
        #endif
        }
  
        /// \}
        /// \anchor hyperbolic
        /// \name Hyperbolic functions
        /// \{

        /// Hyperbolic sine.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::sinh](https://en.cppreference.com/w/cpp/numeric/math/sinh).
        /// \param arg function argument
        /// \return hyperbolic sine value of \a arg
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 sinh(bfloat16 arg)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_Style>(std::sinh(detail::bfloat162float<detail::internal_t>(arg.data_))));







	//temp
	#else

		return bfloat16(std::sinh(detail::bfloat162float_temp(arg)));

	#endif







        #if 0
                int abs = arg.data_ & 0x7FFF, exp;
                if(!abs || abs >= 0x7F80)
                        return (abs>0x7F80) ? bfloat16(detail::binary, detail::signal(arg.data_)) : arg;
                if(abs <= 0x2900)
                        return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(arg.data_, 0 ,1));
                std::pair<detail::uint32,detail::uint32> mm = detail::hyperbolic_args(abs, exp, (bfloat16::round_style==std::round_to_nearest) ? 29 : 17);
                detail::uint32 m = mm.first - mm.second;
                for(exp+=13; m<0x80000000 && exp; m<<=1,--exp) ;
                unsigned int sign = arg.data_ & 0x8000;
                if(exp > 29)
                        return bfloat16(detail::binary, detail::overflow<bfloat16::round_style>(sign));
                return bfloat16(detail::binary, detail::fixed2bfloat16<bfloat16::round_style, 31,false,false,true>(m, exp, sign));
        #endif
        }
  
        /// Hyperbolic cosine.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::cosh](https://en.cppreference.com/w/cpp/numeric/math/cosh).
        /// \param arg function argument
        /// \return hyperbolic cosine value of \a arg
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 cosh(bfloat16 arg)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::cosh(detail::bfloat162float<detail::internal_t>(arg.data_))));



	//temp
	#else
	
		return bfloat16(std::cosh(detail::bfloat162float_temp(arg)));

	#endif



        #if 0
                int abs = arg.data_ & 0x7FFF, exp;
                if(!abs)
                        return bfloat16(detail::binary, 0x3F80);
                if(abs >= 0x7F80)
                        return bfloat16(detail::binary, (abs>0x7F80) ? detail::signal(arg.data_) : 0x7F80);
                std::pair<detail::uint32,detail::uint32> mm = detail::hyperbolic_args(abs, exp, (bfloat16::round_style==std::round_to_nearest) ? 23 : 26);
                detail::uint32 m = mm.first + mm.second, i = (~m&0xFFFFFFFF) >> 31;
                m = (m>>i) | (m&i) | 0x80000000;
                if((exp+=13+i) > 29)
                        return bfloat16(detail::binary, detail::overflow<bfloat16::round_style>());
                return bfloat16(detail::binary, detail::fixed2bfloat16<bfloat16::round_style, 31,false,false,true>(m, exp));
        #endif
        }
  
        /// Hyperbolic tangent.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::tanh](https://en.cppreference.com/w/cpp/numeric/math/tanh).
        /// \param arg function argument
        /// \return hyperbolic tangent value of \a arg
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 tanh(bfloat16 arg)
        {
        #ifdef BFLOAT_ARITHMETIC_TYPE
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::tanh(detail::bfloat162float<detail::internal_t>(arg.data_))));






	//temp
	#else

		return bfloat16(std::tanh(detail::bfloat162float_temp(arg)));

	#endif




        #if 0
                int abs = arg.data_ & 0x7FFF, exp;
                if(!abs)
                        return arg;
                if(abs >= 0x7F80)
                        return bfloat16(detail::binary, (abs>0x7F80) ? detail::signal(arg.data_) : (arg.data_-0x4000));
                if(abs >= 0x4500)
                        return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>((arg.data_&0x8000)|0x3BFF, 1, 1));
                if(abs < 0x2700)
                        return bfloat16(detail::binary, detail::rounded<bfloat16::round_style, true>(arg.data_-1, 1, 1));
                if(bfloat16::round_style != std::round_to_nearest && abs == 0x2D3F)
                        return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(arg.data_-3, 0, 1));
                std::pair<detail::uint32,detail::uint32> mm = detail::hyperbolic_args(abs, exp, 27);
                detail::uint32 my = mm.first - mm.second - (bfloat16::round_style!=std::round_to_nearest), mx = mm.first + mm.second, i = (~mx&0xFFFFFFFF) >> 31;
                for(exp=13; my<0x80000000; my<<=1,--exp) ;
                mx = (mx>>i) | 0x80000000;
                return bfloat16(detail::binary, detail::tangent_post<bfloat16::round_style>(my, mx, exp-i, arg.data_&0x8000));
        #endif
        }
  
        /// Hyperbolic area sine.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::asinh](https://en.cppreference.com/w/cpp/numeric/math/asinh).
        /// \param arg function argument
        /// \return area sine value of \a arg
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 asinh(bfloat16 arg)
        {
        #if defined(BFLOAT_ARITHMETIC_TYPE) && BFLOAT_ENABLE_CPP11_CMATH
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::asinh(detail::bfloat162float<detail::internal_t>(arg.data_))));



	//temp
	#else

		return bfloat16(std::asinh(detail::bfloat162float_temp(arg)));

	#endif


        #if 0
                int abs = arg.data_ & 0x7FFF;
                if(!abs || abs >= 0x7F80)
                        return (abs>0x7F80) ? bfloat16(detail::binary, detail::signal(arg.data_)) : arg;
                if(abs <= 0x2900)
                        return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(arg.data_-1, 1, 1));
                if(bfloat16::round_style != std::round_to_nearest)
                        switch(abs)
                        {
                                case 0x32D4: return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(arg.data_-13, 1, 1));
                                case 0x3B5B: return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(arg.data_-197, 1, 1));
                        }
                return bfloat16(detail::binary, detail::area<bfloat16::round_style,true>(arg.data_));
        #endif
        }
  
        /// Hyperbolic area cosine.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::acosh](https://en.cppreference.com/w/cpp/numeric/math/acosh).
        /// \param arg function argument
        /// \return area cosine value of \a arg
        /// \exception FE_INVALID for signaling NaN or arguments <1
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 acosh(bfloat16 arg)
        {
        #if defined(BFLOAT_ARITHMETIC_TYPE) && BFLOAT_ENABLE_CPP11_CMATH
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::acosh(detail::bfloat162float<detail::internal_t>(arg.data_))));




	//temp
	#else

		return bfloat16(std::acosh(detail::bfloat162float_temp(arg)));

	#endif



        #if 0
                int abs = arg.data_ & 0x7FFF;
                if((arg.data_&0x8000) || abs <0x3F80)
                        return bfloat16(detail::binary, (abs<=0x7F80) ? detail::invalid() : detail::signal(arg.data_));
                if(abs == 0x3F80)
                        return bfloat16(detail::binary, 0);
                if(arg.data_ >= 0x7F80)
                        return (abs>0x7F80) ? bfloat16(detail::binary, detail::signal(arg.data_)) : arg;
                return bfloat16(detail::binary, detail::area<bfloat16::round_style,false>(arg.data_));
        #endif
        }
  
        /// Hyperbolic area tangent.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::atanh](https://en.cppreference.com/w/cpp/numeric/math/atanh).
        /// \param arg function argument
        /// \return area tangent value of \a arg
        /// \exception FE_INVALID for signaling NaN or if abs(\a arg) > 1
        /// \exception FE_DIVBYZERO for +/-1
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 atanh(bfloat16 arg)
        {
        #if defined(BFLOAT_ARITHMETIC_TYPE) && BFLOAT_ENABLE_CPP11_CMATH
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::atanh(detail::bfloat162float<detail::internal_t>(arg.data_))));









	//temp
	#else

		return bfloat16(std::atanh(detail::bfloat162float_temp(arg)));

	#endif





        #if 0
                int abs = arg.data_ & 0x7FFF, exp = 0;
                if(!abs)
                        return arg;
                if(abs >= 0x3F80)
                        return bfloat16(detail::binary, (abs== 0x3F80) ? detail::pole(arg.data_&0x8000) : (abs<= 0x7F80) ? detail::invalid() : detail::signal(arg.data_));
                if(abs < 0x2700)
                        return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>(arg.data_, 0, 1));
                detail::uint32 m = static_cast<detail::uint32>((abs&0x7F)|((abs>0x7F)<<10)) << ((abs>>10)+(abs<=0x7F)+6), my = 0x80000000 + m, mx = 0x80000000 - m;
                for(; mx<0x80000000; mx<<=1, ++exp) ;
                int i = my >= mx, s;
                return bfloat16(detail::binary, detail::log2_post<bfloat16::round_style, 0xB8AA3B2A>(detail::log2((detail::divide64(my>>i, mx, s)+1)>>1, 27) +0x10, exp+i-1, 16, arg.data_&0x8000));
        #endif
        }
  
        /// \}
        /// \anchor special
        /// \name Error and gamma functions
        /// \{

        /// Error function.
        /// This function may be 1 ULP off the correctly rounded exact result for any rounding mode in <0.5% of inputs.
        ///
        /// **See also:** Documentation for [std::erf](https://en.cppreference.com/w/cpp/numeric/math/erf).
        /// \param arg function argument
        /// \return error function value of \a arg
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 erf(bfloat16 arg)
        {
        #if defined(BFLOAT_ARITHMETIC_TYPE) && BFLOAT_ENABLE_CPP11_CMATH
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::erf(detail::bfloat162float<detail::internal_t>(arg.data_))));



	//temp
	#else

		return bfloat16(std::erf(detail::bfloat162float_temp(arg)));

	#endif



        #if 0
                unsigned int abs = arg.data_ & 0x7FFF;
                if(!abs || abs >= 0x7F80)
                        return (abs>=0x7F80) ? bfloat16(detail::binary, (abs==0x7F80) ? (arg.data_-0x4000) : detail::signal(arg.data_)) : arg;
                if(abs >= 0x4200)
                        return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>((arg.data_&0x8000)|0x3BFF, 1, 1));
                return bfloat16(detail::binary, detail::erf<bfloat16::round_style,false>(arg.data_));
        #endif
        }
  
        /// Complementary error function.
        /// This function may be 1 ULP off the correctly rounded exact result for any rounding mode in <0.5% of inputs.
        ///
        /// **See also:** Documentation for [std::erfc](https://en.cppreference.com/w/cpp/numeric/math/erfc).
        /// \param arg function argument
        /// \return 1 minus error function value of \a arg
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 erfc(bfloat16 arg)
        {
        #if defined(BFLOAT_ARITHMETIC_TYPE) && BFLOAT_ENABLE_CPP11_CMATH
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::erfc(detail::bfloat162float<detail::internal_t>(arg.data_))));



	//temp
	#else

		return bfloat16(std::erfc(detail::bfloat162float_temp(arg)));

	#endif


        #if 0
                unsigned int abs = arg.data_ & 0x7FFF, sign = arg.data_ & 0x8000;
                if(abs >= 0x7F80)
                        return (abs>=0x7F80) ? bfloat16(detail::binary, (abs==0x7F80) ? (sign>>1) : detail::signal(arg.data_)) : arg;
                if(!abs)
                        return bfloat16(detail::binary, 0x3F80);
                if(abs >= 0x440)
                        return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,true>((sign>>1)-(sign>>15), sign>>15, 1));
                return bfloat16(detail::binary, detail::erf<bfloat16::round_style,true>(arg.data_));
        #endif
        }
  
        /// Natural logarithm of gamma function.
        /// This function may be 1 ULP off the correctly rounded exact result for any rounding mode in ~0.025% of inputs.
        ///
        /// **See also:** Documentation for [std::lgamma](https://en.cppreference.com/w/cpp/numeric/math/lgamma).
        /// \param arg function argument
        /// \return natural logarith of gamma function for \a arg
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_DIVBYZERO for 0 or negative integer arguments
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 lgamma(bfloat16 arg)
        {
        #if defined(BFLOAT_ARITHMETIC_TYPE) && BFLOAT_ENABLE_CPP11_CMATH
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style(std::lgamma(detail::bfloat162float<detail::internal_t>(arg.data_))));




	//temp
	#else

		return bfloat16(std::lgamma(detail::bfloat162float_temp(arg)));

	#endif



        #if 0
                int abs = arg.data_ & 0x7FFF;
                if(abs >= 0x7F80)
                        return bfloat16(detail::binary, (abs==0x7F80) ? 0x7F80 : detail::signal(arg.data_));
                if(!abs || arg.data_ >= 0xE400 || (arg.data_ >= 0xBC00 && !(abs&((1<<(25-(abs>>10)))-1))))
                        return bfloat16(detail::binary, detail::pole());
                if(arg.data_ == 0x3F80 || arg.data_ == 0x4000)
                        return bfloat16(detail::binary, 0);
                return bfloat16(detail::binary, detail::gamma<bfloat16::round_style,true>(arg.data_));
        #endif
        }
  
        /// Gamma function.
        /// This function may be 1 ULP off the correctly rounded exact result for any rounding mode in <0.25% of inputs.
        ///
        /// **See also:** Documentation for [std::tgamma](https://en.cppreference.com/w/cpp/numeric/math/tgamma).
        /// \param arg function argument
        /// \return gamma function value of \a arg
        /// \exception FE_INVALID for signaling NaN, negative infinity or negative integer arguments
        /// \exception FE_DIVBYZERO for 0
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 tgamma(bfloat16 arg)
        {
        #if defined(BFLOAT_ARITHMETIC_TYPE) && BFLOAT_ENABLE_CPP11_CMATH
                return bfloat16(detail::binary, detail::float2bfloat16<bfloat16::round_style>(std::tgamma(detail::bfloat162float<detail::internal_t>(arg.data_))));




	//temp
	#else

		return bfloat16(std::tgamma(detail::bfloat162float_temp(arg)));

	#endif


        #if 0
                unsigned int abs = arg.data_ & 0x7FFF;
                if(!abs)
                        return bfloat16(detail::binary, detail::pole(arg.data_));
                if(abs >= 0x7F80)
                        return (arg.data_==0x7F80) ? arg : bfloat16(detail::binary, detail::signal(arg.data_));
                if(arg.data_ >= 0xE400 || (arg.data_ >= 0xBC00 && !(abs&((1<<(25-(abs>>10)))-1))))
                        return bfloat16(detail::binary, detail::invalid());
                if(arg.data_ >= 0xCA80)
                        return bfloat16(detail::binary, detail::underflow<bfloat16::round_style>((1-((abs>>(25-(abs>>10)))&1))<<15));
                if(arg.data_ >= 0x100 || (arg.data_ >= 0x4900 && arg.data_ > 0x8000))
                        return bfloat16(detail::binary, detail::overflow<bfloat16::round_style>());
                if(arg.data_ == 0x3F80)
                        return arg;
                return bfloat16(detail::binary, detail::gamma<bfloat16::round_style,false>(arg.data_));
        #endif
        }
  
        /// \}
        /// \anchor rounding
        /// \name Rounding
        /// \{

        /// Nearest integer not less than bfloat16 value.
        /// **See also:** Documentation for [std::ceil](https://en.cppreference.com/w/cpp/numeric/math/ceil).
        /// \param arg bfloat16 to round
        /// \return nearest integer not less than \a arg
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_INEXACT if value had to be rounded
        inline bfloat16 ceil(bfloat16 arg) { return bfloat16(detail::binary, detail::integral<std::round_toward_infinity,true,true>(arg.data_)); }
  
        /// Nearest integer not greater than bfloat16 value.
        /// **See also:** Documentation for [std::floor](https://en.cppreference.com/w/cpp/numeric/math/floor).
        /// \param arg bfloat16 to round
        /// \return nearest integer not greater than \a arg
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_INEXACT if value had to be rounded
        inline bfloat16 floor(bfloat16 arg) { return bfloat16(detail::binary, detail::integral<std::round_toward_neg_infinity,true,true>(arg.data_)); }
  
        /// Nearest integer not greater in magnitude than bfloat16 value.
        /// **See also:** Documentation for [std::trunc](https://en.cppreference.com/w/cpp/numeric/math/trunc).
        /// \param arg bfloat16 to round
        /// \return nearest integer not greater in magnitude than \a arg
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_INEXACT if value had to be rounded
        inline bfloat16 trunc(bfloat16 arg) { return bfloat16(detail::binary, detail::integral<std::round_toward_zero,true,true>(arg.data_)); }
  
        /// Nearest integer.
        /// **See also:** Documentation for [std::round](https://en.cppreference.com/w/cpp/numeric/math/round).
        /// \param arg bfloat16 to round
        /// \return nearest integer, rounded away from zero in bfloat16-way cases
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_INEXACT if value had to be rounded
        inline bfloat16 round(bfloat16 arg) { return bfloat16(detail::binary, detail::integral<std::round_to_nearest,false,true>(arg.data_)); }
  
        /// Nearest integer.
        /// **See also:** Documentation for [std::lround](https://en.cppreference.com/w/cpp/numeric/math/round).
        /// \param arg bfloat16 to round
        /// \return nearest integer, rounded away from zero in bfloat16-way cases
        /// \exception FE_INVALID if value is not representable as `long`
        inline long lround(bfloat16 arg) { return detail::bfloat162int<std::round_to_nearest,false,false,long>(arg.data_); }
  
        /// Nearest integer using bfloat16's internal rounding mode.
        /// **See also:** Documentation for [std::rint](https://en.cppreference.com/w/cpp/numeric/math/rint).
        /// \param arg bfloat16 expression to round
        /// \return nearest integer using default rounding mode
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_INEXACT if value had to be rounded
        inline bfloat16 rint(bfloat16 arg) { return bfloat16(detail::binary, detail::integral<bfloat16::round_style,true,true>(arg.data_)); }
  
        /// Nearest integer using bfloat16's internal rounding mode.
        /// **See also:** Documentation for [std::lrint](https://en.cppreference.com/w/cpp/numeric/math/rint).
        /// \param arg bfloat16 expression to round
        /// \return nearest integer using default rounding mode
        /// \exception FE_INVALID if value is not representable as `long`
        /// \exception FE_INEXACT if value had to be rounded
        inline long lrint(bfloat16 arg) { return detail::bfloat162int<bfloat16::round_style,true,true,long>(arg.data_); }
  
        /// Nearest integer using bfloat16's internal rounding mode.
        /// **See also:** Documentation for [std::nearbyint](https://en.cppreference.com/w/cpp/numeric/math/nearbyint).
        /// \param arg bfloat16 expression to round
        /// \return nearest integer using default rounding mode
        /// \exception FE_INVALID for signaling NaN
        inline bfloat16 nearbyint(bfloat16 arg) { return bfloat16(detail::binary, detail::integral<bfloat16::round_style,true,false>(arg.data_)); }
  
#if BFLOAT_ENABLE_CPP11_LONG_LONG
        /// Nearest integer.
        /// **See also:** Documentation for [std::llround](https://en.cppreference.com/w/cpp/numeric/math/round).
        /// \param arg bfloat16 to round
        /// \return nearest integer, rounded away from zero in bfloat16-way cases
        /// \exception FE_INVALID if value is not representable as `long long`
        inline long long llround(bfloat16 arg) { return detail::bfloat162int<std::round_to_nearest,false,false,long long>(arg.data_); }
  
        /// Nearest integer using bfloat16's internal rounding mode.
        /// **See also:** Documentation for [std::llrint](https://en.cppreference.com/w/cpp/numeric/math/rint).
        /// \param arg bfloat16 expression to round
        /// \return nearest integer using default rounding mode
        /// \exception FE_INVALID if value is not representable as `long long`
        /// \exception FE_INEXACT if value had to be rounded
        inline long long llrint(bfloat16 arg) { return detail::bfloat162int<bfloat16::round_style,true,true,long long>(arg.data_); }
#endif
  
        /// \}
        /// \anchor float
        /// \name Floating point manipulation
        /// \{

        /// Decompress floating-point number.
        /// **See also:** Documentation for [std::frexp](https://en.cppreference.com/w/cpp/numeric/math/frexp).
        /// \param arg number to decompress
        /// \param exp address to store exponent at
        /// \return significant in range [0.5, 1)
        /// \exception FE_INVALID for signaling NaN
        inline bfloat16 frexp(bfloat16 arg, int *exp)
        {
                *exp = 0;
                unsigned int abs = arg.data_ & 0x7FFF;
                if(abs >= 0x7F80 || !abs)
                        return (abs>0x7F80) ? bfloat16(detail::binary, detail::signal(arg.data_)) : arg;
                for(; abs<0x80; abs<<=1,--*exp) ;
                * exp += (abs>>10) -14;
                return bfloat16(detail::binary, (arg.data_&0x8000)|0x3800|(abs&0x7F));
        }
  
        /// Multiply by power of two.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::scalbln](https://en.cppreference.com/w/cpp/numeric/math/scalbn).
        /// \param arg number to modify
        /// \param exp power of two to multiply with
        /// \return \a arg multplied by 2 raised to \a exp
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 scalbln(bfloat16 arg, long exp)
        {

		//temp
		return bfloat16(std::scalbln(detail::bfloat162float_temp(arg), exp));

                unsigned int abs = arg.data_ & 0x7FFF, sign = arg.data_ & 0x8000;
                if(abs >= 0x7F80 || !abs)
                        return (abs>0x7F80) ? bfloat16(detail::binary, detail::signal(arg.data_)) : arg;
                for(; abs<0x80; abs<<=1,--exp) ;
                exp += abs >> 10;
                if(exp > 30)
                        return bfloat16(detail::binary, detail::overflow<bfloat16::round_style>(sign));
                else if(exp < -10)
                        return bfloat16(detail::binary, detail::underflow<bfloat16::round_style>(sign));
                else if(exp > 0)
                        return bfloat16(detail::binary, sign|(exp<<10)|(abs&0x7F));
                unsigned int m = (abs&0x7F) | 0x80;
                return bfloat16(detail::binary, detail::rounded<bfloat16::round_style,false>(sign|(m>>(1-exp)), (m>>-exp)&1, (m&((1<<-exp)-1))!=0));
        }
  
        /// Multiply by power of two.
        /// This function is exact to rounding for all rounding modes.
        ///
        /// **See also:** Documentation for [std::scalbn](https://en.cppreference.com/w/cpp/numeric/math/scalbn).
        /// \param arg number to modify
        /// \param exp power of two to multiply with
        /// \return \a arg multplied by 2 raised to \a exp
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        inline bfloat16 scalbn(bfloat16 arg, int exp) { return scalbln(arg, exp); }
  
	/// Multiply by power of two.
	/// This function is exact to rounding for all rounding modes.
	///
	/// **See also:** Documentation for [std::ldexp](https://en.cppreference.com/w/cpp/numeric/math/ldexp).
	/// \param arg number to modify
	/// \param exp power of two to multiply with
	/// \return \a arg multplied by 2 raised to \a exp
	/// \exception FE_INVALID for signaling NaN
	/// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
	inline bfloat16 ldexp(bfloat16 arg, int exp) { return scalbln(arg, exp); }
  
        /// Extract integer and fractional parts.
        /// **See also:** Documentation for [std::modf](https://en.cppreference.com/w/cpp/numeric/math/modf).
        /// \param arg number to decompress
        /// \param iptr address to store integer part at
        /// \return fractional part
        /// \exception FE_INVALID for signaling NaN
        inline bfloat16 modf(bfloat16 arg, bfloat16 *iptr)
        {
                unsigned int abs = arg.data_ & 0x7FFF;
                if(abs > 0x7F80)
                {
                        arg = bfloat16(detail::binary, detail::signal(arg.data_));
                        return *iptr = arg, arg;
                }
                if(abs >= 0x6400)
                        return *iptr = arg, bfloat16(detail::binary, arg.data_&0x8000);
                if(abs < 0x3F80)
                        return iptr->data_ = arg.data_ & 0x8000, arg;
                unsigned int exp = abs >> 10, mask = (1<<(25-exp)) -1, m = arg.data_ & mask;
                iptr->data_ = arg.data_ & ~mask;
                if(!m)
                        return bfloat16(detail::binary, arg.data_&0x8000);
                for(; m<0x80; m<<=1,--exp) ;
                return bfloat16(detail::binary, (arg.data_&0x8000)|(exp<<10)|(m&0x7F));
        }
  
        /// Extract exponent.
        /// **See also:** Documentation for [std::ilogb](https://en.cppreference.com/w/cpp/numeric/math/ilogb).
        /// \param arg number to query
        /// \return floating-point exponent
        /// \retval FP_ILOGB0 for zero
        /// \retval FP_ILOGBNAN for NaN
        /// \retval INT_MAX for infinity
        /// \exception FE_INVALID for 0 or infinite values
        inline int ilogb(bfloat16 arg)
        {
                int abs = arg.data_ & 0x7FFF, exp;
                if(!abs || abs >= 0x7F80)
                {
                        detail::raise(FE_INVALID);
                        return !abs ? FP_ILOGB0 : (abs==0x7F80) ? INT_MAX : FP_ILOGBNAN;
                }
                for(exp=(abs>>7)-127; abs<0x40; abs<<=1,--exp) ;
                return exp;
        }
  
        /// Extract exponent.
        /// **See also:** Documentation for [std::logb](https://en.cppreference.com/w/cpp/numeric/math/logb).
        /// \param arg number to query
        /// \return floating-point exponent
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_DIVBYZERO for 0

        inline bfloat16 logb(bfloat16 arg)
        {

		//temp
		return bfloat16(std::logb(detail::bfloat162float_temp(arg)));



                int abs = arg.data_ & 0x7FFF, exp;
                if(!abs)
                        return bfloat16(detail::binary, detail::pole(0x8000));
                if(abs >= 0x7F80)
                        return bfloat16(detail::binary, (abs==0x7F80) ? 0x7F80 : detail::signal(arg.data_));
                for(exp=(abs>>7)-127; abs<0x40; abs<<=1,--exp) ;
                unsigned int value = static_cast<unsigned>(exp<0) << 15;
                if(exp)
                {
                        unsigned int m = std::abs(exp) << 6;//why 6
                        for(exp=18; m<0x80; m<<=1,--exp) ;//why 18
                        value |= (exp<<7) + m;
                }
                return bfloat16(detail::binary, value);
        }
  
        /// Next representable value.
        /// **See also:** Documentation for [std::nextafter](https://en.cppreference.com/w/cpp/numeric/math/nextafter).
        /// \param from value to compute next representable value for
        /// \param to direction towards which to compute next value
        /// \return next representable value after \a from in direction towards \a to
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_OVERFLOW for infinite result from finite argument
        /// \exception FE_UNDERFLOW for subnormal result
        inline bfloat16 nextafter(bfloat16 from, bfloat16 to)
        {
                int fabs = from.data_ & 0x7FFF, tabs = to.data_ & 0x7FFF;
                if(fabs > 0x7F80 || tabs > 0x7F80)
                        return bfloat16(detail::binary, detail::signal(from.data_, to.data_));
                if(from.data_ == to.data_ || !(fabs|tabs))
                        return to;
                if(!fabs)
                {
                        detail::raise(FE_UNDERFLOW, !BFLOAT_ERRHANDLING_UNDERFLOW_TO_INEXACT);
                        return bfloat16(detail::binary, (to.data_&0x8000)+1);
                }
                unsigned int out = from.data_ + (((from.data_>>15)^static_cast<unsigned>((from.data_^(0x8000|(0x8000-(from.data_>>15))))<(to.data_^(0x8000|(0x8000-(to.data_>>15))))))<<1) -1;
                detail::raise(FE_OVERFLOW, fabs<0x7F80 && (out&0x7F80)==0x7F80);
                detail::raise(FE_UNDERFLOW, !BFLOAT_ERRHANDLING_UNDERFLOW_TO_INEXACT && (out& 0x7F80)<0x80);
                return bfloat16(detail::binary, out);
        }
  
        /// Next representable value.
        /// **See also:** Documentation for [std::nexttoward](https://en.cppreference.com/w/cpp/numeric/math/nexttoward).
        /// \param from value to compute next representable value for
        /// \param to direction towards which to compute next value
        /// \return next representable value after \a from in direction towards \a to
        /// \exception FE_INVALID for signaling NaN
        /// \exception FE_OVERFLOW for infinite result from finite argument
        /// \exception FE_UNDERFLOW for subnormal result
        inline bfloat16 nexttoward(bfloat16 from, long double to)
        {
                int fabs = from.data_ & 0x7FFF;
                if(fabs > 0x7F80)
                        return bfloat16(detail::binary, detail::signal(from.data_));
                long double lfrom = static_cast<long double>(from);
                if(detail::builtin_isnan(to) || lfrom == to)
                        return bfloat16(static_cast<float>(to));
                if(!fabs)
                {
                        detail::raise(FE_UNDERFLOW, !BFLOAT_ERRHANDLING_UNDERFLOW_TO_INEXACT);
                        return bfloat16(detail::binary, (static_cast<unsigned>(detail::builtin_signbit(to))<<15)+1);
                }
                unsigned int out = from.data_ + (((from.data_>>15)^static_cast<unsigned>(lfrom<to))<<1) - 1;
                detail::raise(FE_UNDERFLOW, (out&0x7FFF)==0x7F80);
                detail::raise(FE_UNDERFLOW, !BFLOAT_ERRHANDLING_UNDERFLOW_TO_INEXACT && (out&0x7FFF) <0x80);
                return bfloat16(detail::binary, out);
        }
  
        /// Take sign.
        /// **See also:** Documentation for [std::copysign](https://en.cppreference.com/w/cpp/numeric/math/copysign).
        /// \param x value to change sign for
        /// \param y value to take sign from
        /// \return value equal to \a x in magnitude and to \a y in sign
        inline BFLOAT_CONSTEXPR bfloat16 copysign(bfloat16 x, bfloat16 y) { return bfloat16(detail::binary, x.data_^((x.data_^y.data_)&0x8000)); }
  
        /// \}
        /// \anchor classification
        /// \name Floating point classification
        /// \{

        /// Classify floating-point value.
        /// **See also:** Documentation for [std::fpclassify](https://en.cppreference.com/w/cpp/numeric/math/fpclassify).
        /// \param arg number to classify
        /// \retval FP_ZERO for positive and negative zero
        /// \retval FP_SUBNORMAL for subnormal numbers
        /// \retval FP_INFINITY for positive and negative infinity
        /// \retval FP_NAN for NaNs
        /// \retval FP_NORMAL for all other (normal) values
        inline BFLOAT_CONSTEXPR int fpclassify(bfloat16 arg)
        {
          return !(arg.data_&0x7FFF) ? FP_ZERO :
              ((arg.data_&0x7FFF)<0x80) ? FP_SUBNORMAL :
              ((arg.data_&0x7FFF)<0x7F80) ? FP_NORMAL :
              ((arg.data_&0x7FFF)==0x7F80) ? FP_INFINITE :
              FP_NAN;
        }
  
        /// Check if finite number.
        /// **See also:** Documentation for [std::isfinite](https://en.cppreference.com/w/cpp/numeric/math/isfinite).
        /// \param arg number to check
        /// \retval true if neither infinity nor NaN
        /// \retval false else
        inline BFLOAT_CONSTEXPR bool isfinite(bfloat16 arg) { return (arg.data_&0x7F80) != 0x7F80; }
  
        /// Check for infinity.
        /// **See also:** Documentation for [std::isinf](https://en.cppreference.com/w/cpp/numeric/math/isinf).
        /// \param arg number to check
        /// \retval true for positive or negative infinity
        /// \retval false else
        inline BFLOAT_CONSTEXPR bool isinf(bfloat16 arg) { return (arg.data_&0x7FFF) == 0x7F80; }
  
        /// Check for NaN.
        /// **See also:** Documentation for [std::isnan](https://en.cppreference.com/w/cpp/numeric/math/isnan).
        /// \param arg number to check
        /// \retval true for NaNs
        /// \retval false else
        inline BFLOAT_CONSTEXPR bool isnan(bfloat16 arg) { return (arg.data_&0x7FFF) > 0x7F80; }
  
        /// Check if normal number.
        /// **See also:** Documentation for [std::isnormal](https://en.cppreference.com/w/cpp/numeric/math/isnormal).
        /// \param arg number to check
        /// \retval true if normal number
        /// \retval false if either subnormal, zero, infinity or NaN
        inline BFLOAT_CONSTEXPR bool isnormal(bfloat16 arg) { return ((arg.data_&0x7F80)!=0) & ((arg.data_&0x7F80)!=0x7F80); }
  
        /// Check sign.
        /// **See also:** Documentation for [std::signbit](https://en.cppreference.com/w/cpp/numeric/math/signbit).
        /// \param arg number to check
        /// \retval true for negative number
        /// \retval false for positive number
        inline BFLOAT_CONSTEXPR bool signbit(bfloat16 arg) { return (arg.data_&0x8000) != 0;}
  
        /// \}
        /// \anchor compfunc
        /// \name Comparison
        /// \{

        /// Quiet comparison for greater than.
        /// **See also:** Documentation for [std::isgreater](https://en.cppreference.com/w/cpp/numeric/math/isgreater).
        /// \param x first operand
        /// \param y second operand
        /// \retval true if \a x greater than \a y
        /// \retval false else
        inline BFLOAT_CONSTEXPR bool isgreater(bfloat16 x, bfloat16 y)
        {
                return ((x.data_^(0x8000|(0x8000-(x.data_>>15))))+(x.data_>>15)) > ((y.data_^(0x8000|(0x8000-(y.data_>>15))))+(y.data_>>15)) && !isnan(x) && !isnan(y);
        }
  
        /// Quiet comparison for greater equal.
        /// **See also:** Documentation for [std::isgreaterequal](https://en.cppreference.com/w/cpp/numeric/math/isgreaterequal).
        /// \param x first operand
        /// \param y second operand
        /// \retval true if \a x greater equal \a y
        /// \retval false else
        inline BFLOAT_CONSTEXPR bool isgreaterequal(bfloat16 x, bfloat16 y)
        {
                return ((x.data_^(0x8000|(0x8000-(x.data_>>15))))+(x.data_>>15)) >= ((y.data_^(0x8000|(0x8000-(y.data_>>15))))+(y.data_>>15)) && !isnan(x) && !isnan(y);
        }
  
        /// Quiet comparison for less than.
        /// **See also:** Documentation for [std::isless](https://en.cppreference.com/w/cpp/numeric/math/isless).
        /// \param x first operand
        /// \param y second operand
        /// \retval true if \a x less than \a y
        /// \retval false else
        inline BFLOAT_CONSTEXPR bool isless(bfloat16 x, bfloat16 y)
        {
		return ((x.data_^(0x8000|(0x8000-(x.data_>>15))))+(x.data_>>15)) < ((y.data_^(0x8000|(0x8000-(y.data_>>15))))+(y.data_>>15)) && !isnan(x) && !isnan(y);
        }
  
        /// Quiet comparison for less equal.
        /// **See also:** Documentation for [std::islessequal](https://en.cppreference.com/w/cpp/numeric/math/islessequal).
        /// \param x first operand
        /// \param y second operand
        /// \retval true if \a x less equal \a y
        /// \retval false else
        inline BFLOAT_CONSTEXPR bool islessequal(bfloat16 x, bfloat16 y)
        {
                return ((x.data_^(0x8000|(0x8000-(x.data_>>15))))+(x.data_>>15)) <= ((y.data_^(0x8000|(0x8000-(y.data_>>15))))+(y.data_>>15)) && !isnan(x) && !isnan(y);
        }
  
        /// Quiet comparison for less or greater.
        /// **See also:** Documentation for [std::islessgreater](https://en.cppreference.com/w/cpp/numeric/math/islessgreater).
        /// \param x first operand
        /// \param y second operand
        /// \retval true if either less or greater
        /// \retval false else
        inline BFLOAT_CONSTEXPR bool islessgreater(bfloat16 x, bfloat16 y)
        {
                return x.data_!=y.data_ && ((x.data_|y.data_)&0x7FFF) && !isnan(x) && !isnan(y);
        }
  
        /// Quiet check if unordered.
        /// **See also:** Documentation for [std::isunordered](https://en.cppreference.com/w/cpp/numeric/math/isunordered).
        /// \param x first operand
        /// \param y second operand
        /// \retval true if unordered (one or two NaN operands)
        /// \retval false else
        inline BFLOAT_CONSTEXPR bool isunordered(bfloat16 x, bfloat16 y) { return isnan(x) || isnan(y); }
  
        /// \}
        /// \anchor casting
        /// \name Casting
        /// \{

        /// Cast to or from bfloat16-precision floating-point number.
        /// This casts between [bfloat16](\ref bfloat16::bfloat16) and any built-in arithmetic type. The values are converted 
        /// directly using the default rounding mode, without any roundtrip over `float` that a `static_cast` would otherwise do.
        ///
        /// Using this cast with neither of the two types being a [bfloat16](\ref bfloat16::bfloat16) or with any of the two types 
        /// not being a built-in arithmetic type (apart from [bfloat16](\ref bfloat16::bfloat16), of course) results in a compiler 
        /// error and casting between [bfloat16](\ref bfloat16::bfloat16)s returns the argument unmodified.
        /// \tparam T destination type (bfloat16 or built-in arithmetic type)
        /// \tparam U source type (bfloat16 or built-in arithmetic type)
        /// \param arg value to cast
        /// \return \a arg converted to destination type
        /// \exception FE_INVALID if \a T is integer type and result is not representable as \a T
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        template<typename T, typename U> T bfloat16_cast(U arg) { return detail::bfloat16_caster<T,U>::cast(arg); }
  
        /// Cast to or from bfloat16-precision floating-point number.
        /// This casts between [bfloat16](\ref bfloat16::bfloat16) and any built-in arithmetic type. The values are converted 
        /// directly using the specified rounding mode, without any roundtrip over `float` that a `static_cast` would otherwise do.
        ///
        /// Using this cast with neither of the two types being a [bfloat16](\ref bfloat16::bfloat16) or with any of the two types 
        /// not being a built-in arithmetic type (apart from [bfloat16](\ref bfloat16::bfloat16), of course) results in a compiler 
        /// error and casting between [bfloat16](\ref bfloat16::bfloat16)s returns the argument unmodified.
        /// \tparam T destination type (bfloat16 or built-in arithmetic type)
        /// \tparam R rounding mode to use.
        /// \tparam U source type (bfloat16 or built-in arithmetic type)
        /// \param arg value to cast
        /// \return \a arg converted to destination type
        /// \exception FE_INVALID if \a T is integer type and result is not representable as \a T
        /// \exception FE_OVERFLOW, ...UNDERFLOW, ...INEXACT according to rounding
        template<typename T, std::float_round_style R, typename U> T bfloat16_cast(U arg) { return detail::bfloat16_caster<T,U,R>::cast(arg); }
        /// \}

        /// \}
        /// \anchor errors
        /// \name Error handling
        /// \{

        /// Clear exception flags.
        /// This function works even if [automatic exception flag handling](\ref BFLOAT_ERRHANDLING_FLAGS) is disabled, 
        /// but in that case manual flag management is the only way to raise flags.
        ///
        /// **See also:** Documentation for [std::feclearexcept](https://en.cppreference.com/w/cpp/numeric/fenv/feclearexcept).
        /// \param excepts OR of exceptions to clear
        /// \retval 0 all selected flags cleared successfully
        inline int feclearexcept(int excepts) { detail::errflags() &= ~excepts; return 0; }
  
        /// Test exception flags.
        /// This function works even if [automatic exception flag handling](\ref BFLOAT_ERRHANDLING_FLAGS) is disabled, 
        /// but in that case manual flag management is the only way to raise flags.
        ///
        /// **See also:** Documentation for [std::fetestexcept](https://en.cppreference.com/w/cpp/numeric/fenv/fetestexcept).
        /// \param excepts OR of exceptions to test
        /// \return OR of selected exceptions if raised
        inline int fetestexcept(int excepts) { return detail::errflags() & excepts; }
  
        /// Raise exception flags.
        /// This raises the specified floating point exceptions and also invokes any additional automatic exception handling as 
        /// configured with the [BFLOAT_ERRHANDLIG_...](\ref BFLOAT_ERRHANDLING_ERRNO) preprocessor symbols.
        /// This function works even if [automatic exception flag handling](\ref BFLOAT_ERRHANDLING_FLAGS) is disabled, 
        /// but in that case manual flag management is the only way to raise flags.
        ///
        /// **See also:** Documentation for [std::feraiseexcept](https://en.cppreference.com/w/cpp/numeric/fenv/feraiseexcept).
        /// \param excepts OR of exceptions to raise
        /// \retval 0 all selected exceptions raised successfully
        inline int feraiseexcepts(int excepts) { detail::errflags() |= excepts; detail::raise(excepts); return 0;}
  
        /// Save exception flags.
        /// This function works even if [automatic exception flag handling](\ref BFLOAT_ERRHANDLING_FLAGS) is disabled, 
        /// but in that case manual flag management is the only way to raise flags.
        ///
        /// **See also:** Documentation for [std::fegetexceptflag](https://en.cppreference.com/w/cpp/numeric/fenv/feexceptflag).
        /// \param flagp adress to store flag state at
        /// \param excepts OR of flags to save
        /// \retval 0 for success
        inline int fegetexceptflag(int *flagp, int excepts) { *flagp = detail::errflags() & excepts; return 0; }
  
        /// Restore exception flags.
        /// This only copies the specified exception state (including unset flags) without incurring any additional exception handling.
        /// This function works even if [automatic exception flag handling](\ref BFLOAT_ERRHANDLING_FLAGS) is disabled, 
        /// but in that case manual flag management is the only way to raise flags.
        ///
        /// **See also:** Documentation for [std::fesetexceptflag](https://en.cppreference.com/w/cpp/numeric/fenv/feexceptflag).
        /// \param flagp adress to take flag state from
        /// \param excepts OR of flags to restore
        /// \retval 0 for success
        inline int fesetexceptflag(const int *flagp, int excepts) { detail::errflags() = (detail::errflags()|(*flagp&excepts)) & (*flagp|~excepts); return 0; }
  
        /// Throw C++ exceptions based on set exception flags.
        /// This function manually throws a corresponding C++ exception if one of the specified flags is set, 
        /// no matter if automatic throwing (via [BFLOAT_ERRHANDLING_THROW_...](\ref BFLOAT_ERRHANDLING_THROW_INVALID)) is enabled or not.
        /// This function works even if [automatic exception flag handling](\ref BFLOAT_ERRHANDLING_FLAGS) is disabled, 
        /// but in that case manual flag management is the only way to raise flags.
        /// \param excepts OR of exceptions to test
        /// \param msg error message to use for exception description
        /// \throw std::domain_error if `FE_INVALID` or `FE_DIVBYZERO` is selected and set
        /// \throw std::overflow_error if `FE_OVERFLOW` is selected and set
        /// \throw std::underflow_error if `FE_UNDERFLOW` is selected and set
        /// \throw std::range_error if `FE_INEXACT` is selected and set
        inline void fethrowexcept(int excepts, const char *msg = "")
        {
                excepts &= detail::errflags();
                if(excepts & (FE_INVALID|FE_DIVBYZERO))
                        throw std::domain_error(msg);
                if(excepts & FE_OVERFLOW)
                        throw std::overflow_error(msg);
                if(excepts & FE_UNDERFLOW)
                        throw std::underflow_error(msg);
                if(excepts & FE_INEXACT)
                        throw std::range_error(msg);
        }
        /// \}
}


#undef BFLOAT_UNUSED_NOERR
#undef BFLOAT_CONSTEXPR
#undef BFLOAT_CONSTEXPR_CONST
#undef BFLOAT_CONSTEXPR_NOERR
#undef BFLOAT_NOEXCEPT
#undef BFLOAT_NOTHROW
#undef BFLOAT_THREAD_LOCAL
#undef BFLOAT_TWOS_COMPLEMENT_INT
#ifdef BFLOAT_POP_WARNING
        #pragma warning(pop)
        #undef BFLOAT_POP_WARNINGS
#endif

#endif
