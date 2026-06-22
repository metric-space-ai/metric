// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATSERIALEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATSERIALEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatSerialExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATSERIALEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a matrix serial evaluation expression (i.e. a type
// derived from the MatSerialExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATSERIALEXPR_TYPE(T)                                                        \
	static_assert(::mtrc::numeric::IsMatSerialExpr_v<T>, "Non-matrix serial evaluation expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATSERIALEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a matrix serial evaluation expression (i.e. a type derived
// from the MatSerialExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATSERIALEXPR_TYPE(T)                                                    \
	static_assert(!::mtrc::numeric::IsMatSerialExpr_v<T>, "Matrix serial evaluation expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
