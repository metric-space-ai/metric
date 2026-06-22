// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_VECNOSIMDEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_VECNOSIMDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsVecNoSIMDExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VECNOSIMDEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a vector no-SIMD expression (i.e. a type derived
// from the VecNoSIMDExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECNOSIMDEXPR_TYPE(T)                                                        \
	static_assert(::mtrc::numeric::IsVecNoSIMDExpr_v<T>, "Non-vector no-SIMD expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VECNOSIMDEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a vector no-SIMD expression (i.e. a type derived from
// the VecNoSIMDExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VECNOSIMDEXPR_TYPE(T)                                                    \
	static_assert(!::mtrc::numeric::IsVecNoSIMDExpr_v<T>, "Vector no-SIMD expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
