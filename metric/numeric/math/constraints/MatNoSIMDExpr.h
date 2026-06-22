// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MATNOSIMDEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MATNOSIMDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatNoSIMDExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MATNOSIMDEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a matrix no-SIMD expression (i.e. a type derived
// from the MatNoSIMDExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATNOSIMDEXPR_TYPE(T)                                                        \
	static_assert(::mtrc::numeric::IsMatNoSIMDExpr_v<T>, "Non-matrix no-SIMD expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MATNOSIMDEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a matrix no-SIMD expression (i.e. a type derived from
// the MatNoSIMDExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATNOSIMDEXPR_TYPE(T)                                                    \
	static_assert(!::mtrc::numeric::IsMatNoSIMDExpr_v<T>, "Matrix no-SIMD expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
