// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_PADDED_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_PADDED_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsPadded.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_PADDED_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T does not employ or simulate padding (i.e. a special treatment
// of remainder elements is necessary) a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_PADDED_TYPE(T)                                                               \
	static_assert(::mtrc::numeric::IsPadded_v<T>, "Non-padded type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_PADDED_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T does employ or simulate padding (i.e. no special treatment
// of remainder elements is necessary) a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_PADDED_TYPE(T)                                                           \
	static_assert(!::mtrc::numeric::IsPadded_v<T>, "Padded type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
