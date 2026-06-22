// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_DERIVEDFROM_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_DERIVEDFROM_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsBaseOf.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_DERIVED_FROM CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the inheritance relationship of a data type.
// \ingroup constraints
//
// In case \a D is not derived from \a B, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_DERIVED_FROM(D, B)                                                           \
	static_assert((::mtrc::numeric::IsBaseOf_v<B, D>), "Broken inheritance relationship detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_DERIVED_FROM CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the inheritance relationship of a data type.
// \ingroup constraints
//
// In case \a D is derived from \a B or in case \a D is the same type as \a B, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DERIVED_FROM(D, B)                                                       \
	static_assert((!::mtrc::numeric::IsBaseOf_v<B, D>), "Unexpected inheritance relationship detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_BE_STRICTLY_DERIVED_FROM CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the inheritance relationship of a data type.
// \ingroup constraints
//
// In case \a D is not derived from \a B, a compilation error is created. In contrast to the
// METRIC_NUMERIC_CONSTRAINT_MUST_BE_DERIVED_FROM constraint, a compilation error is also created in
// case \a D and \a B are the same type.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_STRICTLY_DERIVED_FROM(D, B)                                                  \
	static_assert((::mtrc::numeric::IsBaseOf_v<B, D> && !::mtrc::numeric::IsBaseOf_v<D, B>),                       \
				  "Broken inheritance relationship detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_BE_STRICTLY_DERIVED_FROM CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the inheritance relationship of a data type.
// \ingroup constraints
//
// In case \a D is derived from \a B, a compilation error is created. In contrast to the
// METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DERIVED_FROM constraint, no compilation error is created
// in case \a D and \a B are the same type.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_STRICTLY_DERIVED_FROM(D, B)                                              \
	static_assert((!::mtrc::numeric::IsBaseOf_v<B, D> || ::mtrc::numeric::IsBaseOf_v<D, B>),                       \
				  "Unexpected inheritance relationship detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
