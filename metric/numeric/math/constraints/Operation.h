// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_OPERATION_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_OPERATION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsOperation.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_OPERATION_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not an operational expression (i.e. a type derived from
// the Operation base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_OPERATION_TYPE(T)                                                            \
	static_assert(::mtrc::numeric::IsOperation_v<T>, "Non-operation type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_OPERATION_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is an operational expression (i.e. a type derived from the
// Operation base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_OPERATION_TYPE(T)                                                        \
	static_assert(!::mtrc::numeric::IsOperation_v<T>, "Operation type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
