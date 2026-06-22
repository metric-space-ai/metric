// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_ADAPTOR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_ADAPTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsAdaptor.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_ADAPTOR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not an adaptor type (as for instance a LowerMatrix,
// UpperMatrix, or SymmetricMatrix) a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_ADAPTOR_TYPE(T)                                                              \
	static_assert(::mtrc::numeric::IsAdaptor_v<T>, "Non-adaptor type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_ADAPTOR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is an adaptor type (as for instance LowerMatrix, UpperMatrix,
// or SymmetricMatrix) a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(T)                                                          \
	static_assert(!::mtrc::numeric::IsAdaptor_v<T>, "Adaptor type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
