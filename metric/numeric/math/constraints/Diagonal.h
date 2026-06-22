// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_DIAGONAL_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_DIAGONAL_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsDiagonal.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_DIAGONAL_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a diagonal matrix type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_DIAGONAL_MATRIX_TYPE(T)                                                      \
	static_assert(::mtrc::numeric::IsDiagonal_v<T>, "Non-diagonal matrix type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_DIAGONAL_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a diagonal matrix type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DIAGONAL_MATRIX_TYPE(T)                                                  \
	static_assert(!::mtrc::numeric::IsDiagonal_v<T>, "Diagonal matrix type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
