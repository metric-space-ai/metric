// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_SMPASSIGNABLE_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_SMPASSIGNABLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsSMPAssignable.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SMP_ASSIGNABLE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not SMP-assignable (i.e. cannot be assigned by multiple
// threads), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SMP_ASSIGNABLE(T)                                                            \
	static_assert(::mtrc::numeric::IsSMPAssignable_v<T>, "Non-SMP assignable type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SMP_ASSIGNABLE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is SMP-assignable (can be assigned by multiple threads), a
// compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SMP_ASSIGNABLE(T)                                                        \
	static_assert(!::mtrc::numeric::IsSMPAssignable_v<T>, "SMP assignable type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
