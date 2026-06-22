// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_COLUMNS_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_COLUMNS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsColumns.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_COLUMNS_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a column selection (i.e. a view on selected columns of
// a dense or sparse matrix), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMNS_TYPE(T)                                                              \
	static_assert(::mtrc::numeric::IsColumns_v<T>, "Non-columns type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_COLUMNS_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a column selection type (i.e. a view on selected columns
// of a dense or sparse matrix), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COLUMNS_TYPE(T)                                                          \
	static_assert(!::mtrc::numeric::IsColumns_v<T>, "Columns type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
