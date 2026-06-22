// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_DECLHERMEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_DECLHERMEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsDeclHermExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_DECLHERMEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a declherm expression (i.e. a type derived from the
// DeclHermExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_DECLHERMEXPR_TYPE(T)                                                         \
	static_assert(::mtrc::numeric::IsDeclHermExpr_v<T>, "Non-declherm expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_DECLHERMEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a declherm expression (i.e. a type derived from the
// DeclHermExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DECLHERMEXPR_TYPE(T)                                                     \
	static_assert(!::mtrc::numeric::IsDeclHermExpr_v<T>, "Declherm expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
