// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_DIVEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_DIVEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsDivExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_DIVEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a division expression (i.e. a type derived from the
// DivExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_DIVEXPR_TYPE(T)                                                              \
	static_assert(::mtrc::numeric::IsDivExpr_v<T>, "Non-division expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_DIVEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a division expression (i.e. a type derived from the
// DivExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DIVEXPR_TYPE(T)                                                          \
	static_assert(!::mtrc::numeric::IsDivExpr_v<T>, "Division expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
