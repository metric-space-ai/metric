// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_SHRINKABLE_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_SHRINKABLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsShrinkable.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SHRINKABLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not shrinkable, i.e. does not have a 'shrinkToFit' member
// function that could be used to reduce the capacity, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SHRINKABLE_TYPE(T)                                                           \
	static_assert(::mtrc::numeric::IsShrinkable_v<T>, "Non-shrinkable type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SHRINKABLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is shrinkable, i.e. has a 'shrinkToFit' member function that
// can be used to reduce the capacity, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SHRINKABLE_TYPE(T)                                                       \
	static_assert(!::mtrc::numeric::IsShrinkable_v<T>, "Shrinkable type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
