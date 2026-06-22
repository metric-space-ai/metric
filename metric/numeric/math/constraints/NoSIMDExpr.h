// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_NOSIMDEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_NOSIMDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsNoSIMDExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_NOSIMDEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a no-SIMD expression (i.e. a type derived from
// the NoSIMDExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_NOSIMDEXPR_TYPE(T)                                                           \
	static_assert(::mtrc::numeric::IsNoSIMDExpr_v<T>, "Non-no-SIMD expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_NOSIMDEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a no-SIMD expression (i.e. a type derived from the
// NoSIMDExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_NOSIMDEXPR_TYPE(T)                                                       \
	static_assert(!::mtrc::numeric::IsNoSIMDExpr_v<T>, "No-SIMD expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
