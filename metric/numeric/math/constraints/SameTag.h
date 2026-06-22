// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_SAMETAG_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_SAMETAG_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsSame.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_HAVE_SAME_TAG CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Data type constraint.
// \ingroup constraints
//
// In case the two types \a A and \a B are not the same tag (ignoring all cv-qualifiers of both
// data types), a compilation error is created. The following example illustrates the behavior
// of this constraint:

   \code
   METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TYPE( double, double );        // No compilation error
   METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TYPE( double, const double );  // No compilation error (only cv-qualifiers
differ) METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TYPE( double, float );         // Compilation error, different data
types!
   \endcode
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TAG(A, B)                                                               \
	static_assert(::mtrc::numeric::IsSame_v<A, B>, "Non-matching tags detected")
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
// In case the two types \a A and \a B are the same tag (ignoring all cv-qualifiers of both data
// types), a compilation error is created. The following example illustrates the behavior of
// this constraint:

   \code
   METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SAME_TYPE( double, float );         // No compilation error, different data
types METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SAME_TYPE( double, const double );  // Compilation error (only cv-qualifiers
differ) METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SAME_TYPE( double, double );        // Compilation error, same data type!
   \endcode
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SAME_TAG(A, B)                                                           \
	static_assert(!::mtrc::numeric::IsSame_v<A, B>, "Matching tags detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
