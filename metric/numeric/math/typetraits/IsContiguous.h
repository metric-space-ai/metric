// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISCONTIGUOUS_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISCONTIGUOUS_H
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
/*!\brief Compile time check for the memory layout of data types.
// \ingroup math_type_traits
//
// This type trait tests whether the given data type is array-like and all its elements lie
// contiguous in memory. In case the data type has contiguous elements, the \a value member
// constant is set to \a true, the nested type definition \a Type is \a TrueType, and the class
// derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and
// the class derives from \a FalseType. Examples:

   \code
   using mtrc::numeric::StaticVector;
   using mtrc::numeric::CompressedVector;
   using mtrc::numeric::DynamicMatrix;

   using RowMajorMatrix    = DynamicMatrix<double,rowMajor>;
   using ColumnMajorMatrix = DynamicMatrix<double,columnMajor>;

   using RowType1     = decltype( mtrc::numeric::row<1UL>( std::declval<RowMajorMatrix>() ) );
   using RowType2     = decltype( mtrc::numeric::row<1UL>( std::declval<ColumnMajorMatrix>() ) );
   using ElementsType = decltype( mtrc::numeric::elements<4UL,12UL>( std::declval< StaticVector<int,3UL> >() ) );

   mtrc::numeric::IsContiguous< StaticVector<int,3UL> >::value       // Evaluates to 1
   mtrc::numeric::IsContiguous< const DynamicMatrix<double> >::Type  // Results in TrueType
   mtrc::numeric::IsContiguous< volatile RowType1 >                  // Is derived from TrueType
   mtrc::numeric::IsContiguous< CompressedVector<int> >::value       // Evaluates to 0
   mtrc::numeric::IsContiguous< const RowType2 >::Type               // Results in FalseType
   mtrc::numeric::IsContiguous< volatile ElementsType >              // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsContiguous : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsContiguou type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsContiguous<const T> : public IsContiguous<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsContiguous type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsContiguous<volatile T> : public IsContiguous<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsContiguous type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsContiguous<const volatile T> : public IsContiguous<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsContiguous type trait.
// \ingroup math_type_traits
//
// The IsContiguous_v variable template provides a convenient shortcut to access the nested
// \a value of the IsContiguous class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsContiguous<T>::value;
   constexpr bool value2 = mtrc::numeric::IsContiguous_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsContiguous_v = IsContiguous<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
