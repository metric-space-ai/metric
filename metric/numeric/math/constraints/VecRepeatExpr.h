// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_VECREPEATEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_VECREPEATEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsVecRepeatExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VECREPEATEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a vector repeater expression (i.e. a type derived from
// the VecRepeatExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECREPEATTEXPR_TYPE(T)                                                       \
	static_assert(::mtrc::numeric::IsVecRepeatExpr_v<T>, "Non-vector repeater expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VECREPEATEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a vector repeater expression (i.e. a type derived from the
// VecRepeatExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VECREPEATEXPR_TYPE(T)                                                    \
	static_assert(!::mtrc::numeric::IsVecRepeatExpr_v<T>, "Vector repeater expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
