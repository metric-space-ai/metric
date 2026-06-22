// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_DECLSTRUPPEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_DECLSTRUPPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsDeclStrUppExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_DECLSTRUPPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a declstrupp expression (i.e. a type derived from the
// DeclStrUppExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_DECLSTRUPPEXPR_TYPE(T)                                                       \
	static_assert(::mtrc::numeric::IsDeclStrUppExpr_v<T>, "Non-declstrupp expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_DECLSTRUPPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a declstrupp expression (i.e. a type derived from the
// DeclStrUppExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DECLSTRUPPEXPR_TYPE(T)                                                   \
	static_assert(!::mtrc::numeric::IsDeclStrUppExpr_v<T>, "Declstrupp expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
