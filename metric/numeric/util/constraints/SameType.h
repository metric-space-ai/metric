// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_SAMETYPE_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_SAMETYPE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsSame.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SAME_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Data type constraint.
// \ingroup constraints
//
// In case the two types \a A and \a B are not the same (ignoring all cv-qualifiers of both data
// types), a compilation error is created. The following example illustrates the behavior of this
// constraint:

   \code
   METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TYPE( double, double );        // No compilation error
   METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TYPE( double, const double );  // No compilation error (only cv-qualifiers
differ) METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TYPE( double, float );         // Compilation error, different data
types!
   \endcode

// In case the cv-qualifiers should not be ignored (e.g. 'double' and 'const double' should be
// considered to be unequal), use the mtrc::numeric::METRIC_NUMERIC_CONSTRAINT_MUST_BE_STRICTLY_SAME_TYPE constraint.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TYPE(A, B)                                                              \
	static_assert(::mtrc::numeric::IsSame_v<A, B>, "Non-matching types detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SAME_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Data type constraint.
// \ingroup constraints
//
// In case the two types \a A and \a B are the same (ignoring all cv-qualifiers of both data
// types), a compilation error is created. The following example illustrates the behavior of
// this constraint:

   \code
   METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SAME_TYPE( double, float );         // No compilation error, different data
types METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SAME_TYPE( double, const double );  // Compilation error (only cv-qualifiers
differ) METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SAME_TYPE( double, double );        // Compilation error, same data type!
   \endcode

// In case the cv-qualifiers should not be ignored (e.g. 'double' and 'const double' should
// be considered to be unequal), use the mtrc::numeric::METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_STRICTLY_SAME_TYPE
// constraint.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SAME_TYPE(A, B)                                                          \
	static_assert(!::mtrc::numeric::IsSame_v<A, B>, "Matching types detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_BE_STRICTLY_SAME_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Data type constraint.
// \ingroup constraints
//
// In case the two types \a A and \a B are not the same, a compilation error is created. Note
// that this constraint even considers two types as unequal if the cv-qualifiers differ, e.g.

   \code
   METRIC_NUMERIC_CONSTRAINT_MUST_BE_STRICTLY_SAME_TYPE( double, double );        // No compilation error
   METRIC_NUMERIC_CONSTRAINT_MUST_BE_STRICTLY_SAME_TYPE( double, const double );  // Compilation error, different
cv-qualifiers! METRIC_NUMERIC_CONSTRAINT_MUST_BE_STRICTLY_SAME_TYPE( double, float );         // Compilation error,
different data types!
   \endcode

// In case the cv-qualifiers should be ignored (e.g. 'double' and 'const double' should be
// considered to be equal), use the mtrc::numeric::METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TYPE constraint.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_STRICTLY_SAME_TYPE(A, B)                                                     \
	static_assert(::mtrc::numeric::IsStrictlySame_v<A, B>, "Non-matching types detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_STRICTLY_SAME_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Data type constraint.
// \ingroup constraints
//
// In case the two types \a A and \a B are the same, a compilation error is created. Note that
// this constraint even considers two types as unequal if the cv-qualifiers differ, e.g.

   \code
   METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_STRICTLY_SAME_TYPE( double, float );         // No compilation error, different
data types METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_STRICTLY_SAME_TYPE( double, const double );  // No compilation error,
different cv-qualifiers! METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_STRICTLY_SAME_TYPE( double, double );        //
Compilation error, same data type!
   \endcode

// In case the cv-qualifiers should be ignored (e.g. 'double' and 'const double' should be
// considered to be equal), use the mtrc::numeric::METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SAME_TYPE constraint.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_STRICTLY_SAME_TYPE(A, B)                                                 \
	static_assert(!::mtrc::numeric::IsStrictlySame_v<A, B>, "Matching types detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
