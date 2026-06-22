// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_TYPERESTRICTION_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_TYPERESTRICTION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/TypeList.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  SOFT_TYPE_RESTRICTION CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given type list \a TYPELIST does not contain a type related to \a T, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_SOFT_TYPE_RESTRICTION(T, TYPELIST)                                                   \
	static_assert(::mtrc::numeric::ContainsRelated<TYPELIST, T>::value, "Unapproved type detected")
//*************************************************************************************************

//=================================================================================================
//
//  TYPE_RESTRICTION CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given type list \a TYPELIST does not contain the data type \a T, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_TYPE_RESTRICTION(T, TYPELIST)                                                        \
	static_assert(::mtrc::numeric::Contains<TYPELIST, T>::value, "Unapproved type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
