// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_COMPUTATION_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_COMPUTATION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsComputation.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_COMPUTATION_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a computational expression (i.e. a type derived from
// the Computation base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_COMPUTATION_TYPE(T)                                                          \
	static_assert(::mtrc::numeric::IsComputation_v<T>, "Non-computation type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_COMPUTATION_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a computational expression (i.e. a type derived from the
// Computation base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(T)                                                      \
	static_assert(!::mtrc::numeric::IsComputation_v<T>, "Computation type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
