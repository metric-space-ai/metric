// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_VECTORIZABLE_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_VECTORIZABLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsVectorizable.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VECTORIZABLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not a vectorizable data type, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE(T)                                                         \
	static_assert(mtrc::numeric::IsVectorizable_v<T>, "Non-vectorizable type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VECTORIZABLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is a vectorizable data type, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VECTORIZABLE_TYPE(T)                                                     \
	static_assert(!mtrc::numeric::IsVectorizable_v<T>, "Vectorizable type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
