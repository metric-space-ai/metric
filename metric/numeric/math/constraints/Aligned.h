// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_ALIGNED_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_ALIGNED_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsAligned.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_ALIGNED_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T does not provide aligned data values with respect to the
// requirements of the available instruction set a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_ALIGNED_TYPE(T)                                                              \
	static_assert(::mtrc::numeric::IsAligned_v<T>, "Non-aligned type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_ALIGNED_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T does provide aligned data values with respect to the
// requirements of the available instruction set a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ALIGNED_TYPE(T)                                                          \
	static_assert(!::mtrc::numeric::IsAligned_v<T>, "Aligned type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
