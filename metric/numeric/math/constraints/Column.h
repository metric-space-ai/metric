// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_COLUMN_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_COLUMN_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsColumn.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_COLUMN_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a column type (i.e. a dense or sparse column), a
// compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_TYPE(T)                                                               \
	static_assert(::mtrc::numeric::IsColumn_v<T>, "Non-column type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_COLUMN_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a column type (i.e. a dense or sparse column), a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COLUMN_TYPE(T)                                                           \
	static_assert(!::mtrc::numeric::IsColumn_v<T>, "Column type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
