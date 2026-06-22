// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_CROSSTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_CROSSTRAIT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename, typename, typename = void> struct CrossTrait;
template <typename, typename, typename = void> struct CrossTraitEval1;
template <typename, typename, typename = void> struct CrossTraitEval2;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2>
auto evalCrossTrait(const volatile T1 &, const volatile T2 &) -> CrossTraitEval1<T1, T2>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the CrossTrait class.
// \ingroup math_traits
//
// \section crosstrait_general General
//
// The CrossTrait class template offers the possibility to select the resulting data type of
// a generic cross product operation between the two given types \a T1 and \a T2. CrossTrait
// defines the nested type \a Type, which represents the resulting data type of the cross
// product. In case \a T1 and \a T2 cannot be combined in a cross product, there is no nested
// type \a Type. Note that \a const and \a volatile qualifiers and reference modifiers are
// generally ignored.
//
//
// \n \section crosstrait_specializations Creating custom specializations
//
// Per default, CrossTrait supports all vector types of the Metric numeric library (including views and
// adaptors). For all other data types it is possible to specialize the CrossTrait template. The
// following example shows the according specialization for the cross product between two static
// column vectors:

   \code
   template< typename T1, typename T2 >
   struct CrossTrait< StaticVector<T1,3UL,false>, StaticVector<T2,3UL,false> >
   {
	  using Type = StaticVector< typename SubTrait< typename MultTrait<T1,T2>::Type
												  , typename MultTrait<T1,T2>::Type >::Type, 3UL, false >;
   };
   \endcode

// \n \section crosstrait_examples Examples
//
// The following example demonstrates the use of the CrossTrait template, where depending on
// the two given data types the resulting data type is selected:

   \code
   template< typename T1, typename T2 >  // The two generic types
   typename CrossTrait<T1,T2>::Type      // The resulting generic return type
   cross( T1 t1, T2 t2 )                 //
   {                                     // The function 'cross' returns the cross
	  return t1 % t2;                    // product of the two given values
   }                                     //
   \endcode
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename> // Restricting condition
struct CrossTrait : public decltype(evalCrossTrait(std::declval<T1 &>(), std::declval<T2 &>())){};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the CrossTrait class template.
// \ingroup math_traits
//
// The CrossTrait_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the CrossTrait class template. For instance, given the types \a T1 and \a T2 the following
// two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::CrossTrait<T1,T2>::Type;
   using Type2 = mtrc::numeric::CrossTrait_t<T1,T2>;
   \endcode
*/
template <typename T1, typename T2> using CrossTrait_t = typename CrossTrait<T1, T2>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the CrossTrait type trait.
// \ingroup math_traits
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename> // Restricting condition
struct CrossTraitEval1 : public CrossTraitEval2<T1, T2> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the CrossTrait type trait.
// \ingroup math_traits
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename> // Restricting condition
struct CrossTraitEval2 {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
