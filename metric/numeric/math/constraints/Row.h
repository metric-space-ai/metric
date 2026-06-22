// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_ROW_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_ROW_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsRow.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_ROW_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a row type (i.e. a dense or sparse row), a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_TYPE(T)                                                                  \
	static_assert(::mtrc::numeric::IsRow_v<T>, "Non-row type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_ROW_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a row type (i.e. a dense or sparse row), a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ROW_TYPE(T)                                                              \
	static_assert(!::mtrc::numeric::IsRow_v<T>, "Row type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
