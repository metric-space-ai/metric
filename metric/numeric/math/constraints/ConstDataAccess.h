// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_CONSTDATAACCESS_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_CONSTDATAACCESS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/HasConstDataAccess.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_HAVE_CONST_DATA_ACCESS CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T does not provide low-level data access to constant data,
// i.e. does not have a const 'data' member function, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_CONST_DATA_ACCESS(T)                                                       \
	static_assert(::mtrc::numeric::HasConstDataAccess_v<T>, "Type without const data access detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_HAVE_CONST_DATA_ACCESS CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T does provide low-level data access to constant data, i.e.
// does have a const 'data' member function, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_HAVE_CONST_DATA_ACCESS(T)                                                   \
	static_assert(!::mtrc::numeric::HasConstDataAccess_v<T>, "Type with const data access detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
