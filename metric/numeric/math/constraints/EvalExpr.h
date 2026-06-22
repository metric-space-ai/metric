// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_EVALEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_EVALEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsEvalExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_EVALEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not an evaluation expression (i.e. a type derived from
// the EvalExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_EVALEXPR_TYPE(T)                                                             \
	static_assert(::mtrc::numeric::IsEvalExpr_v<T>, "Non-evaluation expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_EVALEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is an evaluation expression (i.e. a type derived from the
// EvalExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_EVALEXPR_TYPE(T)                                                         \
	static_assert(!::mtrc::numeric::IsEvalExpr_v<T>, "Evaluation expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
