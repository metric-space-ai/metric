// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISCOLUMN_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISCOLUMN_H
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
/*!\brief Compile time check for columns.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a column (i.e. dense
// or sparse column). In case the type is a column, the \a value member constant is set to
// \a true, the nested type definition \a Type is \a TrueType, and the class derives from
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

   using ColumnType1 = decltype( mtrc::numeric::column<4UL>( A ) );
   using ColumnType2 = decltype( mtrc::numeric::column( B, 16UL ) );
   using ColumnType3 = decltype( mtrc::numeric::column( C, 17UL ) );

   mtrc::numeric::IsColumn< ColumnType1 >::value       // Evaluates to 1
   mtrc::numeric::IsColumn< const ColumnType2 >::Type  // Results in TrueType
   mtrc::numeric::IsColumn< volatile ColumnType3 >     // Is derived from TrueType
   mtrc::numeric::IsColumn< MatrixType1 >::value       // Evaluates to 0
   mtrc::numeric::IsColumn< const MatrixType2 >::Type  // Results in FalseType
   mtrc::numeric::IsColumn< volatile MatrixType3 >     // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsColumn : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsColumn type trait for 'Column'.
// \ingroup math_type_traits
*/
template <typename MT, bool SO, bool DF, bool SF, size_t... CCAs>
struct IsColumn<Column<MT, SO, DF, SF, CCAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsColumn type trait for 'const Column'.
// \ingroup math_type_traits
*/
template <typename MT, bool SO, bool DF, bool SF, size_t... CCAs>
struct IsColumn<const Column<MT, SO, DF, SF, CCAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsColumn type trait for 'volatile Column'.
// \ingroup math_type_traits
*/
template <typename MT, bool SO, bool DF, bool SF, size_t... CCAs>
struct IsColumn<volatile Column<MT, SO, DF, SF, CCAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsColumn type trait for 'const volatile Column'.
// \ingroup math_type_traits
*/
template <typename MT, bool SO, bool DF, bool SF, size_t... CCAs>
struct IsColumn<const volatile Column<MT, SO, DF, SF, CCAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsColumn type trait.
// \ingroup math_type_traits
//
// The IsColumn_v variable template provides a convenient shortcut to access the nested \a value
// of the IsColumn class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsColumn<T>::value;
   constexpr bool value2 = mtrc::numeric::IsColumn_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsColumn_v = IsColumn<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
