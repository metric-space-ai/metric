// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATEVALEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATEVALEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatEvalExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATEVALEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a matrix evaluation expression (i.e. a type derived
// from the MatEvalExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATEVALEXPR_TYPE(T)                                                          \
	static_assert(::mtrc::numeric::IsMatEvalExpr_v<T>, "Non-matrix evaluation expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATEVALEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a matrix evaluation expression (i.e. a type derived from
// the MatEvalExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATEVALEXPR_TYPE(T)                                                      \
	static_assert(!::mtrc::numeric::IsMatEvalExpr_v<T>, "Matrix evaluation expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
