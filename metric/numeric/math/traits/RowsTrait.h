// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_ROWSTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_ROWSTRAIT_H
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
template <typename, size_t> struct RowsTrait;
template <typename, size_t, typename = void> struct RowsTraitEval1;
template <typename, size_t, typename = void> struct RowsTraitEval2;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <size_t M, typename T> auto evalRowsTrait(const volatile T &) -> RowsTraitEval1<T, M>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the RowsTrait class.
// \ingroup math_traits
//
// \section rowstrait_general General
//
// The RowsTrait class template offers the possibility to select the resulting data type when
// creating a view on a set of rows of a dense or sparse matrix. In case the given type \a MT
// is a dense or sparse matrix type, RowsTrait defines the nested type \a Type, which represents
// the resulting data type of the rows operation. Otherwise there is no nested type \a Type.
// Note that \a const and \a volatile qualifiers and reference modifiers are generally ignored.
//
//
// \section rowstrait_specializations Creating custom specializations
//
// Per default, RowsTrait supports all matrix types of the Metric numeric library (including views and
// adaptors). For all other data types it is possible to specialize the RowsTrait template. The
// following example shows the according specialization for the DynamicMatrix class template:

   \code
   template< typename T1, bool SO, size_t M >
   struct RowsTrait< DynamicMatrix<T1,SO>, M >
   {
	  using Type = DynamicMatrix<T1,false>;
   };
   \endcode

// \n \section rowstrait_examples Examples
//
// The following example demonstrates the use of the RowsTrait template, where depending on
// the given matrix type the resulting rows type is selected:

   \code
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnMajor;

   // Definition of the rows type of a row-major dynamic matrix
   using MatrixType1 = mtrc::numeric::DynamicMatrix<int,rowMajor>;
   using ResultType1 = typename mtrc::numeric::RowsTrait<MatrixType1,0UL>::Type;

   // Definition of the rows type for two rows of a column-major static matrix
   using MatrixType2 = mtrc::numeric::StaticMatrix<int,4UL,3UL,columnMajor>;
   using ResultType2 = typename mtrc::numeric::RowsTrait<MatrixType2,2UL>::Type;
   \endcode
*/
template <typename MT // Type of the matrix
		  ,
		  size_t M> // Number of compile time indices
struct RowsTrait : public decltype(evalRowsTrait<M>(std::declval<MT &>())) {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the RowsTrait type trait.
// \ingroup math_traits
//
// The RowsTrait_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the RowsTrait class template. For instance, given the matrix type \a MT the
// following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::RowsTrait<MT>::Type;
   using Type2 = mtrc::numeric::RowsTrait_t<MT>;
   \endcode
*/
template <typename MT // Type of the matrix
		  ,
		  size_t M> // Number of compile time indices
using RowsTrait_t = typename RowsTrait<MT, M>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the RowsTrait type trait.
// \ingroup math_traits
*/
template <typename MT // Type of the matrix
		  ,
		  size_t M // Number of compile time indices
		  ,
		  typename> // Restricting condition
struct RowsTraitEval1 : public RowsTraitEval2<MT, M> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the RowsTrait type trait.
// \ingroup math_traits
*/
template <typename MT // Type of the matrix
		  ,
		  size_t M // Number of compile time indices
		  ,
		  typename> // Restricting condition
struct RowsTraitEval2 {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
