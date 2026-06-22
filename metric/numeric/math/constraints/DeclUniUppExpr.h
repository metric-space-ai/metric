// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_DECLUNIUPPEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_DECLUNIUPPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsDeclUniUppExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_DECLUNIUPPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a decluniupp expression (i.e. a type derived from the
// DeclUniUppExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_DECLUNIUPPEXPR_TYPE(T)                                                       \
	static_assert(::mtrc::numeric::IsDeclUniUppExpr_v<T>, "Non-decluniupp expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_DECLUNIUPPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a decluniupp expression (i.e. a type derived from the
// DeclUniUppExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DECLUNIUPPEXPR_TYPE(T)                                                   \
	static_assert(!::mtrc::numeric::IsDeclUniUppExpr_v<T>, "Decluniupp expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
