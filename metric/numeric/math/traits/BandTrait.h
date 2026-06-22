// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_BANDTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_BANDTRAIT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Infinity.h>
#include <metric/numeric/util/InvalidType.h>
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
template <typename, ptrdiff_t...> struct BandTrait;
template <typename, ptrdiff_t, typename = void> struct BandTraitEval1;
template <typename, ptrdiff_t, typename = void> struct BandTraitEval2;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <ptrdiff_t I, typename T> auto evalBandTrait(const volatile T &) -> BandTraitEval1<T, I>;

template <typename T> auto evalBandTrait(const volatile T &) -> BandTraitEval1<T, inf>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the BandTrait class.
// \ingroup math_traits
//
// \section bandtrait_general General
//
// The BandTrait class template offers the possibility to select the resulting data type when
// creating a view on a specific band of a dense or sparse matrix. In case the given type \a MT
// is a dense or sparse matrix type, BandTrait defines the nested type \a Type, which represents
// the resulting data type of the band operation. Otherwise there is no nested type \a Type.
// Note that \a const and \a volatile qualifiers and reference modifiers are generally ignored.
//
//
// \section bandtrait_specializations Creating custom specializations
//
// Per default, BandTrait supports all matrix types of the Metric numeric library (including views and
// adaptors). For all other data types it is possible to specialize the BandTrait template. The
// following example shows the according specialization for the DynamicMatrix class template:

   \code
   template< typename T1, bool SO, ptrdiff_t... CBAs >
   struct BandTrait< DynamicMatrix<T1,SO>, CBAs... >
   {
	  using Type = DynamicVector<T1,true>;
   };
   \endcode

// \n \section bandtrait_examples Examples
//
// The following example demonstrates the use of the BandTrait template, where depending on
// the given matrix type the resulting vector type is selected:

   \code
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnMajor;

   // Definition of the fitting type for any band of a row-major dynamic matrix
   using MatrixType1 = mtrc::numeric::DynamicMatrix<int,rowMajor>;
   using ResultType1 = typename mtrc::numeric::BandTrait<MatrixType1>::Type;

   // Definition of the fitting type of the 3rd band of a column-major static matrix
   using MatrixType2 = mtrc::numeric::StaticMatrix<int,3UL,3UL,columnMajor>;
   using ResultType2 = typename mtrc::numeric::BandTrait<MatrixType2,3L>::Type;
   \endcode
*/
template <typename MT // Type of the matrix
		  ,
		  ptrdiff_t... CBAs> // Compile time band arguments
struct BandTrait : public decltype(evalBandTrait<CBAs...>(std::declval<MT &>())) {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the BandTrait type trait.
// \ingroup math_traits
//
// The BandTrait_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the BandTrait class template. For instance, given the matrix type \a MT the
// following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::BandTrait<MT>::Type;
   using Type2 = mtrc::numeric::BandTrait_t<MT>;
   \endcode
*/
template <typename MT // Type of the matrix
		  ,
		  ptrdiff_t... CBAs> // Compile time band arguments
using BandTrait_t = typename BandTrait<MT, CBAs...>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the BandTrait type trait.
// \ingroup math_traits
*/
template <typename MT // Type of the matrix
		  ,
		  ptrdiff_t I // Compile time band index
		  ,
		  typename> // Restricting condition
struct BandTraitEval1 : public BandTraitEval2<MT, I> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the BandTrait type trait.
// \ingroup math_traits
*/
template <typename MT // Type of the matrix
		  ,
		  ptrdiff_t I // Compile time band index
		  ,
		  typename> // Restricting condition
struct BandTraitEval2 {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
