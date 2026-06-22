// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_UNION_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_UNION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsUnion.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_UNION CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type is not a union type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_UNION(T)                                                                     \
	static_assert(::mtrc::numeric::IsUnion_v<T>, "Non-union type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_UNION CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type is a union type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_UNION(T)                                                                 \
	static_assert(!::mtrc::numeric::IsUnion_v<T>, "Union type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
