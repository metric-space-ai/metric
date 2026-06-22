// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_HASSIZE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_HASSIZE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS HASSIZE
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time size check.
// \ingroup type_traits
//
// This class offers the possibility to test the size of a type at compile time. If the type
// \a T is exactly \a S bytes large, the \a value member constant is set to \a true, the
// nested type definition \a Type is \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class derives from
// \a FalseType.

   \code
   mtrc::numeric::HasSize<int,4>::value              // Evaluates to 'true' (on most architectures)
   mtrc::numeric::HasSize<float,4>::Type             // Results in TrueType (on most architectures)
   mtrc::numeric::HasSize<const double,8>            // Is derived from TrueType (on most architectures)
   mtrc::numeric::HasSize<volatile double,2>::value  // Evaluates to 'false'
   mtrc::numeric::HasSize<const char,8>::Type        // Results in FalseType
   mtrc::numeric::HasSize<unsigned char,4>           // Is derived from FalseType
   \endcode
*/
template <typename T, size_t S> struct HasSize : public BoolConstant<sizeof(T) == S> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Partial specialization of the compile time size constraint.
// \ingroup type_traits
//
// This class ia a partial specialization of the HasSize template for the type \a void. This
// specialization assumes that an object of type \a void has a size of 0. Therefore \a value
// is set to \a true, \a Type is \a TrueType, and the class derives from \a TrueType only if the
// \a S template argument is 0. Otherwise \a value is set to \a false, \a Type is \a FalseType,
// and the class derives from \a FalseType.
*/
template <size_t S> struct HasSize<void, S> : public BoolConstant<0 == S> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Partial specialization of the compile time size constraint.
// \ingroup type_traits
//
// This class ia a partial specialization of the HasSize template for constant \a void. This
// specialization assumes that an object of type \a void has a size of 0. Therefore \a value
// is set to \a true, \a Type is \a TrueType, and the class derives from \a TrueType only if
// the \a S template argument is 0. Otherwise \a value is set to \a false, \a Type is
// \a FalseType, and the class derives from \a FalseType.
*/
template <size_t S> struct HasSize<const void, S> : public BoolConstant<0 == S> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Partial specialization of the compile time size constraint.
// \ingroup type_traits
//
// This class ia a partial specialization of the HasSize template for volatile \a void. This
// specialization assumes that an object of type \a void has a size of 0. Therefore \a value
// is set to \a true, \a Type is \a TrueType, and the class derives from \a TrueType only if
// the \a S template argument is 0. Otherwise \a value is set to \a false, \a Type is
// \a FalseType, and the class derives from \a FalseType.
*/
template <size_t S> struct HasSize<volatile void, S> : public BoolConstant<0 == S> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Partial specialization of the compile time size constraint.
// \ingroup type_traits
//
// This class ia a partial specialization of the HasSize template for constant volatile \a void.
// This specialization assumes that an object of type \a void has a size of 0. Therefore \a value
// is set to \a true, \a Type is \a TrueType, and the class derives from \a TrueType only if the
// \a S template argument is 0. Otherwise \a value is set to \a false, \a Type is \a FalseType,
// and the class derives from \a FalseType.
*/
template <size_t S> struct HasSize<const volatile void, S> : public BoolConstant<0 == S> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasSize type trait.
// \ingroup type_traits
//
// The HasSize_v variable template provides a convenient shortcut to access the nested
// \a value of the HasSize class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasSize<T,8UL>::value;
   constexpr bool value2 = mtrc::numeric::HasSize_v<T,8UL>;
   \endcode
*/
template <typename T, size_t S> constexpr bool HasSize_v = HasSize<T, S>::value;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS HAS1BYTE
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time size check.
// \ingroup type_traits
//
// This type trait offers the possibility to test whether a given type has a size of exactly
// one byte. If the type \a T has one byte, the \a value member constant is set to \a true,
// the nested type definition \a Type is \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class derives from
// \a FalseType.

   \code
   mtrc::numeric::Has1Byte<const char>::value       // Evaluates to 'true' (on most architectures)
   mtrc::numeric::Has1Byte<unsigned char>::Type     // Results in TrueType (on most architectures)
   mtrc::numeric::Has1Byte<signed char>             // Is derived from TrueType (on most architectures)
   mtrc::numeric::Has1Byte<volatile double>::value  // Evaluates to 'false'
   mtrc::numeric::Has1Byte<const float>::Type       // Results in FalseType
   mtrc::numeric::Has1Byte<unsigned short>          // Is derived from FalseType
   \endcode
