// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MUTABLEDATAACCESS_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MUTABLEDATAACCESS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/HasMutableDataAccess.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_HAVE_MUTABLE_DATA_ACCESS CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T does not provide low-level data access to mutable data,
// i.e. does not have a non-const 'data' member function, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(T)                                                     \
	static_assert(::mtrc::numeric::HasMutableDataAccess_v<T>, "Type without mutable data access detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_HAVE_MUTABLE_DATA_ACCESS CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T does provide low-level data access to mutable data, i.e.
// does have a non-const 'data' member function, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_HAVE_MUTABLE_DATA_ACCESS(T)                                                 \
	static_assert(!::mtrc::numeric::HasMutableDataAccess_v<T>, "Type with mutable data access detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
