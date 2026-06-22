// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATSCALARDIVEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATSCALARDIVEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatScalarDivExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATSCALARDIVEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a matrix/scalar division expression (i.e. a type
// derived from the MatScalarDivExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATSCALARDIVEXPR_TYPE(T)                                                     \
	static_assert(::mtrc::numeric::IsMatScalarDivExpr_v<T>, "Non-matrix/scalar division expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATSCALARDIVEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a matrix/scalar division expression (i.e. a type derived
// from the MatScalarDivExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATSCALARDIVEXPR_TYPE(T)                                                 \
	static_assert(!::mtrc::numeric::IsMatScalarDivExpr_v<T>, "Matrix/scalar division expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
