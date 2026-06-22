// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_SOLVEEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_SOLVEEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsSolveExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SOLVEEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a solver expression (i.e. a type derived from the
// SolveExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SOLVEEXPR_TYPE(T)                                                            \
	static_assert(::mtrc::numeric::IsSolveExpr_v<T>, "Non-solver expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SOLVEEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a solver expression (i.e. a type derived from the
// SolveExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SOLVEEXPR_TYPE(T)                                                        \
	static_assert(!::mtrc::numeric::IsSolveExpr_v<T>, "Solver expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
