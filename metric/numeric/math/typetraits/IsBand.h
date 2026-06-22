// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISBAND_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISBAND_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/views/Forward.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for bands.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a band (i.e. dense or
// sparse band). In case the type is a band, the \a value member constant is set to \a true,
// the nested type definition \a Type is \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class derives from
// \a FalseType.

   \code
   using mtrc::numeric::aligned;

   using MatrixType1 = mtrc::numeric::StaticMatrix<int,10UL,16UL>;
   using MatrixType2 = mtrc::numeric::DynamicMatrix<double>;
   using MatrixType3 = mtrc::numeric::CompressedMatrix<float>;

   MatrixType1 A;
   MatrixType2 B( 100UL, 200UL );
   MatrixType3 C( 200UL, 250UL );

   using BandType1 = decltype( mtrc::numeric::band<0L>( A ) );
   using BandType2 = decltype( mtrc::numeric::band( B, 2L ) );
   using BandType3 = decltype( mtrc::numeric::band( C, -4L ) );

   mtrc::numeric::IsBand< BandType1 >::value         // Evaluates to 1
   mtrc::numeric::IsBand< const BandType2 >::Type    // Results in TrueType
   mtrc::numeric::IsBand< volatile BandType3 >       // Is derived from TrueType
   mtrc::numeric::IsBand< MatrixType1 >::value       // Evaluates to 0
   mtrc::numeric::IsBand< const MatrixType2 >::Type  // Results in FalseType
   mtrc::numeric::IsBand< volatile MatrixType3 >     // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsBand : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsBand type trait for 'Band'.
// \ingroup math_type_traits
*/
template <typename MT, bool TF, bool DF, bool MF, ptrdiff_t... CBAs>
struct IsBand<Band<MT, TF, DF, MF, CBAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsBand type trait for 'const Band'.
// \ingroup math_type_traits
*/
template <typename MT, bool TF, bool DF, bool MF, ptrdiff_t... CBAs>
struct IsBand<const Band<MT, TF, DF, MF, CBAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsBand type trait for 'volatile Band'.
// \ingroup math_type_traits
*/
template <typename MT, bool TF, bool DF, bool MF, ptrdiff_t... CBAs>
struct IsBand<volatile Band<MT, TF, DF, MF, CBAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsBand type trait for 'const volatile Band'.
// \ingroup math_type_traits
*/
template <typename MT, bool TF, bool DF, bool MF, ptrdiff_t... CBAs>
struct IsBand<const volatile Band<MT, TF, DF, MF, CBAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsBand type trait.
// \ingroup math_type_traits
//
// The IsBand_v variable template provides a convenient shortcut to access the nested \a value
// of the IsBand class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsBand<T>::value;
   constexpr bool value2 = mtrc::numeric::IsBand_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsBand_v = IsBand<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
