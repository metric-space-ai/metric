// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_COMPLEX_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_COMPLEX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsComplex.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_COMPLEX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// This compile time constraint checks that the given data type \a T is a complex data type
// (i.e. an instantiation of \a complex, ignoring the cv-qualifiers). In case \a T is not a
// complex data type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_COMPLEX_TYPE(T)                                                              \
	static_assert(::mtrc::numeric::IsComplex_v<T>, "Non-complex type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_COMPLEX_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// This compile time constraint checks that the given data type \a T is not a complex data type
// (i.e. an instantiation of \a complex, ignoring the cv-qualifiers). In case \a T is a complex
// data type, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPLEX_TYPE(T)                                                          \
	static_assert(!::mtrc::numeric::IsComplex_v<T>, "Complex type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
