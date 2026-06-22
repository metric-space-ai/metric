// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MULTEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MULTEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMultExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MULTEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a multiplication expression (i.e. a type derived from
// the MultExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MULTEXPR_TYPE(T)                                                             \
	static_assert(::mtrc::numeric::IsMultExpr_v<T>, "Non-multiplication expression detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MULTEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a multiplication expression (i.e. a type derived from the
// MultExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MULTEXPR_TYPE(T)                                                         \
	static_assert(!::mtrc::numeric::IsMultExpr_v<T>, "Multiplication expression detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
