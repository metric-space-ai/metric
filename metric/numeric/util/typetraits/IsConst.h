// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISCONST_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISCONST_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <type_traits>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for constant data types.
// \ingroup type_traits
//
// The IsConst type trait tests whether or not the given template parameter is a (top level)
// const-qualified data type. In case the given data type is const-qualified, the \a value
// member constant is set to \a true, the nested type definition \a Type is \a TrueType, and
// the class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::IsConst<const int>::value          // Evaluates to 'true'
   mtrc::numeric::IsConst<const volatile int>::Type  // Results in TrueType
   mtrc::numeric::IsConst<int* const>                // Is derived from TrueType
   mtrc::numeric::IsConst<int>::value                // Evaluates to 'false'
   mtrc::numeric::IsConst<const int*>::Type          // Results in FalseType (the const-qualifier is not at the top
level) mtrc::numeric::IsConst<const int* volatile>       // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsConst : public BoolConstant<std::is_const<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsConst type trait.
// \ingroup type_traits
//
// The IsConst_v variable template provides a convenient shortcut to access the nested \a value
// of the IsConst class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsConst<T>::value;
   constexpr bool value2 = mtrc::numeric::IsConst_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsConst_v = IsConst<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
