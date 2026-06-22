// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_STRICTLYTRIANGULAR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_STRICTLYTRIANGULAR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsStrictlyTriangular.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_STRICTLY_TRIANGULAR_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a strictly lower or upper triangular matrix type, a
// compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_STRICTLY_TRIANGULAR_MATRIX_TYPE(T)                                           \
	static_assert(::mtrc::numeric::IsStrictlyTriangular_v<T>, "Non-strictly triangular matrix type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_STRICTLY_TRIANGULAR_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a strictly lower or upper triangular matrix type, a
// compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_STRICTLY_TRIANGULAR_MATRIX_TYPE(T)                                       \
	static_assert(!::mtrc::numeric::IsStrictlyTriangular_v<T>, "Strictly triangular matrix type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
