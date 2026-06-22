// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_DECLEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_DECLEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsDeclExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_DECLEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a declaration expression (i.e. a type derived from
// the DeclExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_DECLEXPR_TYPE(T)                                                             \
	static_assert(::mtrc::numeric::IsDeclExpr_v<T>, "Non-declaration expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_DECLEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a declaration expression (i.e. a type derived from the
// DeclExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DECLEXPR_TYPE(T)                                                         \
	static_assert(!::mtrc::numeric::IsDeclExpr_v<T>, "Declaration expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
