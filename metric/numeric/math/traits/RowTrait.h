// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_ROWTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_ROWTRAIT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Infinity.h>
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
template <typename, size_t...> struct RowTrait;
template <typename, size_t, typename = void> struct RowTraitEval1;
template <typename, size_t, typename = void> struct RowTraitEval2;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <size_t I, typename T> auto evalRowTrait(const volatile T &) -> RowTraitEval1<T, I>;

template <typename T> auto evalRowTrait(const volatile T &) -> RowTraitEval1<T, inf>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the RowTrait class.
// \ingroup math_traits
//
// \section rowtrait_general General
//
// The RowTrait class template offers the possibility to select the resulting data type when
// creating a view on a specific row of a dense or sparse matrix. In case the given type \a MT
// is a dense or sparse matrix type, RowTrait defines the nested type \a Type, which represents
// the resulting data type of the row operation. Otherwise there is no nested type \a Type.
// Note that \a const and \a volatile qualifiers and reference modifiers are generally ignored.
//
//
// \section rowtrait_specializations Creating custom specializations
//
// Per default, RowTrait supports all matrix types of the Metric numeric library (including views and
// adaptors). For all other data types it is possible to specialize the RowTrait template. The
// following example shows the according specialization for the DynamicMatrix class template:

   \code
   template< typename T1, bool SO, size_t... CRAs >
   struct RowTrait< DynamicMatrix<T1,SO>, CRAs... >
   {
	  using Type = DynamicVector<T1,true>;
   };
   \endcode

// \n \section rowtrait_examples Examples
//
// The following example demonstrates the use of the RowTrait template, where depending on
// the given matrix type the resulting row type is selected:

   \code
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnMajor;

   // Definition of the row type of a row-major dynamic matrix
   using MatrixType1 = mtrc::numeric::DynamicMatrix<int,rowMajor>;
   using ResultType1 = typename mtrc::numeric::RowTrait<MatrixType1>::Type;

   // Definition of the row type for the 1st row of a column-major static matrix
   using MatrixType2 = mtrc::numeric::StaticMatrix<int,4UL,3UL,columnMajor>;
   using ResultType2 = typename mtrc::numeric::RowTrait<MatrixType2,1UL>::Type;
   \endcode
*/
template <typename MT // Type of the matrix
		  ,
		  size_t... CRAs> // Compile time row arguments
struct RowTrait : public decltype(evalRowTrait<CRAs...>(std::declval<MT &>())) {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the RowTrait type trait.
// \ingroup math_traits
//
// The RowTrait_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the RowTrait class template. For instance, given the matrix type \a MT the
// following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::RowTrait<MT>::Type;
   using Type2 = mtrc::numeric::RowTrait_t<MT>;
   \endcode
*/
template <typename MT // Type of the matrix
		  ,
		  size_t... CRAs> // Compile time row arguments
using RowTrait_t = typename RowTrait<MT, CRAs...>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the RowTrait type trait.
// \ingroup math_traits
*/
template <typename MT // Type of the matrix
		  ,
		  size_t I // Compile time row index
		  ,
		  typename> // Restricting condition
struct RowTraitEval1 : public RowTraitEval2<MT, I> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the RowTrait type trait.
// \ingroup math_traits
*/
template <typename MT // Type of the matrix
		  ,
		  size_t I // Compile time row index
		  ,
		  typename> // Restricting condition
struct RowTraitEval2 {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
