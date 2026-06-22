// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_VECNOALIASEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_VECNOALIASEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsVecNoAliasExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VECNOALIASEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a vector no-alias expression (i.e. a type derived
// from the VecNoAliasExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECNOALIASEXPR_TYPE(T)                                                       \
	static_assert(::mtrc::numeric::IsVecNoAliasExpr_v<T>, "Non-vector no-alias expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VECNOALIASEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a vector no-alias expression (i.e. a type derived from
// the VecNoAliasExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VECNOALIASEXPR_TYPE(T)                                                   \
	static_assert(!::mtrc::numeric::IsVecNoAliasExpr_v<T>, "Vector no-alias expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
