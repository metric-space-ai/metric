// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_DECLUPPEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_DECLUPPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsDeclUppExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_DECLUPPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a declupp expression (i.e. a type derived from the
// DeclUppExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_DECLUPPEXPR_TYPE(T)                                                          \
	static_assert(::mtrc::numeric::IsDeclUppExpr_v<T>, "Non-declupp expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_DECLUPPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a declupp expression (i.e. a type derived from the
// DeclUppExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DECLUPPEXPR_TYPE(T)                                                      \
	static_assert(!::mtrc::numeric::IsDeclUppExpr_v<T>, "Declupp expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
