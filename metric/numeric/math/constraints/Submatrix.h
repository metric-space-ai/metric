// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_SUBMATRIX_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_SUBMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsSubmatrix.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_SUBMATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a submatrix type (i.e. a dense or sparse submatrix),
// a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_SUBMATRIX_TYPE(T)                                                            \
	static_assert(::mtrc::numeric::IsSubmatrix_v<T>, "Non-submatrix type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_SUBMATRIX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a submatrix type (i.e. a dense or sparse submatrix), a
// compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SUBMATRIX_TYPE(T)                                                        \
	static_assert(!::mtrc::numeric::IsSubmatrix_v<T>, "Submatrix type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
