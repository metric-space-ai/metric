// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_CUSTOM_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_CUSTOM_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsCustom.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_CUSTOM_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a custom data type, i.e. a custom vector or matrix,
// a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_CUSTOM_TYPE(T)                                                               \
	static_assert(::mtrc::numeric::IsCustom_v<T>, "Non-custom type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_CUSTOM_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a custom data type, i.e. a custom vector or matrix,
// a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_CUSTOM_TYPE(T)                                                           \
	static_assert(!::mtrc::numeric::IsCustom_v<T>, "Custom type dected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
