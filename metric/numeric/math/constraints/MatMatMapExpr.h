// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATMATMAPEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATMATMAPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatMatMapExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATMATMAPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a binary matrix map expression (i.e. a type derived
// from the MatMatMapExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATMATMAPEXPR_TYPE(T)                                                        \
	static_assert(::mtrc::numeric::IsMatMatMapExpr_v<T>, "Non-binary matrix map expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATMATMAPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a binary matrix map expression (i.e. a type derived from
// the MatMatMapExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATMATMAPEXPR_TYPE(T)                                                    \
	static_assert(!::mtrc::numeric::IsMatMatMapExpr_v<T>, "Binary matrix map expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
