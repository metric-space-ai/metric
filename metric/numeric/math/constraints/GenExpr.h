// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_GENEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_GENEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsGenExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_GENEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a generator expression (i.e. a type derived from the
// GenExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_GENEXPR_TYPE(T)                                                              \
	static_assert(::mtrc::numeric::IsGenExpr_v<T>, "Non-generator expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_GENEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a generator expression (i.e. a type derived from the
// GenExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_GENEXPR_TYPE(T)                                                          \
	static_assert(!::mtrc::numeric::IsGenExpr_v<T>, "Generator expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
