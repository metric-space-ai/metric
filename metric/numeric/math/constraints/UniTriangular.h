// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_UNITRIANGULAR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_UNITRIANGULAR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsUniTriangular.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_UNITRIANGULAR_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a lower or upper unitriangular matrix type, a
// compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_UNITRIANGULAR_MATRIX_TYPE(T)                                                 \
	static_assert(::mtrc::numeric::IsUniTriangular_v<T>, "Non-unitriangular matrix type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_UNITRIANGULAR_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a lower or upper unitriangular matrix type, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_UNITRIANGULAR_MATRIX_TYPE(T)                                             \
	static_assert(!::mtrc::numeric::IsUniTriangular_v<T>, "Unitriangular matrix type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
