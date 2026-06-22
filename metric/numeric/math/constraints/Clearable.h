// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_CLEARABLE_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_CLEARABLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsClearable.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_CLEARABLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not clearable, i.e. does not have a \c clear() member
// function, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_CLEARABLE_TYPE(T)                                                            \
	static_assert(::mtrc::numeric::IsClearable_v<T>, "Non-clearable type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_CLEARABLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is clearable, i.e. has a \c clear() member function, a
// compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_CLEARABLE_TYPE(T)                                                        \
	static_assert(!::mtrc::numeric::IsClearable_v<T>, "Clearable type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
