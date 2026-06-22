// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_REPEATTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_REPEATTRAIT_H
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
template <typename, size_t...> struct RepeatTrait;
template <typename, size_t, size_t, size_t, typename = void> struct RepeatTraitEval1;
template <typename, size_t, size_t, size_t, typename = void> struct RepeatTraitEval2;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <size_t R0, size_t R1, size_t R2, typename T>
auto evalRepeatTrait(const volatile T &) -> RepeatTraitEval1<T, R0, R1, R2>;

template <size_t R0, size_t R1, typename T>
auto evalRepeatTrait(const volatile T &) -> RepeatTraitEval1<T, R0, R1, inf>;

template <size_t R0, typename T> auto evalRepeatTrait(const volatile T &) -> RepeatTraitEval1<T, R0, inf, inf>;

template <typename T> auto evalRepeatTrait(const volatile T &) -> RepeatTraitEval1<T, inf, inf, inf>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the RepeatTrait class.
// \ingroup math_traits
//
// \section repeattrait_general General
//
// The RepeatTrait class template offers the possibility to select the resulting data type when
// repeating a dense or sparse vector or matrix. RepeatTrait defines the nested type \a Type,
// which represents the resulting data type of the repeat operation. In case the given data type
// is not a dense or sparse vector or matrix type, there is no nested type \a Type. Note that
// \a const and \a volatile qualifiers and reference modifiers are generally ignored.
//
//
// \section repeattrait_specializations Creating custom specializations
//
// Per default, RepeatTrait supports all vector and matrix types of the Metric numeric library (including
// views and adaptors). For all other data types it is possible to specialize the RepeatTrait
// template. The following example shows the according specialization for the DynamicVector class
// template:

   \code
   template< typename Type, bool TF, size_t... CRAs >
   struct RepeatTrait< DynamicVector<Type,TF>, CRAs... >
   {
	  using Type = DynamicVector<Type,TF>;
   };
   \endcode

// \n \section repeattrait_examples Examples
//
// The following example demonstrates the use of the RepeatTrait template, where depending on
// the given vector or matrix type the resulting type is selected:

   \code
   using mtrc::numeric::columnVector;
   using mtrc::numeric::rowVector;

   // Definition of the resulting type of a dynamic column vector
   using VectorType1 = mtrc::numeric::DynamicVector<int,columnVector>;
   using ResultType1 = typename mtrc::numeric::RepeatTrait<VectorType1>::Type;

   // Definition of the resulting type of a static row vector
   using VectorType2 = mtrc::numeric::StaticVector<int,5UL,rowVector>;
   using ResultType2 = typename mtrc::numeric::RepeatTrait<VectorType2>::Type;
   \endcode
*/
template <typename T // Type of the operand
		  ,
		  size_t... CRAs> // Compile time repetition arguments
struct RepeatTrait : public decltype(evalRepeatTrait<CRAs...>(std::declval<T &>())) {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the RepeatTrait type trait.
// \ingroup math_traits
//
// The RepeatTrait_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the RepeatTrait class template. For instance, given the matrix type \a MT the
// following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::RepeatTrait<MT>::Type;
   using Type2 = mtrc::numeric::RepeatTrait_t<MT>;
   \endcode
*/
template <typename T // Type of the operand
		  ,
		  size_t... CRAs> // Compile time repeater arguments
using RepeatTrait_t = typename RepeatTrait<T, CRAs...>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the RepeatTrait type trait.
// \ingroup math_traits
*/
template <typename T // Type of the operand
		  ,
		  size_t R0 // Number of repetitions in the first dimension
		  ,
		  size_t R1 // Number of repetitions in the second dimension
		  ,
		  size_t R2 // Number of repetitions in the third dimension
		  ,
		  typename> // Restricting condition
struct RepeatTraitEval1 : public RepeatTraitEval2<T, R0, R1, R2> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the RepeatTrait type trait.
// \ingroup math_traits
*/
template <typename T // Type of the operand
		  ,
		  size_t R0 // Number of repetitions in the first dimension
		  ,
		  size_t R1 // Number of repetitions in the second dimension
		  ,
		  size_t R2 // Number of repetitions in the third dimension
		  ,
		  typename> // Restricting condition
struct RepeatTraitEval2 {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
