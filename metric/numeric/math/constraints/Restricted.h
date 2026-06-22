// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_RESTRICTED_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_RESTRICTED_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsRestricted.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_RESTRICTED CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T does not have a restricted data access, i.e. does not restrict
// specific operations on its data, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_RESTRICTED(T)                                                                \
	static_assert(::mtrc::numeric::IsRestricted_v<T>, "Unrestricted type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_RESTRICTED CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T does have a restricted data access, i.e. does not restrict
// any operation on its data, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_RESTRICTED(T)                                                            \
	static_assert(!::mtrc::numeric::IsRestricted_v<T>, "Restricted type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
