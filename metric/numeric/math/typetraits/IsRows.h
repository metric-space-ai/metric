// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISROWS_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISROWS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/views/Forward.h>
#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for row selections.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a row selection (i.e. a
// view on rows of a dense or sparse matrix). In case the type is a row selection, the \a value
// member constant is set to \a true, the nested type definition \a Type is \a TrueType, and the
// class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType,
// and the class derives from \a FalseType.

   \code
   using mtrc::numeric::aligned;

   using MatrixType1 = mtrc::numeric::StaticMatrix<int,10UL,16UL>;
   using MatrixType2 = mtrc::numeric::DynamicMatrix<double>;
   using MatrixType3 = mtrc::numeric::CompressedMatrix<float>;

   MatrixType1 A;
   MatrixType2 B( 100UL, 200UL );
   MatrixType3 C( 200UL, 250UL );

   using RowsType1 = decltype( mtrc::numeric::rows<2UL,4UL>( A ) );
   using RowsType2 = decltype( mtrc::numeric::rows( B, 8UL, 24UL ) );
   using RowsType3 = decltype( mtrc::numeric::rows( C, 5UL, 13UL ) );

   mtrc::numeric::IsRows< RowsType1 >::value         // Evaluates to 1
   mtrc::numeric::IsRows< const RowsType2 >::Type    // Results in TrueType
   mtrc::numeric::IsRows< volatile RowsType3 >       // Is derived from TrueType
   mtrc::numeric::IsRows< MatrixType1 >::value       // Evaluates to 0
   mtrc::numeric::IsRows< const MatrixType2 >::Type  // Results in FalseType
   mtrc::numeric::IsRows< volatile MatrixType3 >     // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsRows : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsRows type trait for 'Rows'.
// \ingroup math_type_traits
*/
template <typename MT, bool SO, bool DF, bool SF, typename... CRAs>
struct IsRows<Rows<MT, SO, DF, SF, CRAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsRows type trait for 'const Rows'.
// \ingroup math_type_traits
*/
template <typename MT, bool SO, bool DF, bool SF, typename... CRAs>
struct IsRows<const Rows<MT, SO, DF, SF, CRAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsRows type trait for 'volatile Rows'.
// \ingroup math_type_traits
*/
template <typename MT, bool SO, bool DF, bool SF, typename... CRAs>
struct IsRows<volatile Rows<MT, SO, DF, SF, CRAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsRows type trait for 'const volatile Rows'.
// \ingroup math_type_traits
*/
template <typename MT, bool SO, bool DF, bool SF, typename... CRAs>
struct IsRows<const volatile Rows<MT, SO, DF, SF, CRAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsRows type trait.
// \ingroup math_type_traits
//
// The IsRows_v variable template provides a convenient shortcut to access the nested \a value
// of the IsRows class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsRows<T>::value;
   constexpr bool value2 = mtrc::numeric::IsRows_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsRows_v = IsRows<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
