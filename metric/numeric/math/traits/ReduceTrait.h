// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_REDUCETRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_REDUCETRAIT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/ReductionFlag.h>
#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/IsVector.h>
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
template <typename, typename, ReductionFlag...> struct ReduceTrait;
template <typename, typename, typename = void> struct TotalReduceTraitEval1;
template <typename, typename, typename = void> struct TotalReduceTraitEval2;
template <typename, typename, ReductionFlag, typename = void> struct PartialReduceTraitEval1;
template <typename, typename, ReductionFlag, typename = void> struct PartialReduceTraitEval2;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <ReductionFlag RF, typename T, typename OP>
auto evalReduceTrait(const volatile T &, OP) -> PartialReduceTraitEval1<T, OP, RF>;

template <typename T, typename OP> auto evalReduceTrait(const volatile T &, OP) -> TotalReduceTraitEval1<T, OP>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the ReduceTrait class.
// \ingroup math_traits
//
// \section reducetrait_general General
//
// The ReduceTrait class template offers the possibility to select the resulting data type of
// a generic reduction operation on the given type \a T. ReduceTrait defines the nested type
// \a Type, which represents the resulting data type of the reduction operation. In case no
// result type can be determined for the type \a T, there is no nested type \a Type. Note that
// \c const and \c volatile qualifiers and reference modifiers are generally ignored.
//
//
// \n \section reducetrait_specializations Creating custom specializations
//
// ReduceTrait is guaranteed to work for all vector and matrix types of the Metric numeric library
// (including views and adaptors) and all data types that work in combination with the provided
// reduction operation \a OP. In order to add support for user-defined data types or in order to
// adapt to special cases it is possible to specialize the ReduceTrait template. The following
// examples shows the according specialization for total and partial reduction operations with
// a dynamic matrix, respectively:

   \code
   template< typename T, bool SO, typename OP >
   struct ReduceTrait< DynamicMatrix<T,SO>, OP >
   {
	  using Type = T;
   };
   \endcode

   \code
   template< typename T, bool SO, typename OP >
   struct ReduceTrait< DynamicMatrix<T,SO>, OP, columnwise >
   {
	  using Type = DynamicVector<T,rowVector>;
   };

   template< typename T, bool SO, typename OP >
   struct ReduceTrait< DynamicMatrix<T,SO>, OP, rowwise >
   {
	  using Type = DynamicVector<T,columnVector>;
   };
   \endcode
*/
template <typename T // Type of the operand
		  ,
		  typename OP // Type of the reduction operation
		  ,
		  ReductionFlag... RF> // Reduction flag
struct ReduceTrait : public decltype(evalReduceTrait<RF...>(std::declval<T &>(), std::declval<OP>())) {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the ReduceTrait class template.
// \ingroup math_traits
//
// The ReduceTrait_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the ReduceTrait class template. For instance, given the type \a T and the custom
// operation type \a OP the following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::ReduceTrait<T,OP>::Type;
   using Type2 = mtrc::numeric::ReduceTrait_t<T,OP>;
   \endcode
*/
template <typename T // Type of the operand
		  ,
		  typename OP // Type of the reduction operation
		  ,
		  ReductionFlag... RF> // Reduction flag
using ReduceTrait_t = typename ReduceTrait<T, OP, RF...>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the ReduceTrait type trait.
// \ingroup math_traits
*/
template <typename T // Type of the operand
		  ,
		  typename OP // Type of the custom operation
		  ,
		  typename> // Restricting condition
struct TotalReduceTraitEval1 : public TotalReduceTraitEval2<T, OP> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the ReduceTrait type trait.
// \ingroup math_traits
*/
template <typename T // Type of the operand
		  ,
		  typename OP // Type of the custom operation
		  ,
		  typename> // Restricting condition
struct TotalReduceTraitEval2 {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the TotalReduceTraitEval2 class template for vectors and matrices.
// \ingroup math_traits
*/
template <typename T // Type of the operand
		  ,
		  typename OP> // Type of the custom operation
struct TotalReduceTraitEval2<T, OP, EnableIf_t<IsVector_v<T> || IsMatrix_v<T>>> {
  public:
	//**********************************************************************************************
	using Type = decltype(std::declval<OP>()(std::declval<ElementType_t<T>>(), std::declval<ElementType_t<T>>()));
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the ReduceTrait type trait.
// \ingroup math_traits
*/
template <typename T // Type of the operand
		  ,
		  typename OP // Type of the custom operation
		  ,
		  ReductionFlag RF // Reduction flag
		  ,
		  typename> // Restricting condition
struct PartialReduceTraitEval1 : public PartialReduceTraitEval2<T, OP, RF> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the ReduceTrait type trait.
// \ingroup math_traits
*/
template <typename T // Type of the operand
		  ,
		  typename OP // Type of the custom operation
		  ,
		  ReductionFlag RF // Reduction flag
		  ,
		  typename> // Restricting condition
struct PartialReduceTraitEval2 {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
