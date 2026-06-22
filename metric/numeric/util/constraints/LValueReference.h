// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_LVALUEREFERENCE_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_LVALUEREFERENCE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsLValueReference.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_LVALUE_REFERENCE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not an lvalue reference type, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_LVALUE_REFERENCE_TYPE(T)                                                     \
	static_assert(::mtrc::numeric::IsLValueReference_v<T>, "Non-lvalue reference type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_LVALUE_REFERENCE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T is not an lvalue reference type, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_LVALUE_REFERENCE_TYPE(T)                                                 \
	static_assert(!::mtrc::numeric::IsLValueReference_v<T>, "Lvalue reference type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
