// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_RESULTTYPE_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_RESULTTYPE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/HasResultType.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_HAVE_RESULT_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T does not provide a public nested type alias named
// \a ResultType a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_RESULT_TYPE(T)                                                             \
	static_assert(::mtrc::numeric::HasResultType_v<T>, "Type without result type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_HAVE_RESULT_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T does provide a public nested type alias named \a ResultType
// a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_HAVE_RESULT_TYPE(T)                                                         \
	static_assert(!::mtrc::numeric::HasResultType_v<T>, "Type with result type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
