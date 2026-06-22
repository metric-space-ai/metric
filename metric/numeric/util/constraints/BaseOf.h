// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_BASEOF_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_BASEOF_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsBaseOf.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_BASE_OF CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the inheritance relationship.
// \ingroup constraints
//
// In case \a B is not a base class of \a D, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_BASE_OF(B, D)                                                                \
	static_assert(::mtrc::numeric::IsBaseOf_v<B, D>, "Broken inheritance relationship detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_BASE_OF CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the inheritance relationship.
// \ingroup constraints
//
// In case \a B is a base class of \a D or in case \a B is the same type as \a D, a
// compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_BASE_OF(B, D)                                                            \
	static_assert(!::mtrc::numeric::IsBaseOf_v<B, D>, "Unexpected inheritance relationship detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_BE_STRICTLY_BASE_OF CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the inheritance relationship.
// \ingroup constraints
//
// In case \a B is not a base class of \a D, a compilation error is created. In contrast to
// the METRIC_NUMERIC_CONSTRAINT_MUST_BE_BASE_OF constraint, a compilation error is also created in case
// \a B and \a D are the same type.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_STRICTLY_BASE_OF(B, D)                                                       \
	static_assert(::mtrc::numeric::IsBaseOf_v<B, D> && !::mtrc::numeric::IsBaseOf_v<D, B>,                         \
				  "Broken inheritance relationship detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_STRICTLY_BASE_OF CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the inheritance relationship.
// \ingroup constraints
//
// In case \a B is a base class of \a D, a compilation error is created. In contrast to the
// METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_BASE_OF constraint, no compilation error is created in case \a B
// and \a D are the same type.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_STRICTLY_BASE_OF(B, D)                                                   \
	static_assert(!::mtrc::numeric::IsBaseOf_v<B, D> || ::mtrc::numeric::IsBaseOf_v<D, B>,                         \
				  "Unexpected inheritance relationship detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
