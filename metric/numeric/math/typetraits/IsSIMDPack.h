// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSIMDPACK_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSIMDPACK_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/simd/SIMDPack.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper functions for the IsSIMDPack type trait.
// \ingroup math_type_traits
*/
template <typename U> TrueType isSIMDPack_backend(const volatile SIMDPack<U> *);

FalseType isSIMDPack_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for SIMD data types.
// \ingroup math_type_traits
//
// This type trait tests whether the given data type is a Metric numeric SIMD packed data type. The
// following types are considered valid SIMD packed types:
//
// <ul>
//    <li>Basic SIMD packed data types:</li>
//    <ul>
//       <li>SIMDint8</li>
//       <li>SIMDint16</li>
//       <li>SIMDint32</li>
//       <li>SIMDint64</li>
//       <li>SIMDfloat</li>
//       <li>SIMDdouble</li>
//       <li>SIMDcint8</li>
//       <li>SIMDcint16</li>
//       <li>SIMDcint32</li>
//       <li>SIMDcint64</li>
//       <li>SIMDcfloat</li>
//       <li>SIMDcdouble</li>
//    </ul>
//    <li>Derived SIMD packed data types:</li>
//    <ul>
//       <li>SIMDshort</li>
//       <li>SIMDushort</li>
//       <li>SIMDint</li>
//       <li>SIMDuint</li>
//       <li>SIMDlong</li>
//       <li>SIMDulong</li>
//       <li>SIMDcshort</li>
//       <li>SIMDcushort</li>
//       <li>SIMDcint</li>
//       <li>SIMDcuint</li>
//       <li>SIMDclong</li>
//       <li>SIMDculong</li>
//    </ul>
// </ul>
//
// In case the data type is a SIMD data type, the \a value member constant is set to \a true,
// the nested type definition \a Type is \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class derives from
// \a FalseType. Examples:

   \code
   mtrc::numeric::IsSIMDPack< SIMDint32 >::value          // Evaluates to 1
   mtrc::numeric::IsSIMDPack< const SIMDdouble >::Type    // Results in TrueType
   mtrc::numeric::IsSIMDPack< volatile SIMDint >          // Is derived from TrueType
   mtrc::numeric::IsSIMDPack< int >::value                // Evaluates to 0
   mtrc::numeric::IsSIMDPack< const double >::Type        // Results in FalseType
   mtrc::numeric::IsSIMDPack< volatile complex<double> >  // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsSIMDPack : public decltype(isSIMDPack_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSIMDPack type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsSIMDPack<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsSIMDPack type trait.
// \ingroup math_type_traits
//
// The IsSIMDPack_v variable template provides a convenient shortcut to access the nested
// \a value of the IsSIMDPack class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsSIMDPack<T>::value;
   constexpr bool value2 = mtrc::numeric::IsSIMDPack_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsSIMDPack_v = IsSIMDPack<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
