// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATMAPEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATMAPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatMapExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATMAPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a unary matrix map expression (i.e. a type derived
// from the MatMapExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATMAPEXPR_TYPE(T)                                                           \
	static_assert(::mtrc::numeric::IsMatMapExpr_v<T>, "Non-unary matrix map expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATMAPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a unary matrix map expression (i.e. a type derived from
// the MatMapExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATMAPEXPR_TYPE(T)                                                       \
	static_assert(!::mtrc::numeric::IsMatMapExpr_v<T>, "Unary matrix map expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
