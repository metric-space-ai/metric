// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_ADDTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_ADDTRAIT_H
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
template <typename, typename, typename = void> struct AddTrait;
template <typename, typename, typename = void> struct AddTraitEval1;
template <typename, typename, typename = void> struct AddTraitEval2;
template <typename, typename, typename = void> struct AddTraitEval3;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2>
auto evalAddTrait(const volatile T1 &, const volatile T2 &) -> AddTraitEval1<T1, T2>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the AddTrait class.
// \ingroup math_traits
//
// \section addtrait_general General
//
// The AddTrait class template offers the possibility to select the resulting data type of a
// generic addition operation between the two given types \a T1 and \a T2. AddTrait defines
// the nested type \a Type, which represents the resulting data type of the addition. In case
// the two types \a T1 and \a T2 cannot be added, there is no nested type \a Type. Note that
// \c const and \c volatile qualifiers and reference modifiers are generally ignored.
//
//
// \n \section addtrait_specializations Creating custom specializations
//
// AddTrait is guaranteed to work for all built-in data types, complex numbers, all vector and
// matrix types of the Metric numeric library (including views and adaptors) and all data types that
// provide an addition operator (i.e. \c operator+). In order to add support for user-defined
// data types that either don't provide an addition operator or whose addition operator returns
// a proxy object instead of a concrete type (as it is for instance common in expression template
// libraries) it is possible to specialize the AddTrait template. The following example shows the
// according specialization for the addition between two dynamic column vectors:

   \code
   template< typename T1, typename T2 >
   struct AddTrait< DynamicVector<T1,columnVector>, DynamicVector<T2,columnVector> >
   {
	  using Type = DynamicVector< typename AddTrait<T1,T2>::Type, columnVector >;
   };
   \endcode

// \n \section addtrait_examples Examples
//
// The following example demonstrates the use of the AddTrait template, where depending on
// the two given data types the resulting data type is selected:

   \code
   template< typename T1, typename T2 >  // The two generic types
   typename AddTrait<T1,T2>::Type        // The resulting generic return type
   add( const T1& t1, const T2& t2 )     //
   {                                     // The function 'add' returns the sum
	  return t1 + t2;                    // of the two given values
   }                                     //
   \endcode
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename> // Restricting condition
struct AddTrait : public decltype(evalAddTrait(std::declval<T1 &>(), std::declval<T2 &>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the AddTrait class template for two numeric types.
// \ingroup math_traits
*/
template <typename T1, typename T2> struct AddTrait<T1, T2, EnableIf_t<IsNumeric_v<T1> && IsNumeric_v<T2>>> {
  public:
	//**********************************************************************************************
	using Type = CommonType_t<T1, T2>;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the AddTrait class template.
// \ingroup math_traits
//
// The AddTrait_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the AddTrait class template. For instance, given the types \a T1 and \a T2 the following
// two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::AddTrait<T1,T2>::Type;
   using Type2 = mtrc::numeric::AddTrait_t<T1,T2>;
   \endcode
*/
template <typename T1, typename T2> using AddTrait_t = typename AddTrait<T1, T2>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the AddTrait type trait.
// \ingroup math_traits
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename> // Restricting condition
struct AddTraitEval1 : public AddTraitEval2<T1, T2> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the AddTraitEval1 class template for two 'GroupTag'.
// \ingroup math_traits
*/
template <size_t ID> struct AddTraitEval1<GroupTag<ID>, GroupTag<ID>, void> {
  public:
	//**********************************************************************************************
	using Type = GroupTag<ID>;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the AddTrait type trait.
// \ingroup math_traits
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename> // Restricting condition
struct AddTraitEval2 : public AddTraitEval3<T1, T2> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Third auxiliary helper struct for the AddTrait type trait.
// \ingroup math_traits
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename> // Restricting condition
struct AddTraitEval3 {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the AddTraitEval3 class template for two types supporting addition.
// \ingroup math_traits
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2> // Type of the right-hand side operand
struct AddTraitEval3<T1, T2, Void_t<decltype(std::declval<T1>() + std::declval<T2>())>> {
  public:
	//**********************************************************************************************
	using Type = RemoveCVRef_t<decltype(std::declval<T1>() + std::declval<T2>())>;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
