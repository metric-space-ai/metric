// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_SUBVECTORTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_SUBVECTORTRAIT_H
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
template <typename, size_t...> struct SubvectorTrait;
template <typename, size_t, size_t, typename = void> struct SubvectorTraitEval1;
template <typename, size_t, size_t, typename = void> struct SubvectorTraitEval2;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <size_t I, size_t N, typename T> auto evalSubvectorTrait(const volatile T &) -> SubvectorTraitEval1<T, I, N>;

template <typename T> auto evalSubvectorTrait(const volatile T &) -> SubvectorTraitEval1<T, inf, inf>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the SubvectorTrait class.
// \ingroup math_traits
//
// \section subvectortrait_general General
//
// The SubvectorTrait class template offers the possibility to select the resulting data type
// when creating a subvector of a dense or sparse vector. In case the given type \a VT is a
// dense or sparse vector type, SubvectorTrait defines the nested type \a Type, which represents
// the resulting data type of the subvector operation. Otherwise there is no nested type \a Type.
// Note that \a const and \a volatile qualifiers and reference modifiers are generally ignored.
//
//
// \section subvectortrait_specializations Creating custom specializations
//
// Per default, SubvectorTrait supports all vector types of the Metric numeric library (including views
// and adaptors). For all other data types it is possible to specialize the SubvectorTrait
// template. The following example shows the according specialization for the DynamicVector
// class template:

   \code
   template< typename T1, bool TF, size_t... CSAs >
   struct SubvectorTrait< DynamicVector<T1,TF>, CSAs... >
   {
	  using Type = DynamicVector<T1,TF>;
   };
   \endcode

// \n \section subvectortrait_examples Examples
//
// The following example demonstrates the use of the SubvectorTrait template, where depending
// on the given vector type the according result type is selected:

   \code
   using mtrc::numeric::columnVector;
   using mtrc::numeric::rowVector;

   // Definition of the result type of a dynamic column vector
   using VectorType1 = mtrc::numeric::DynamicVector<int,columnVector>;
   using ResultType1 = typename mtrc::numeric::SubvectorTrait<VectorType1>::Type;

   // Definition of the result type for the inner two elements of a static row vector
   using VectorType2 = mtrc::numeric::StaticVector<int,4UL,rowVector>;
   using ResultType2 = typename mtrc::numeric::SubvectorTrait<VectorType2,1UL,2UL>::Type;
   \endcode
*/
template <typename VT // Type of the vector
		  ,
		  size_t... CSAs> // Compile time subvector arguments
struct SubvectorTrait : public decltype(evalSubvectorTrait<CSAs...>(std::declval<VT &>())) {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the SubvectorTrait type trait.
// \ingroup math_traits
//
// The SubvectorTrait_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the SubvectorTrait class template. For instance, given the vector type \a VT the
// following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::SubvectorTrait<VT>::Type;
   using Type2 = mtrc::numeric::SubvectorTrait_t<VT>;
   \endcode
*/
template <typename VT // Type of the vector
		  ,
		  size_t... CSAs> // Compile time subvector arguments
using SubvectorTrait_t = typename SubvectorTrait<VT, CSAs...>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the SubvectorTrait type trait.
// \ingroup math_traits
*/
template <typename VT // Type of the vector
		  ,
		  size_t I // Index of the first element
		  ,
		  size_t N // Number of elements
		  ,
		  typename> // Restricting condition
struct SubvectorTraitEval1 : public SubvectorTraitEval2<VT, I, N> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the SubvectorTrait type trait.
// \ingroup math_traits
*/
template <typename VT // Type of the vector
		  ,
		  size_t I // Index of the first element
		  ,
		  size_t N // Number of elements
		  ,
		  typename> // Restricting condition
struct SubvectorTraitEval2 {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
