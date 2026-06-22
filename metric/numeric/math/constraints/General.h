// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_GENERAL_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_GENERAL_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsGeneral.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_GENERAL_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a general matrix type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_GENERAL_MATRIX_TYPE(T)                                                       \
	static_assert(::mtrc::numeric::IsGeneral_v<T>, "Non-general matrix type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_GENERAL_MATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a general matrix type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_GENERAL_MATRIX_TYPE(T)                                                   \
	static_assert(!::mtrc::numeric::IsGeneral_v<T>, "General matrix type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
