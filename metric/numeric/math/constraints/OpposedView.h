// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_OPPOSEDVIEW_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_OPPOSEDVIEW_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsOpposedView.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_OPPOSED_VIEW CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not an opposed view, i.e. is not a view that is opposed
// to the natural storage order of its underlying type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_OPPOSED_VIEW(T)                                                              \
	static_assert(::mtrc::numeric::IsOpposedView_v<T>, "Non-opposed view detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_OPPOSED_VIEW CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is an opposed view, i.e. is a view that is opposed to the
// natural storage order of its underlying type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_OPPOSED_VIEW(T)                                                          \
	static_assert(!::mtrc::numeric::IsOpposedView_v<T>, "Opposed view detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
