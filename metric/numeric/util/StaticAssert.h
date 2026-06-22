// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_STATICASSERT_H
#define METRIC_NUMERIC_UTIL_STATICASSERT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/Suffix.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  COMPILE TIME ASSERTION
//
//=================================================================================================

//*************************************************************************************************
/*!\defgroup static_assert Compile time assertion
// \ingroup assert
//
// Static assertion offers the possibility to stop the compilation process if a specific compile
// time condition is not met. The mtrc::numeric::METRIC_NUMERIC_STATIC_ASSERT and
mtrc::numeric::METRIC_NUMERIC_STATIC_ASSERT_MSG
// macros can be used to check a constant expression at compile time. If the expression evaluates
// to \a false, a compilation error is generated that stops the compilation process. If the
// expression (hopefully) evaluates to \a true, the compilation process is not aborted and the
// static check leaves neither code nor data and is therefore not affecting the performance.\n
//
// Both static assertion macros can be used wherever a standard typedef statement can be declared,
// i.e. in namespace scope, in class scope and in function scope. The following examples illustrate
// the use of the static assertion macros: The element type of the rotation matrix is checked at
// compile time and restricted to be of floating point type.

   \code
   #include <metric/numeric/util/StaticAssert.h>
   #include <metric/numeric/util/typetraits/FloatingPoint.h>

   template< typename T >
   class RotationMatrix {
	  ...
	  METRIC_NUMERIC_STATIC_ASSERT( IsFloatingPoint_v<T> );
	  // ... or ...
	  METRIC_NUMERIC_STATIC_ASSERT_MSG( IsFloatingPoint_v<T>, "Given type is not a floating point type" );
	  ...
   };
   \endcode

// The static assertion implementation is based on the C++11 \c static_assert declaration. Thus
// the error message contains the violated compile time condition and directly refers to the
// violated static assertion. The following examples show a possible error message from the GNU
// g++ compiler:

   \code
   error: static assertion failed: Compile time condition violated
	  static_assert( expr, "Compile time condition violated" )
	  ^
   note: in expansion of macro 'METRIC_NUMERIC_STATIC_ASSERT'
	  METRIC_NUMERIC_STATIC_ASSERT( IsFloatingPoint_v<T> );

   error: static assertion failed: Given type is not a floating point type
	  static_assert( expr, msg )
	  ^
   note: in expansion of macro 'METRIC_NUMERIC_STATIC_ASSERT_MSG'
	  METRIC_NUMERIC_STATIC_ASSERT_MSG( IsFloatingPoint_v<T>, "Given type is not a floating point type" );
   \endcode
*/
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time assertion macro.
// \ingroup static_assert
//
// In case of an invalid compile time expression, a compilation error is created.
*/
#define METRIC_NUMERIC_STATIC_ASSERT(expr) static_assert(expr, "Compile time condition violated")
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time assertion macro.
// \ingroup static_assert
//
// In case of an invalid compile time expression, a compilation error is created.
*/
#define METRIC_NUMERIC_STATIC_ASSERT_MSG(expr, msg) static_assert(expr, msg)
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
