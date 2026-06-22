// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ALWAYSTRUE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ALWAYSTRUE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Type dependent compile time \a true.
// \ingroup type_traits
//
// This type trait represents a type dependent compile time \a true. For all possible types,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType.
*/
template <typename T> struct AlwaysTrue : public TrueType {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the AlwaysTrue type trait.
// \ingroup type_traits
//
// The AlwaysTrue_v variable template provides a convenient shortcut to access the nested
// \a value of the AlwaysTrue class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::AlwaysTrue<T>::value;
   constexpr bool value2 = mtrc::numeric::AlwaysTrue_v<T>;
   \endcode
*/
template <typename T> constexpr bool AlwaysTrue_v = true;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
