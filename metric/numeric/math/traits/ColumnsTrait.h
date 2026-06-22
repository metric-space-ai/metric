// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_COLUMNSTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_COLUMNSTRAIT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/Types.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename, size_t> struct ColumnsTrait;
template <typename, size_t, typename = void> struct ColumnsTraitEval1;
template <typename, size_t, typename = void> struct ColumnsTraitEval2;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <size_t N, typename T> auto evalColumnsTrait(const volatile T &) -> ColumnsTraitEval1<T, N>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the ColumnsTrait class.
// \ingroup math_traits
//
// \section columnstrait_general General
//
// The ColumnsTrait class template offers the possibility to select the resulting data type when
// creating a view on a set of columns of a dense or sparse matrix. In case the given type \a MT
// is a dense or sparse matrix type, ColumnsTrait defines the nested type \a Type, which represents
// the resulting data type of the columns operation. Otherwise there is no nested type \a Type.
// Note that \a const and \a volatile qualifiers and reference modifiers are generally ignored.
//
//
// \section columnstrait_specializations Creating custom specializations
//
// Per default, ColumnsTrait supports all matrix types of the Metric numeric library (including views and
// adaptors). For all other data types it is possible to specialize the ColumnsTrait template.
// The following example shows the according specialization for the DynamicMatrix class template:

   \code
   template< typename T1, bool SO, size_t N >
   struct ColumnsTrait< DynamicMatrix<T1,SO>, N >
   {
	  using Type = DynamicMatrix<T1,true>;
   };
   \endcode

// \n \section columnstrait_examples Examples
//
// The following example demonstrates the use of the ColumnsTrait template, where depending on
// the given matrix type the resulting columns type is selected:

   \code
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnMajor;

   // Definition of the columns type of a column-major dynamic matrix
   using MatrixType1 = mtrc::numeric::DynamicMatrix<int,columnMajor>;
   using ResultType1 = typename mtrc::numeric::ColumnsTrait<MatrixType1,0UL>::Type;

   // Definition of the columns type for two columns of a row-major static matrix
   using MatrixType2 = mtrc::numeric::StaticMatrix<int,4UL,3UL,rowMajor>;
   using ResultType2 = typename mtrc::numeric::ColumnsTrait<MatrixType2,2UL>::Type;
   \endcode
*/
template <typename MT // Type of the matrix
		  ,
		  size_t N> // Number of compile time indices
struct ColumnsTrait : public decltype(evalColumnsTrait<N>(std::declval<MT &>())) {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the ColumnsTrait type trait.
// \ingroup math_traits
//
// The ColumnsTrait_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the ColumnsTrait class template. For instance, given the matrix type \a MT the
// following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::ColumnsTrait<MT>::Type;
   using Type2 = mtrc::numeric::ColumnsTrait_t<MT>;
   \endcode
*/
template <typename MT // Type of the matrix
		  ,
		  size_t N> // Number of compile time indices
using ColumnsTrait_t = typename ColumnsTrait<MT, N>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the ColumnsTrait type trait.
// \ingroup math_traits
*/
template <typename MT // Type of the matrix
		  ,
		  size_t N // Number of compile time indices
		  ,
		  typename> // Restricting condition
struct ColumnsTraitEval1 : public ColumnsTraitEval2<MT, N> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the ColumnsTrait type trait.
// \ingroup math_traits
*/
template <typename MT // Type of the matrix
		  ,
		  size_t N // Number of compile time indices
		  ,
		  typename> // Restricting condition
struct ColumnsTraitEval2 {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
