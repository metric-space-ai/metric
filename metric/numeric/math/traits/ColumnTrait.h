// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_COLUMNTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_COLUMNTRAIT_H
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
template <typename, size_t...> struct ColumnTrait;
template <typename, size_t, typename = void> struct ColumnTraitEval1;
template <typename, size_t, typename = void> struct ColumnTraitEval2;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <size_t I, typename T> auto evalColumnTrait(const volatile T &) -> ColumnTraitEval1<T, I>;

template <typename T> auto evalColumnTrait(const volatile T &) -> ColumnTraitEval1<T, inf>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the ColumnTrait class.
// \ingroup math_traits
//
// \section columntrait_general General
//
// The ColumnTrait class template offers the possibility to select the resulting data type when
// creating a view on a specific column of a dense or sparse matrix. In case the given type \a MT
// is a dense or sparse matrix type, ColumnTrait defines the nested type \a Type, which represents
// the resulting data type of the column operation. Otherwise there is no nested type \a Type.
// Note that \a const and \a volatile qualifiers and reference modifiers are generally ignored.
//
//
// \section columntrait_specializations Creating custom specializations
//
// Per default, ColumnTrait supports all matrix types of the Metric numeric library (including views and
// adaptors). For all other data types it is possible to specialize the ColumnTrait template. The
// following example shows the according specialization for the DynamicMatrix class template:

   \code
   template< typename T1, bool SO, size_t... CCAs >
   struct ColumnTrait< DynamicMatrix<T1,SO>, CCAs... >
   {
	  using Type = DynamicVector<T1,true>;
   };
   \endcode

// \n \section columntrait_examples Examples
//
// The following example demonstrates the use of the ColumnTrait template, where depending on
// the given matrix type the resulting column type is selected:

   \code
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnMajor;

   // Definition of the column type of a column-major dynamic matrix
   using MatrixType1 = mtrc::numeric::DynamicMatrix<int,columnMajor>;
   using ResultType1 = typename mtrc::numeric::ColumnTrait<MatrixType1>::Type;

   // Definition of the column type for the 1st column of a row-major static matrix
   using MatrixType2 = mtrc::numeric::StaticMatrix<int,3UL,4UL,rowMajor>;
   using ResultType2 = typename mtrc::numeric::ColumnTrait<MatrixType2,1UL>::Type;
   \endcode
*/
template <typename MT // Type of the matrix
		  ,
		  size_t... CCAs> // Compile time column arguments
struct ColumnTrait : public decltype(evalColumnTrait<CCAs...>(std::declval<MT &>())) {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the ColumnTrait type trait.
// \ingroup math_traits
//
// The ColumnTrait_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the ColumnTrait class template. For instance, given the matrix type \a MT the
// following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::ColumnTrait<MT>::Type;
   using Type2 = mtrc::numeric::ColumnTrait_t<MT>;
   \endcode
*/
template <typename MT // Type of the matrix
		  ,
		  size_t... CCAs> // Compile time column arguments
using ColumnTrait_t = typename ColumnTrait<MT, CCAs...>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the ColumnTrait type trait.
// \ingroup math_traits
*/
template <typename MT // Type of the matrix
		  ,
		  size_t I // Compile time column index
		  ,
		  typename> // Restricting condition
struct ColumnTraitEval1 : public ColumnTraitEval2<MT, I> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the ColumnTrait type trait.
// \ingroup math_traits
*/
template <typename MT // Type of the matrix
		  ,
		  size_t I // Compile time column index
		  ,
		  typename> // Restricting condition
struct ColumnTraitEval2 {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
