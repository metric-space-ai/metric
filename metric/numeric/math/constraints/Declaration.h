// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_DECLARATION_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_DECLARATION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsDeclaration.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_DECLARATION_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a declaration expression (i.e. a type derived from
// the Declaration base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_DECLARATION_TYPE(T)                                                          \
	static_assert(::mtrc::numeric::IsDeclaration_v<T>, "Non-declaration type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_DECLARATION_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a declaration expression (i.e. a type derived from the
// Declaration base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DECLARATION_TYPE(T)                                                      \
	static_assert(!::mtrc::numeric::IsDeclaration_v<T>, "Declaration type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
