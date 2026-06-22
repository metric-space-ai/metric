// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASLOAD_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASLOAD_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/HasMember.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the given data type \a T provides a load()
// operation (taking the cv-qualifiers into account). In case the operation is available, the
// \a value member constant is set to \a true, the nested type definition \a Type is \a TrueType,
// and the class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType, and the class derives from \a FalseType.
*/
METRIC_NUMERIC_CREATE_HAS_DATA_OR_FUNCTION_MEMBER_TYPE_TRAIT(HasLoad, load);
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
