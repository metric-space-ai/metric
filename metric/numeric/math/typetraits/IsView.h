// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISVIEW_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISVIEW_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/View.h>
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
/*!\brief Auxiliary helper functions for the IsView type trait.
// \ingroup math_type_traits
*/
template <typename U> TrueType isView_backend(const volatile View<U> *);

FalseType isView_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for views.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a view (i.e. subvector,
// submatrix, row, column, ...). In case the type is a view, the \a value member constant is
// set to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.

   \code
   using mtrc::numeric::aligned;

   using VectorType = mtrc::numeric::StaticVector<int,10UL>;
   using MatrixType = mtrc::numeric::DynamicMatrix<double>;

   VectorType a;
   MatrixType A( 100UL, 200UL );

   using SubvectorType = decltype( mtrc::numeric::subvector( a, 2UL, 8UL ) );
   using RowType       = decltype( mtrc::numeric::row( A, 8UL ) );
   using ColumnType    = decltype( mtrc::numeric::column( A, 5UL ) );

   mtrc::numeric::IsView< SubvectorType >::value    // Evaluates to 1
   mtrc::numeric::IsView< const RowType >::Type     // Results in TrueType
   mtrc::numeric::IsView< volatile ColumnType >     // Is derived from TrueType
   mtrc::numeric::IsView< float >::value            // Evaluates to 0
   mtrc::numeric::IsView< const VectorType >::Type  // Results in FalseType
   mtrc::numeric::IsView< volatile MatrixType >     // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsView : public decltype(isView_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsView type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsView<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsView type trait.
// \ingroup math_type_traits
//
// The IsView_v variable template provides a convenient shortcut to access the nested \a value
// of the IsView class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsView<T>::value;
   constexpr bool value2 = mtrc::numeric::IsView_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsView_v = IsView<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
