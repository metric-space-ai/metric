// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_MODIFICATION_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_MODIFICATION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsModification.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_MODIFICATION_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a modification expression (i.e. a type derived from
// the Modification base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MODIFICATION_TYPE(T)                                                         \
	static_assert(::mtrc::numeric::IsModification_v<T>, "Non-modification type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MODIFICATION_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a modification expression (i.e. a type derived from the
// Modification base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MODIFICATION_TYPE(T)                                                     \
	static_assert(!::mtrc::numeric::IsModification_v<T>, "Modification type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
