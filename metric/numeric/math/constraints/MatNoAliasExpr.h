// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATNOALIASEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATNOALIASEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatNoAliasExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATNOALIASEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a matrix no-alias expression (i.e. a type derived
// from the MatNoAliasExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATNOALIASEXPR_TYPE(T)                                                       \
	static_assert(::mtrc::numeric::IsMatNoAliasExpr_v<T>, "Non-matrix no-alias expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATNOALIASEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a matrix no-alias expression (i.e. a type derived from
// the MatNoAliasExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATNOALIASEXPR_TYPE(T)                                                   \
	static_assert(!::mtrc::numeric::IsMatNoAliasExpr_v<T>, "Matrix no-alias expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
