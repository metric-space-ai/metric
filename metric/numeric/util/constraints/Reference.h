// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_REFERENCE_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_REFERENCE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsReference.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_REFERENCE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not a reference type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_REFERENCE_TYPE(T)                                                            \
	static_assert(::mtrc::numeric::IsReference_v<T>, "Non-reference type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_REFERENCE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not a reference type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_REFERENCE_TYPE(T)                                                        \
	static_assert(!::mtrc::numeric::IsReference_v<T>, "Reference type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
