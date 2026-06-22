// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_SUBVECTOR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_SUBVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsSubvector.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SUBVECTOR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a subvector type (i.e. a dense or sparse subvector),
// a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SUBVECTOR_TYPE(T)                                                            \
	static_assert(::mtrc::numeric::IsSubvector_v<T>, "Non-subvector type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SUBVECTOR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a subvector type (i.e. a dense or sparse subvector), a
// compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SUBVECTOR_TYPE(T)                                                        \
	static_assert(!::mtrc::numeric::IsSubvector_v<T>, "Subvector type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
