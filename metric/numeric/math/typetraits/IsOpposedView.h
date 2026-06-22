// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISOPPOSEDVIEW_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISOPPOSEDVIEW_H
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
/*!\brief Compile time check for resizable data types.
// \ingroup math_type_traits
//
// This type trait tests whether the given data type is an opposed view, i.e. a view that is
// opposed to the natural storage order of its underlying type. In case the data type is an
// opposed view, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set
// to \a false, \a Type is \a FalseType, and the class derives from \a FalseType. Examples:

   \code
   using mtrc::numeric::Column;
   using mtrc::numeric::Row;

   using DenseColumnMajor = mtrc::numeric::DynamicMatrix<int,columnMajor>;
   using SparseRowMajor   = mtrc::numeric::CompressedMatrix<int,rowMajor>;

   mtrc::numeric::IsOpposedView< Row<DenseColumnMajor> >::value          // Evaluates to 1
   mtrc::numeric::IsOpposedView< Column<SparseRowMajor> >::Type          // Results in TrueType
   mtrc::numeric::IsOpposedView< const volatile Row<DenseColumnMajor> >  // Is derived from TrueType
   mtrc::numeric::IsOpposedView< Column<DenseColumnMajor> >::value       // Evaluates to 0
   mtrc::numeric::IsOpposedView< Row<SparseRowMajor> >::Type             // Results in FalseType
   mtrc::numeric::IsOpposedView< const volatile Row<SparseRowMajor> >    // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsOpposedView : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsOpposedView type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsOpposedView<const T> : public IsOpposedView<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsOpposedView type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsOpposedView<volatile T> : public IsOpposedView<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsOpposedView type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsOpposedView<const volatile T> : public IsOpposedView<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsOpposedView type trait.
// \ingroup math_type_traits
//
// The IsOpposedView_v variable template provides a convenient shortcut to access the nested
// \a value of the IsOpposedView class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsOpposedView<T>::value;
   constexpr bool value2 = mtrc::numeric::IsOpposedView_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsOpposedView_v = IsOpposedView<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
