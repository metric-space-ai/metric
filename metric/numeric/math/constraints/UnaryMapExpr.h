// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_UNARYMAPEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_UNARYMAPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsUnaryMapExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_UNARYMAPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a unary map expression (i.e. a type derived from the
// UnaryMapExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_UNARYMAPEXPR_TYPE(T)                                                         \
	static_assert(::mtrc::numeric::IsUnaryMapExpr_v<T>, "Non-unary map expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_UNARYMAPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a unary map expression (i.e. a type derived from the
// UnaryMapExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_UNARYMAPEXPR_TYPE(T)                                                     \
	static_assert(!::mtrc::numeric::IsUnaryMapExpr_v<T>, "Unary map expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
