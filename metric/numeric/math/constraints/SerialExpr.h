// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_SERIALEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_SERIALEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsSerialExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SERIALEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a serial evaluation expression (i.e. a type derived
// from the SerialExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SERIALEXPR_TYPE(T)                                                           \
	static_assert(::mtrc::numeric::IsSerialExpr_v<T>, "Non-serial evaluation expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SERIALEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a serial evaluation expression (i.e. a type derived from
// the SerialExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SERIALEXPR_TYPE(T)                                                       \
	static_assert(!::mtrc::numeric::IsSerialExpr_v<T>, "Serial evaluation expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
