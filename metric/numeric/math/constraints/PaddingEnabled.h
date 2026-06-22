// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_PADDINGENABLED_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_PADDINGENABLED_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsPaddingEnabled.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_PADDING_ENABLED CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T does not provide support for padding (i.e. does not properly
// treat padding elements), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_PADDING_ENABLED(T)                                                           \
	static_assert(::mtrc::numeric::IsPaddingEnabled_v<T>, "Non-padding enabled type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_PADDING_ENABLED CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T does provide support for padding (i.e. properly treats padding
// elements), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_PADDING_ENABLED(T)                                                       \
	static_assert(!::mtrc::numeric::IsPaddingEnabled_v<T>, "Padding enabled type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
