// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_SQUARE_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_SQUARE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsSquare.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SQUARE_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a square matrix type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SQUARE_MATRIX_TYPE(T)                                                        \
	static_assert(::mtrc::numeric::IsSquare_v<T>, "Non-square matrix type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SQUARE_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a square matrix type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SQUARE_MATRIX_TYPE(T)                                                    \
	static_assert(!::mtrc::numeric::IsSquare_v<T>, "Square matrix type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
