// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_SUBTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_SUBTRAIT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/GroupTag.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/typetraits/CommonType.h>
#include <metric/numeric/util/typetraits/IsNumeric.h>
#include <metric/numeric/util/typetraits/RemoveCVRef.h>
#include <metric/numeric/util/typetraits/Void.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename, typename, typename = void> struct SubTrait;
template <typename, typename, typename = void> struct SubTraitEval1;
template <typename, typename, typename = void> struct SubTraitEval2;
template <typename, typename, typename = void> struct SubTraitEval3;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2>
auto evalSubTrait(const volatile T1 &, const volatile T2 &) -> SubTraitEval1<T1, T2>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the SubTrait class.
// \ingroup math_traits
//
// \section subtrait_general General
//
// The SubTrait class template offers the possibility to select the resulting data type of a
// generic subtraction operation between the two given types \a T1 and \a T2. SubTrait defines
// the nested type \a Type, which represents the resulting data type of the subtraction. In case
// the two types \a T1 and \a T2 cannot be subtracted, there is no nested type \a Type. Note that
// \c const and \c volatile qualifiers and reference modifiers are generally ignored.
//
//
// \n \section subtrait_specializations Creating custom specializations
//
// SubTrait is guaranteed to work for all built-in data types, complex numbers, all vector and
// matrix types of the Metric numeric library (including views and adaptors) and all data types that
// provide a subtraction operator (i.e. \c operator-). In order to add support for user-defined
// data types that either don't provide a subtraction operator or whose subtraction operator
// returns a proxy object instead of a concrete type (as it is for instance common in expression
// template libraries) it is possible to specialize the SubTrait template. The following example
// shows the according specialization for the subtraction between two dynamic column vectors:

   \code
   template< typename T1, typename T2 >
   struct SubTrait< DynamicVector<T1,columnVector>, DynamicVector<T2,columnVector> >
   {
	  using Type = DynamicVector< typename SubTrait<T1,T2>::Type, columnVector >;
   };
   \endcode

// \n \section subtrait_examples Examples
//
// The following example demonstrates the use of the SubTrait template, where depending on
// the two given data types the resulting data type is selected:

   \code
   template< typename T1, typename T2 >  // The two generic types
   typename SubTrait<T1,T2>::Type        // The resulting generic return type
   sub( const T1& t1, const T2& t2 )     //
   {                                     // The function 'sub' returns the
	  return t1 - t2;                    // difference of the two given values
   }                                     //
   \endcode
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename> // Restricting condition
struct SubTrait : public decltype(evalSubTrait(std::declval<T1 &>(), std::declval<T2 &>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the SubTrait class template for two numeric types.
// \ingroup math_traits
*/
template <typename T1, typename T2> struct SubTrait<T1, T2, EnableIf_t<IsNumeric_v<T1> && IsNumeric_v<T2>>> {
  public:
	//**********************************************************************************************
	using Type = CommonType_t<T1, T2>;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the SubTrait class template.
// \ingroup math_traits
//
// The SubTrait_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the SubTrait class template. For instance, given the types \a T1 and \a T2 the following
// two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::SubTrait<T1,T2>::Type;
   using Type2 = mtrc::numeric::SubTrait_t<T1,T2>;
   \endcode
*/
template <typename T1, typename T2> using SubTrait_t = typename SubTrait<T1, T2>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the SubTrait type trait.
// \ingroup math_traits
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename> // Restricting condition
struct SubTraitEval1 : public SubTraitEval2<T1, T2> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the SubTraitEval1 class template for two 'GroupTag'.
// \ingroup math_traits
*/
template <size_t ID> struct SubTraitEval1<GroupTag<ID>, GroupTag<ID>, void> {
  public:
	//**********************************************************************************************
	using Type = GroupTag<ID>;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the SubTrait type trait.
// \ingroup math_traits
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename> // Restricting condition
struct SubTraitEval2 : public SubTraitEval3<T1, T2> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Third auxiliary helper struct for the SubTrait type trait.
// \ingroup math_traits
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename> // Restricting condition
struct SubTraitEval3 {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the SubTraitEval3 class template for two types supporting subtraction.
// \ingroup math_traits
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
struct SubTraitEval3<T1, T2, Void_t<decltype(std::declval<T1>() - std::declval<T2>())>> {
  public:
	//**********************************************************************************************
	using Type = RemoveCVRef_t<decltype(std::declval<T1>() - std::declval<T2>())>;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
