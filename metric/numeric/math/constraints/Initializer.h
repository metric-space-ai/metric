// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_INITIALIZER_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_INITIALIZER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsInitializer.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_INITIALIZER_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not an initializer data type, i.e. an initializer vector
// or matrix, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_INITIALIZER_TYPE(T)                                                          \
	static_assert(::mtrc::numeric::IsInitialiizer_v<T>, "Non-initializer type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_INITIALIZER_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is an initializer data type, i.e. an initializer vector or
// matrix, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_INITIALIZER_TYPE(T)                                                      \
	static_assert(!::mtrc::numeric::IsInitializer_v<T>, "Initializer type dected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
