// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_ROWS_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_ROWS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsRows.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_ROWS_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a row selection (i.e. a view on selected rows of a
// dense or sparse matrix), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROWS_TYPE(T)                                                                 \
	static_assert(::mtrc::numeric::IsRows_v<T>, "Non-rows type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_ROWS_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a row selection type (i.e. a view on selected rows of a
// dense or sparse matrix), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ROWS_TYPE(T)                                                             \
	static_assert(!::mtrc::numeric::IsRows_v<T>, "Rows type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
