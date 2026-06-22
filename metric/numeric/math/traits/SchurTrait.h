// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_SCHURTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_SCHURTRAIT_H
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
template <typename, typename, typename = void> struct SchurTrait;
template <typename, typename, typename = void> struct SchurTraitEval1;
template <typename, typename, typename = void> struct SchurTraitEval2;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2>
auto evalSchurTrait(const volatile T1 &, const volatile T2 &) -> SchurTraitEval1<T1, T2>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the SchurTrait class.
// \ingroup math_traits
//
// \section schurtrait_general General
//
// The SchurTrait class template offers the possibility to select the resulting data type of
// a generic Schur product operation between the two given types \a T1 and \a T2. SchurTrait
// defines the nested type \a Type, which represents the resulting data type of the Schur
// product. In case \a T1 and \a T2 cannot be combined in a Schur product, there is no nested
// type \a Type. Note that \a const and \a volatile qualifiers and reference modifiers are
// generally ignored.
//
//
// \n \section schurtrait_specializations Creating custom specializations
//
// Per default, SchurTrait supports all matrix types of the Metric numeric library (including views and
// adaptors). For all other data types it is possible to specialize the SchurTrait template. The
// following example shows the according specialization for the Schur product between two static
// matrices with equal storage order:

   \code
   template< typename T1, size_t M, size_t N, bool SO, typename T2 >
   struct SchurTrait< StaticMatrix<T1,M,N,SO>, StaticMatrix<T2,M,N,SO> >
   {
	  using Type = StaticMatrix< MultTrait_t<T1,T2>, M, N, SO >;
   };
   \endcode

// \n \section schurtrait_examples Examples
//
// The following example demonstrates the use of the SchurTrait template, where depending on
// the two given data types the resulting data type is selected:

   \code
   template< typename T1, typename T2 >  // The two generic types
   typename SchurTrait<T1,T2>::Type      // The resulting generic return type
   schur( T1 t1, T2 t2 )                 //
   {                                     // The function 'schur' returns the Schur
	  return t1 % t2;                    // product of the two given values
   }                                     //
   \endcode
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename> // Restricting condition
struct SchurTrait : decltype(evalSchurTrait(std::declval<T1 &>(), std::declval<T2 &>())){};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the SchurTrait class template.
// \ingroup math_traits
//
// The SchurTrait_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the SchurTrait class template. For instance, given the types \a T1 and \a T2 the following
// two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::SchurTrait<T1,T2>::Type;
   using Type2 = mtrc::numeric::SchurTrait_t<T1,T2>;
   \endcode
*/
template <typename T1, typename T2> using SchurTrait_t = typename SchurTrait<T1, T2>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the SchurTrait type trait.
// \ingroup math_traits
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename> // Restricting condition
struct SchurTraitEval1 : public SchurTraitEval2<T1, T2> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the SchurTrait type trait.
// \ingroup math_traits
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename> // Restricting condition
struct SchurTraitEval2 {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