*/
template <typename T> struct Has1Byte : public HasSize<T, 1UL> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the Has1Byte type trait.
// \ingroup type_traits
//
// The Has1Byte_v variable template provides a convenient shortcut to access the nested
// \a value of the Has1Byte class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::Has1Byte<T>::value;
   constexpr bool value2 = mtrc::numeric::Has1Byte_v<T>;
   \endcode
*/
template <typename T> constexpr bool Has1Byte_v = Has1Byte<T>::value;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS HAS2BYTES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time size check.
// \ingroup type_traits
//
// This type trait offers the possibility to test whether a given type has a size of exactly
// two bytes. If the type \a T has two bytes, the \a value member constant is set to \a true,
// the nested type definition \a Type is \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class derives from
// \a FalseType.

   \code
   mtrc::numeric::Has2Bytes<const short>::value      // Evaluates to 'true' (on most architectures)
   mtrc::numeric::Has2Bytes<unsigned short>::Type    // Results in TrueType (on most architectures)
   mtrc::numeric::Has2Bytes<volatile short>          // Is derived from TrueType (on most architectures)
   mtrc::numeric::Has2Bytes<volatile double>::value  // Evaluates to 'false'
   mtrc::numeric::Has2Bytes<const float>::Type       // Results in FalseType
   mtrc::numeric::Has2Bytes<unsigned int>            // Is derived from FalseType
   \endcode
*/
template <typename T> struct Has2Bytes : public HasSize<T, 2UL> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the Has2Bytes type trait.
// \ingroup type_traits
//
// The Has2Bytes_v variable template provides a convenient shortcut to access the nested
// \a value of the Has2Bytes class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::Has2Bytes<T>::value;
   constexpr bool value2 = mtrc::numeric::Has2Bytes_v<T>;
   \endcode
*/
template <typename T> constexpr bool Has2Bytes_v = Has2Bytes<T>::value;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS HAS4BYTES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time size check.
// \ingroup type_traits
//
// This type trait offers the possibility to test whether a given type has a size of exactly
// four bytes. If the type \a T has four bytes, the \a value member constant is set to \a true,
// the nested type definition \a Type is \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class derives from
// \a FalseType.

   \code
   mtrc::numeric::Has4Bytes<const int>::value        // Evaluates to 'true' (on most architectures)
   mtrc::numeric::Has4Bytes<unsigned int>::Type      // Results in TrueType (on most architectures)
   mtrc::numeric::Has4Bytes<volatile float>          // Is derived from TrueType (on most architectures)
   mtrc::numeric::Has4Bytes<volatile double>::value  // Evaluates to 'false'
   mtrc::numeric::Has4Bytes<const float>::Type       // Results in FalseType
   mtrc::numeric::Has4Bytes<short>                   // Is derived from FalseType
   \endcode
*/
template <typename T> struct Has4Bytes : public HasSize<T, 4UL> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the Has4Bytes type trait.
// \ingroup type_traits
//
// The Has4Bytes_v variable template provides a convenient shortcut to access the nested
// \a value of the Has4Bytes class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::Has4Bytes<T>::value;
   constexpr bool value2 = mtrc::numeric::Has4Bytes_v<T>;
   \endcode
*/
template <typename T> constexpr bool Has4Bytes_v = Has4Bytes<T>::value;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS HAS8BYTES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time size check.
// \ingroup type_traits
//
// This type trait offers the possibility to test whether a given type has a size of exactly
// four bytes. If the type \a T has four bytes, the \a value member constant is set to \a true,
// the nested type definition \a Type is \a TrueType, and the class derives from\a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType, and the classderives from
// \a FalseType.

   \code
   mtrc::numeric::Has8Bytes<double>::value        // Evaluates to 'true' (on most architectures)
   mtrc::numeric::Has8Bytes<const double>::Type   // Results in TrueType (on most architectures)
   mtrc::numeric::Has8Bytes<volatile double>      // Is derived from TrueType (on most architectures)
   mtrc::numeric::Has8Bytes<unsigned int>::value  // Evaluates to 'false'
   mtrc::numeric::Has8Bytes<const float>::Type    // Results in FalseType
   mtrc::numeric::Has8Bytes<volatile short>       // Is derived from FalseType
   \endcode
*/
template <typename T> struct Has8Bytes : public HasSize<T, 8UL> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the Has8Bytes type trait.
// \ingroup type_traits
//
// The Has8Bytes_v variable template provides a convenient shortcut to access the nested
// \a value of the Has8Bytes class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::Has8Bytes<T>::value;
   constexpr bool value2 = mtrc::numeric::Has8Bytes_v<T>;
   \endcode
*/
template <typename T> constexpr bool Has8Bytes_v = Has8Bytes<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
