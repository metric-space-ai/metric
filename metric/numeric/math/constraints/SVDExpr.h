// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_SVDEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_SVDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsSVDExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SVDEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a singular value expression (i.e. a type derived from
// the SVDExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SVDEXPR_TYPE(T)                                                              \
	static_assert(::mtrc::numeric::IsSVDExpr_v<T>, "Non-singular value expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SVDEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a singular value expression (i.e. a type derived from the
// SVDExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SVDEXPR_TYPE(T)                                                          \
	static_assert(!::mtrc::numeric::IsSVDExpr_v<T>, "Singular value expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
