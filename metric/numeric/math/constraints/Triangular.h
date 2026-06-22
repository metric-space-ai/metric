// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_TRIANGULAR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_TRIANGULAR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsTriangular.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_TRIANGULAR_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a lower or upper triangular matrix type, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_TRIANGULAR_MATRIX_TYPE(T)                                                    \
	static_assert(::mtrc::numeric::IsTriangular_v<T>, "Non-triangular matrix type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_TRIANGULAR_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a lower or upper triangular matrix type, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_TRIANGULAR_MATRIX_TYPE(T)                                                \
	static_assert(!::mtrc::numeric::IsTriangular_v<T>, "Triangular matrix type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
