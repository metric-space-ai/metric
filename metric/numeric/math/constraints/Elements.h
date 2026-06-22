// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_ELEMENTS_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_ELEMENTS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsElements.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_ELEMENTS_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not an element selection (i.e. a view on selected elements
// of a dense or sparse vector), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_ELEMENTS_TYPE(T)                                                             \
	static_assert(::mtrc::numeric::IsElements_v<T>, "Non-elements type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_ELEMENTS_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is an element selection (i.e. a view on selected elements of
// a dense or sparse vector), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ELEMENTS_TYPE(T)                                                         \
	static_assert(!::mtrc::numeric::IsElements_v<T>, "Elements type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
