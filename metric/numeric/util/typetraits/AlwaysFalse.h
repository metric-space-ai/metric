// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ALWAYSFALSE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ALWAYSFALSE_H
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
/*!\brief Type dependent compile time \a false.
// \ingroup type_traits
//
// This type trait represents a type dependent compile time \a false. For all possible types,
// the \a value member constant is set to \a false, the nested type definition \a Type is
// \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct AlwaysFalse : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the AlwaysFalse type trait.
// \ingroup type_traits
//
// The AlwaysFalse_v variable template provides a convenient shortcut to access the nested
// \a value of the AlwaysFalse class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::AlwaysFalse<T>::value;
   constexpr bool value2 = mtrc::numeric::AlwaysFalse_v<T>;
   \endcode
*/
template <typename T> constexpr bool AlwaysFalse_v = false;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
