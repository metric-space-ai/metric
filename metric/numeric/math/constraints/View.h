// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_VIEW_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_VIEW_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsView.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_VIEW_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a view type (i.e. a subvector, submatrix, row, column,
// ...), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_VIEW_TYPE(T)                                                                 \
	static_assert(::mtrc::numeric::IsView_v<T>, "Non-view type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_VIEW_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a view type (i.e. a subvector, submatrix, row, column,
// ...), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VIEW_TYPE(T)                                                             \
	static_assert(!::mtrc::numeric::IsView_v<T>, "View type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
