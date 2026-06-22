// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_ASSERT_H
#define METRIC_NUMERIC_UTIL_ASSERT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <cassert>
#include <metric/numeric/system/Assertion.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  RUN TIME ASSERTION
//
//=================================================================================================

//*************************************************************************************************
/*!\defgroup assert Assertions
// \ingroup util
*/
/*!\defgroup runtime_assert Run time assertions
// \ingroup assert
*/
/*!\brief Assertion helper function.
// \ingroup runtime_assert
//
// The ASSERT_MESSAGE function is a small helper function to assist in printing an informative
// message in case an assert fires. This function builds on the ideas of Matthew Wilson, who
// directly combines a C-string error message with the run time expression (Imperfect C++,
// ISBN: 0321228774):

   \code
   assert( ... &&  "Error message" );
   assert( ... || !"Error message" );
   \endcode

// However, both approaches fail to compile without warning on certain compilers. Therefore
// this inline function is used instead of the direct approaches, which circumvents all compiler
// warnings:

   \code
   assert( ... || ASSERT_MESSAGE( "Error message" ) );
   \endcode
*/
inline bool ASSERT_MESSAGE(const char * /*msg*/) { return false; }
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Run time assertion macro for internal checks.
// \ingroup runtime_assert
//
// In case of an invalid run time expression, the program execution is terminated.\n
// The METRIC_NUMERIC_INTERNAL_ASSERT macro can be disabled by setting the \a METRIC_NUMERIC_USER_ASSERTION
// flag to zero or by defining \a NDEBUG during the compilation.
*/
#if METRIC_NUMERIC_INTERNAL_ASSERTION
#define METRIC_NUMERIC_INTERNAL_ASSERT(expr, msg) assert((expr) || mtrc::numeric::ASSERT_MESSAGE(msg))
#else
#define METRIC_NUMERIC_INTERNAL_ASSERT(expr, msg)
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Run time assertion macro for user checks.
// \ingroup runtime_assert
//
// In case of an invalid run time expression, the program execution is terminated.\n
// The METRIC_NUMERIC_USER_ASSERT macro can be disabled by setting the \a METRIC_NUMERIC_USER_ASSERT flag
// to zero or by defining \a NDEBUG during the compilation.
*/
#if METRIC_NUMERIC_USER_ASSERTION
#define METRIC_NUMERIC_USER_ASSERT(expr, msg) assert((expr) || mtrc::numeric::ASSERT_MESSAGE(msg))
#else
#define METRIC_NUMERIC_USER_ASSERT(expr, msg)
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
