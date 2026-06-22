// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_SUBMATRIXTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_SUBMATRIXTRAIT_H
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
template <typename, size_t...> struct SubmatrixTrait;
template <typename, size_t, size_t, size_t, size_t, typename = void> struct SubmatrixTraitEval1;
template <typename, size_t, size_t, size_t, size_t, typename = void> struct SubmatrixTraitEval2;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <size_t I, size_t J, size_t M, size_t N, typename T>
auto evalSubmatrixTrait(const volatile T &) -> SubmatrixTraitEval1<T, I, J, M, N>;

template <typename T> auto evalSubmatrixTrait(const volatile T &) -> SubmatrixTraitEval1<T, inf, inf, inf, inf>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the SubmatrixTrait class.
// \ingroup math_traits
//
// \section submatrixtrait_general General
//
// The SubmatrixTrait class template offers the possibility to select the resulting data type
// when creating a submatrix of a dense or sparse matrix. In case the given type \a MT is a
// dense or sparse matrix type, SubmatrixTrait defines the nested type \a Type, which represents
// the resulting data type of the submatrix operation. Otherwise therer is no nested type \a Type.
// Note that \a const and \a volatile qualifiers and reference modifiers are generally ignored.
//
//
// \section submatrixtrait_specializations Creating custom specializations
//
// Per default, SubmatrixTrait supports all matrix types of the Metric numeric library (including views
// and adaptors). For all other data types it is possible to specialize the SubmatrixTrait
// template. The following example shows the according specialization for the DynamicMatrix
// class template:

   \code
   template< typename T1, bool SO >
   struct SubmatrixTrait< DynamicMatrix<T1,SO> >
   {
	  using Type = DynamicMatrix<T1,SO>;
   };
   \endcode

// \n \section submatrixtrait_examples Examples
//
// The following example demonstrates the use of the SubmatrixTrait template, where depending
// on the given matrix type the according result type is selected:

   \code
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnMajor;

   // Definition of the result type of a row-major dynamic matrix
   using MatrixType1 = mtrc::numeric::DynamicMatrix<int,rowMajor>;
   using ResultType1 = typename mtrc::numeric::SubmatrixTrait<MatrixType1>::Type;

   // Definition of the result type for the inner four elements of a 4x4 column-major static matrix
   using MatrixType2 = mtrc::numeric::StaticMatrix<int,4UL,4UL,columnMajor>;
   using ResultType2 = typename mtrc::numeric::SubmatrixTrait<MatrixType2,1UL,1UL,2UL,2UL>::Type;
   \endcode
*/
template <typename MT // Type of the matrix
		  ,
		  size_t... CSAs> // Compile time submatrix arguments
struct SubmatrixTrait : public decltype(evalSubmatrixTrait<CSAs...>(std::declval<MT &>())) {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the SubmatrixTrait type trait.
// \ingroup math_traits
//
// The SubmatrixTrait_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the SubmatrixTrait class template. For instance, given the matrix type \a MT the
// following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::SubmatrixTrait<MT>::Type;
   using Type2 = mtrc::numeric::SubmatrixTrait_t<MT>;
   \endcode
*/
template <typename MT // Type of the matrix
		  ,
		  size_t... CSAs> // Compile time submatrix arguments
using SubmatrixTrait_t = typename SubmatrixTrait<MT, CSAs...>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the SubmatrixTrait type trait.
// \ingroup math_traits
*/
template <typename MT // Type of the matrix
		  ,
		  size_t I // Index of the first row
		  ,
		  size_t J // Index of the first column
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  typename> // Restricting condition
struct SubmatrixTraitEval1 : public SubmatrixTraitEval2<MT, I, J, M, N> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the SubmatrixTrait type trait.
// \ingroup math_traits
*/
template <typename MT // Type of the matrix
		  ,
		  size_t I // Index of the first row
		  ,
		  size_t J // Index of the first column
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  typename> // Restricting condition
struct SubmatrixTraitEval2 {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
