// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_RVALUEREFERENCE_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_RVALUEREFERENCE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsRValueReference.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_RVALUE_REFERENCE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not an rvalue reference type, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_RVALUE_REFERENCE_TYPE(T)                                                     \
	static_assert(::mtrc::numeric::IsRValueReference_v<T>, "Non-rvalue reference type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_RVALUE_REFERENCE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not an rvalue reference type, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_RVALUE_REFERENCE_TYPE(T)                                                 \
	static_assert(!::mtrc::numeric::IsRValueReference_v<T>, "Rvalue reference type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
