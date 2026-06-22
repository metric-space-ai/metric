// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_ELEMENTSTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_ELEMENTSTRAIT_H
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
template <typename, size_t> struct ElementsTrait;
template <typename, size_t, typename = void> struct ElementsTraitEval1;
template <typename, size_t, typename = void> struct ElementsTraitEval2;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <size_t N, typename T> auto evalElementsTrait(const volatile T &) -> ElementsTraitEval1<T, N>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the ElementsTrait class.
// \ingroup math_traits
//
// \section elementstrait_general General
//
// The ElementsTrait class template offers the possibility to select the resulting data type
// when selecting elements from a dense or sparse vector. In case the given type \a VT is a
// dense or sparse vector type, ElementsTrait defines the nested type \a Type, which represents
// the resulting data type of the elements operation. Otherwise there is no nested type \a Type.
// Note that \a const and \a volatile qualifiers and reference modifiers are generally ignored.
//
//
// \section elementstrait_specializations Creating custom specializations
//
// Per default, ElementsTrait supports all vector types of the Metric numeric library (including views and
// adaptors). For all other data types it is possible to specialize the ElementsTrait template.
// The following example shows the according specialization for the DynamicVector class template:

   \code
   template< typename T1, bool TF, size_t N >
   struct ElementsTrait< DynamicVector<T1,TF>, N >
   {
	  using Type = DynamicVector<T1,TF>;
   };
   \endcode

// \n \section elementstrait_examples Examples
//
// The following example demonstrates the use of the ElementsTrait template, where depending
// on the given vector type the according result type is selected:

   \code
   using mtrc::numeric::columnVector;
   using mtrc::numeric::rowVector;

   // Definition of the result type of a dynamic column vector
   using VectorType1 = mtrc::numeric::DynamicVector<int,columnVector>;
   using ResultType1 = typename mtrc::numeric::ElementsTrait<VectorType1,0UL>::Type;

   // Definition of the result type for two elements of a static row vector
   using VectorType2 = mtrc::numeric::StaticVector<int,4UL,rowVector>;
   using ResultType2 = typename mtrc::numeric::ElementsTrait<VectorType2,2UL>::Type;
   \endcode
*/
template <typename VT // Type of the vector
		  ,
		  size_t N> // Number of compile time indices
struct ElementsTrait : public decltype(evalElementsTrait<N>(std::declval<VT &>())) {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the ElementsTrait type trait.
// \ingroup math_traits
//
// The ElementsTrait_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the ElementsTrait class template. For instance, given the vector type \a VT the
// following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::ElementsTrait<VT>::Type;
   using Type2 = mtrc::numeric::ElementsTrait_t<VT>;
   \endcode
*/
template <typename VT // Type of the vector
		  ,
		  size_t N> // Number of compile time indices
using ElementsTrait_t = typename ElementsTrait<VT, N>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the ElementsTrait type trait.
// \ingroup math_traits
*/
template <typename VT // Type of the vector
		  ,
		  size_t N // Number of compile time indices
		  ,
		  typename> // Restricting condition
struct ElementsTraitEval1 : public ElementsTraitEval2<VT, N> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the ElementsTrait type trait.
// \ingroup math_traits
*/
template <typename VT // Type of the vector
		  ,
		  size_t N // Number of compile time indices
		  ,
		  typename> // Restricting condition
struct ElementsTraitEval2 {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
