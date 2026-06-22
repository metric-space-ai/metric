// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATSCALARMULTEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATSCALARMULTEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatScalarMultExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATSCALARMULTEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a matrix/scalar multiplication expression (i.e. a type
// derived from the MatScalarMultExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATSCALARMULTEXPR_TYPE(T)                                                    \
	static_assert(::mtrc::numeric::IsMatScalarMultExpr_v<T>,                                                         \
				  "Non-matrix/scalar multiplication expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATSCALARMULTEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a matrix/scalar multiplication expression (i.e. a type
// derived from the MatScalarMultExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATSCALARMULTEXPR_TYPE(T)                                                \
	static_assert(!::mtrc::numeric::IsMatScalarMultExpr_v<T>, "Matrix/scalar multiplication expression type "        \
																"detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
