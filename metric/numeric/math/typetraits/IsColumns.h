// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISCOLUMNS_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISCOLUMNS_H
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
/*!\brief Compile time check for column selections.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a column selection (i.e. a
// view on columns of a dense or sparse matrix). In case the type is a column selection, the \a value
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

   using ColumnsType1 = decltype( mtrc::numeric::columns<2UL,4UL>( A ) );
   using ColumnsType2 = decltype( mtrc::numeric::columns( B, 8UL, 24UL ) );
   using ColumnsType3 = decltype( mtrc::numeric::columns( C, 5UL, 13UL ) );

   mtrc::numeric::IsColumns< ColumnsType1 >::value       // Evaluates to 1
   mtrc::numeric::IsColumns< const ColumnsType2 >::Type  // Results in TrueType
   mtrc::numeric::IsColumns< volatile ColumnsType3 >     // Is derived from TrueType
   mtrc::numeric::IsColumns< MatrixType1 >::value        // Evaluates to 0
   mtrc::numeric::IsColumns< const MatrixType2 >::Type   // Results in FalseType
   mtrc::numeric::IsColumns< volatile MatrixType3 >      // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsColumns : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsColumns type trait for 'Columns'.
// \ingroup math_type_traits
*/
template <typename MT, bool SO, bool DF, bool SF, typename... CRAs>
struct IsColumns<Columns<MT, SO, DF, SF, CRAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsColumns type trait for 'const Columns'.
// \ingroup math_type_traits
*/
template <typename MT, bool SO, bool DF, bool SF, typename... CRAs>
struct IsColumns<const Columns<MT, SO, DF, SF, CRAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsColumns type trait for 'volatile Columns'.
// \ingroup math_type_traits
*/
template <typename MT, bool SO, bool DF, bool SF, typename... CRAs>
struct IsColumns<volatile Columns<MT, SO, DF, SF, CRAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsColumns type trait for 'const volatile Columns'.
// \ingroup math_type_traits
*/
template <typename MT, bool SO, bool DF, bool SF, typename... CRAs>
struct IsColumns<const volatile Columns<MT, SO, DF, SF, CRAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsColumns type trait.
// \ingroup math_type_traits
//
// The IsColumns_v variable template provides a convenient shortcut to access the nested \a value
// of the IsColumns class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsColumns<T>::value;
   constexpr bool value2 = mtrc::numeric::IsColumns_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsColumns_v = IsColumns<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
