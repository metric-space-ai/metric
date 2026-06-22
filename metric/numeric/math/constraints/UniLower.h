// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_UNILOWER_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_UNILOWER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsUniLower.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_UNILOWER_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a lower unitriangular matrix type, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_UNILOWER_MATRIX_TYPE(T)                                                      \
	static_assert(::mtrc::numeric::IsUniLower_v<T>, "Non-lower unitriangular matrix type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_UNILOWER_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a lower unitriangular matrix type, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_UNILOWER_MATRIX_TYPE(T)                                                  \
	static_assert(!::mtrc::numeric::IsUniLower_v<T>, "Lower unitriangular matrix type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
