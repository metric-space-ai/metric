// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_CONST_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_CONST_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsConst.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_CONST CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type is not a const-qualified type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_CONST(T)                                                                     \
	static_assert(::mtrc::numeric::IsConst_v<T>, "Non-const-qualified type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_CONST CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type is a const-qualified type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_CONST(T)                                                                 \
	static_assert(!::mtrc::numeric::IsConst_v<T>, "Const-qualified type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
