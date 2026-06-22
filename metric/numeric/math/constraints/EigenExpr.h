// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_EIGENEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_EIGENEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsEigenExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_EIGENEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not an eigenvalue expression (i.e. a type derived from
// the EigenExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_EIGENEXPR_TYPE(T)                                                            \
	static_assert(::mtrc::numeric::IsEigenExpr_v<T>, "Non-eigenvalue expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_EIGENEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is an eigenvalue expression (i.e. a type derived from the
// EigenExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_EIGENEXPR_TYPE(T)                                                        \
	static_assert(!::mtrc::numeric::IsEigenExpr_v<T>, "Eigenvalue expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
