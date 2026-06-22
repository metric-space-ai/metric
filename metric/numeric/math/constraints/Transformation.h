// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_TRANSFORMATION_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_TRANSFORMATION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsTransformation.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_TRANSFORMATION_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a transformation expression (i.e. a type derived from
// the Transformation base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_TRANSFORMATION_TYPE(T)                                                       \
	static_assert(::mtrc::numeric::IsTransformation_v<T>, "Non-transformation type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_TRANSFORMATION_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a transformation expression (i.e. a type derived from the
// Transformation base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_TRANSFORMATION_TYPE(T)                                                   \
	static_assert(!::mtrc::numeric::IsTransformation_v<T>, "Transformation type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
