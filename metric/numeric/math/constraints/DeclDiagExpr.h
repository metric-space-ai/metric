// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_DECLDIAGEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_DECLDIAGEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsDeclDiagExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_DECLDIAGEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a decldiag expression (i.e. a type derived from the
// DeclDiagExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_DECLDIAGEXPR_TYPE(T)                                                         \
	static_assert(::mtrc::numeric::IsDeclDiagExpr_v<T>, "Non-decldiag expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_DECLDIAGEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a decldiag expression (i.e. a type derived from the
// DeclDiagExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DECLDIAGEXPR_TYPE(T)                                                     \
	static_assert(!::mtrc::numeric::IsDeclDiagExpr_v<T>, "Decldiag expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
