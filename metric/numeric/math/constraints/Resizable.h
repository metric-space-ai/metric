// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_RESIZABLE_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_RESIZABLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsResizable.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_RESIZABLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not resizable, i.e. does not have a 'resize' member
// function that could be used to change the size, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_RESIZABLE_TYPE(T)                                                            \
	static_assert(::mtrc::numeric::IsResizable_v<T>, "Non-resizable type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_RESIZABLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is resizable, i.e. has a 'resize' member function that can
// be used to change the size, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_RESIZABLE_TYPE(T)                                                        \
	static_assert(!::mtrc::numeric::IsResizable_v<T>, "Resizable type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
