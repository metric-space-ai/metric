// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_DECLUNILOWEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_DECLUNILOWEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsDeclUniLowExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_DECLUNILOWEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a declunilow expression (i.e. a type derived from the
// DeclUniLowExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_DECLUNILOWEXPR_TYPE(T)                                                       \
	static_assert(::mtrc::numeric::IsDeclUniLowExpr_v<T>, "Non-declunilow expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_DECLUNILOWEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a declunilow expression (i.e. a type derived from the
// DeclUniLowExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DECLUNILOWEXPR_TYPE(T)                                                   \
	static_assert(!::mtrc::numeric::IsDeclUniLowExpr_v<T>, "Declunilow expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
