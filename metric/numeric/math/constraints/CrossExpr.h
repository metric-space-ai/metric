// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_CROSSEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_CROSSEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsCrossExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_CROSSEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a cross product expression (i.e. a type derived from
// the CrossExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_CROSSEXPR_TYPE(T)                                                            \
	static_assert(::mtrc::numeric::IsCrossExpr_v<T>, "Non-cross product expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_CROSSEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a cross product expression (i.e. a type derived from the
// CrossExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_CROSSEXPR_TYPE(T)                                                        \
	static_assert(!::mtrc::numeric::IsCrossExpr_v<T>, "Cross product expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
