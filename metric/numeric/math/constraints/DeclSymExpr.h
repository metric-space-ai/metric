// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_DECLSYMEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_DECLSYMEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsDeclSymExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_DECLSYMEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a declsym expression (i.e. a type derived from the
// DeclSymExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_DECLSYMEXPR_TYPE(T)                                                          \
	static_assert(::mtrc::numeric::IsDeclSymExpr_v<T>, "Non-declsym expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_DECLSYMEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a declsym expression (i.e. a type derived from the
// DeclSymExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DECLSYMEXPR_TYPE(T)                                                      \
	static_assert(!::mtrc::numeric::IsDeclSymExpr_v<T>, "Declsym expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
