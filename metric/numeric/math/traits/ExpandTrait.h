// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_EXPANDTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_EXPANDTRAIT_H
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
template <typename, size_t...> struct ExpandTrait;
template <typename, size_t, typename = void> struct ExpandTraitEval1;
template <typename, size_t, typename = void> struct ExpandTraitEval2;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <size_t E, typename T> auto evalExpandTrait(const volatile T &) -> ExpandTraitEval1<T, E>;

template <typename T> auto evalExpandTrait(const volatile T &) -> ExpandTraitEval1<T, inf>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the ExpandTrait class.
// \ingroup math_traits
//
// \section expandtrait_general General
//
// The ExpandTrait class template offers the possibility to select the resulting data type when
// expanding a dense or sparse vector or matrix. ExpandTrait defines the nested type \a Type,
// which represents the resulting data type of the expand operation. In case the given data type
// is not a dense or sparse vector or matrix type, there is no nested type \a Type. Note that
// \a const and \a volatile qualifiers and reference modifiers are generally ignored.
//
//
// \section expandtrait_specializations Creating custom specializations
//
// Per default, ExpandTrait supports all vector and matrix types of the Metric numeric library (including
// views and adaptors). For all other data types it is possible to specialize the ExpandTrait
// template. The following example shows the according specialization for the DynamicVector class
// template:

   \code
   template< typename Type, bool TF, size_t... CEAs >
   struct ExpandTrait< DynamicVector<Type,TF>, CEAs... >
   {
	  using Type = DynamicMatrix<Type,( TF == columnVector ? columnMajor : rowMajor )>;
   };
   \endcode

// \n \section expandtrait_examples Examples
//
// The following example demonstrates the use of the ExpandTrait template, where depending on
// the given vector or matrix type the resulting type is selected:

   \code
   using mtrc::numeric::columnVector;
   using mtrc::numeric::rowVector;

   // Definition of the resulting type of a dynamic column vector
   using VectorType1 = mtrc::numeric::DynamicVector<int,columnVector>;
   using ResultType1 = typename mtrc::numeric::ExpandTrait<VectorType1>::Type;

   // Definition of the resulting type of a static row vector
   using VectorType2 = mtrc::numeric::StaticVector<int,5UL,rowVector>;
   using ResultType2 = typename mtrc::numeric::ExpandTrait<VectorType2>::Type;
   \endcode
*/
template <typename T // Type of the operand
		  ,
		  size_t... CEAs> // Compile time expansion arguments
struct ExpandTrait : public decltype(evalExpandTrait<CEAs...>(std::declval<T &>())) {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the ExpandTrait type trait.
// \ingroup math_traits
//
// The ExpandTrait_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the ExpandTrait class template. For instance, given the matrix type \a MT the
// following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::ExpandTrait<MT>::Type;
   using Type2 = mtrc::numeric::ExpandTrait_t<MT>;
   \endcode
*/
template <typename T // Type of the operand
		  ,
		  size_t... CEAs> // Compile time expansion arguments
using ExpandTrait_t = typename ExpandTrait<T, CEAs...>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the ExpandTrait type trait.
// \ingroup math_traits
*/
template <typename T // Type of the operand
		  ,
		  size_t E // Compile time expansion
		  ,
		  typename> // Restricting condition
struct ExpandTraitEval1 : public ExpandTraitEval2<T, E> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the ExpandTrait type trait.
// \ingroup math_traits
*/
template <typename T // Type of the operand
		  ,
		  size_t E // Compile time expansion
		  ,
		  typename> // Restricting condition
struct ExpandTraitEval2 {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
