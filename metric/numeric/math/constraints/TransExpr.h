// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_TRANSEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_TRANSEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsTransExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_TRANSEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a transposition expression (i.e. a type derived from
// the TransExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_TRANSEXPR_TYPE(T)                                                            \
	static_assert(::mtrc::numeric::IsTransExpr_v<T>, "Non-transposition expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_TRANSEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a transposition expression (i.e. a type derived from the
// TransExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_TRANSEXPR_TYPE(T)                                                        \
	static_assert(!::mtrc::numeric::IsTransExpr_v<T>, "Transposition expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
