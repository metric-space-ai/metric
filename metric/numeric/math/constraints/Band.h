// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_BAND_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_BAND_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsBand.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_BAND_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a band type (i.e. a dense or sparse band), a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_BAND_TYPE(T)                                                                 \
	static_assert(::mtrc::numeric::IsBand_v<T>, "Non-band type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_BAND_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a band type (i.e. a dense or sparse band), a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_BAND_TYPE(T)                                                             \
	static_assert(!::mtrc::numeric::IsBand_v<T>, "Band type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
