// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISROW_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISROW_H
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
/*!\brief Compile time check for rows.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a row (i.e. a view on a
// row of a dense or sparse matrix). In case the type is a row, the \a value member constant is
// set to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.

   \code
   using mtrc::numeric::aligned;

   using MatrixType1 = mtrc::numeric::StaticMatrix<int,10UL,16UL>;
   using MatrixType2 = mtrc::numeric::DynamicMatrix<double>;
   using MatrixType3 = mtrc::numeric::CompressedMatrix<float>;

   MatrixType1 A;
   MatrixType2 B( 100UL, 200UL );
   MatrixType3 C( 200UL, 250UL );

   using RowType1 = decltype( mtrc::numeric::row<4UL>( A ) );
   using RowType2 = decltype( mtrc::numeric::row( B, 16UL ) );
   using RowType3 = decltype( mtrc::numeric::row( C, 17UL ) );

   mtrc::numeric::IsRow< RowType1 >::value          // Evaluates to 1
   mtrc::numeric::IsRow< const RowType2 >::Type     // Results in TrueType
   mtrc::numeric::IsRow< volatile RowType3 >        // Is derived from TrueType
   mtrc::numeric::IsRow< MatrixType1 >::value       // Evaluates to 0
   mtrc::numeric::IsRow< const MatrixType2 >::Type  // Results in FalseType
   mtrc::numeric::IsRow< volatile MatrixType3 >     // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsRow : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsRow type trait for 'Row'.
// \ingroup math_type_traits
*/
template <typename MT, bool SO, bool DF, bool SF, size_t... CRAs>
struct IsRow<Row<MT, SO, DF, SF, CRAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsRow type trait for 'const Row'.
// \ingroup math_type_traits
*/
template <typename MT, bool SO, bool DF, bool SF, size_t... CRAs>
struct IsRow<const Row<MT, SO, DF, SF, CRAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsRow type trait for 'volatile Row'.
// \ingroup math_type_traits
*/
template <typename MT, bool SO, bool DF, bool SF, size_t... CRAs>
struct IsRow<volatile Row<MT, SO, DF, SF, CRAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsRow type trait for 'const volatile Row'.
// \ingroup math_type_traits
*/
template <typename MT, bool SO, bool DF, bool SF, size_t... CRAs>
struct IsRow<const volatile Row<MT, SO, DF, SF, CRAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsRow type trait.
// \ingroup math_type_traits
//
// The IsRow_v variable template provides a convenient shortcut to access the nested \a value
// of the IsRow class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsRow<T>::value;
   constexpr bool value2 = mtrc::numeric::IsRow_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsRow_v = IsRow<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
