// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_NOALIASEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_NOALIASEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsNoAliasExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_NOALIASEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a no-alias expression (i.e. a type derived from
// the NoAliasExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_NOALIASEXPR_TYPE(T)                                                          \
	static_assert(::mtrc::numeric::IsNoAliasExpr_v<T>, "Non-no-alias expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_NOALIASEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a no-alias expression (i.e. a type derived from the
// NoAliasExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_NOALIASEXPR_TYPE(T)                                                      \
	static_assert(!::mtrc::numeric::IsNoAliasExpr_v<T>, "No-alias expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
