// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_CONTIGUOUS_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_CONTIGUOUS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsContiguous.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_CONTIGUOUS_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not an array-like data type with contiguous elements
// a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_CONTIGUOUS_TYPE(T)                                                           \
	static_assert(::mtrc::numeric::IsContiguous_v<T>, "Non-contiguous type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_CONTIGUOUS_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is an array-like data type with contiguous elements a
// compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_CONTIGUOUS_TYPE(T)                                                       \
	static_assert(!::mtrc::numeric::IsContiguous_v<T>, "Contiguous type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
