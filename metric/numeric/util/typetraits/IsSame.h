// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISSAME_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISSAME_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/RemoveCV.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type relationship analysis.
// \ingroup type_traits
//
// This class tests if the two data types \a A and \a B are equal. For this type comparison,
// the cv-qualifiers of both data types are not ignored. If \a A and \a B are the same data
// type, then the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set
// to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::IsStrictlySame<int,int>::value                   // Evaluates to 'true'
   mtrc::numeric::IsStrictlySame<const double,const double>::Type  // Results in TrueType
   mtrc::numeric::IsStrictlySame<volatile float,volatile float>    // Is derived from TrueType
   mtrc::numeric::IsStrictlySame<char,wchar_t>::value              // Evaluates to 'false'
   mtrc::numeric::IsStrictlySame<int,const int>::Type              // Results in FalseType
   mtrc::numeric::IsStrictlySame<float,volatile float>             // Is derived from FalseType
   \endcode
*/
template <typename A, typename B> struct IsStrictlySame : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsStrictlySame class template for a single, matching data type.
template <typename T> struct IsStrictlySame<T, T> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsStrictlySame type trait.
// \ingroup type_traits
//
// The IsStrictlySame_v variable template provides a convenient shortcut to access the nested
// \a value of the IsStrictlySame class template. For instance, given the types \a T1 and \a T2
// the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsStrictlySame<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::IsStrictlySame_v<T1,T2>;
   \endcode
*/
template <typename A, typename B> constexpr bool IsStrictlySame_v = IsStrictlySame<A, B>::value;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Type relationship analysis.
// \ingroup type_traits
//
// This class tests if the two data types \a A and \a B are equal. For this type comparison,
// the cv-qualifiers of both data types are ignored. If \a A and \a B are the same data type
// (ignoring the cv-qualifiers), then the \a value member constant is set to \a true, the
// nested type definition \a Type is \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class derives from
// \a FalseType.

   \code
   mtrc::numeric::IsSame<int,int>::value               // Evaluates to 'true'
   mtrc::numeric::IsSame<int,const int>::Type          // Results in TrueType
   mtrc::numeric::IsSame<float,volatile float>         // Is derived from TrueType
   mtrc::numeric::IsSame<char,wchar_t>::value          // Evaluates to 'false'
   mtrc::numeric::IsSame<char,volatile float>::Type    // Results in FalseType
   mtrc::numeric::IsSame<int,double>                   // Is derived from FalseType
   \endcode
*/
template <typename A, typename B>
struct IsSame : public BoolConstant<IsStrictlySame_v<RemoveCV_t<A>, RemoveCV_t<B>>> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsSame type trait.
// \ingroup type_traits
//
// The IsSame_v variable template provides a convenient shortcut to access the nested \a value
// of the IsSame class template. For instance, given the types \a T1 and \a T2 the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsSame<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::IsSame_v<T1,T2>;
   \endcode
*/
template <typename A, typename B> constexpr bool IsSame_v = IsSame<A, B>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
