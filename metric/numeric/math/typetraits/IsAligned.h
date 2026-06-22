// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISALIGNED_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISALIGNED_H
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
/*!\brief Compile time check for the alignment of data types.
// \ingroup math_type_traits
//
// This type trait tests whether the given data type guarantees to provide aligned data values
// with respect to the requirements of the available instruction set. For instance, vectorizable
// data types such as built-in and complex data types are required to be 16-bit aligned for SSE,
// 32-bit aligned for AVX, and 64-bit aligned for MIC. In case the data type is properly aligned,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType. Examples:

   \code
   using mtrc::numeric::StaticVector;
   using mtrc::numeric::CustomVector;
   using mtrc::numeric::CompressedVector;
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::CustomMatrix;
   using mtrc::numeric::aligned;
   using mtrc::numeric::unaligned;
   using mtrc::numeric::padded;
   using mtrc::numeric::unpadded;

   mtrc::numeric::IsAligned< StaticVector<int,3UL> >::value                         // Evaluates to 1
   mtrc::numeric::IsAligned< const DynamicMatrix<double> >::Type                    // Results in TrueType
   mtrc::numeric::IsAligned< volatile CustomVector<float,aligned,unpadded> >        // Is derived from TrueType
   mtrc::numeric::IsAligned< CompressedVector<int> >::value                         // Evaluates to 0
   mtrc::numeric::IsAligned< const CustomVector<double,unaligned,unpadded> >::Type  // Results in FalseType
   mtrc::numeric::IsAligned< volatile CustomMatrix<float,unaligned,padded> >        // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsAligned : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsAligned type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsAligned<const T> : public IsAligned<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsAligned type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsAligned<volatile T> : public IsAligned<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsAligned type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsAligned<const volatile T> : public IsAligned<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsAligned type trait.
// \ingroup math_type_traits
//
// The IsAligned_v variable template provides a convenient shortcut to access the nested
// \a value of the IsAligned class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsAligned<T>::value;
   constexpr bool value2 = mtrc::numeric::IsAligned_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsAligned_v = IsAligned<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
