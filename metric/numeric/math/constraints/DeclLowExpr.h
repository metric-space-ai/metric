// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_DECLLOWEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_DECLLOWEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsDeclLowExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_DECLLOWEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a decllow expression (i.e. a type derived from the
// DeclLowExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_DECLLOWEXPR_TYPE(T)                                                          \
	static_assert(::mtrc::numeric::IsDeclLowExpr_v<T>, "Non-decllow expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_DECLLOWEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a decllow expression (i.e. a type derived from the
// DeclLowExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DECLLOWEXPR_TYPE(T)                                                      \
	static_assert(!::mtrc::numeric::IsDeclLowExpr_v<T>, "Decllow expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
