// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_SOLVETRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_SOLVETRAIT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/InvalidType.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename, typename, typename = void> struct SolveTrait;
template <typename, typename, typename = void> struct SolveTraitEval1;
template <typename, typename, typename = void> struct SolveTraitEval2;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2>
auto evalSolveTrait(const volatile T1 &, const volatile T2 &) -> SolveTraitEval1<T1, T2>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the SolveTrait class.
// \ingroup math_traits
//
// \section solvetrait_general General
//
// The SolveTrait class template offers the possibility to select the resulting data type for
// solving a linear system of equations (LSE) with the two given types \a T1 and \a T2 via the
// \a solve() operation. SolveTrait defines the nested type \a Type, which represents the
// resulting data type of the \a solve() operation. In case \a T1 and \a T2 cannot be combined
// in a \a solve() operation, there is no nested type \a Type. Note that \a const and \a volatile
// qualifiers and reference modifiers are generally ignored.
//
//
// \n \section solvetrait_specializations Creating custom specializations
//
// Per default, SolveTrait supports all vector and matrix types of the Metric numeric library (including
// views and adaptors). For all other data types it is possible to specialize the SolveTrait
// template. The following example shows the according specialization for solving an LSE with a
// static matrix and a static vector:

   \code
   template< typename T, size_t N, bool SO, bool TF >
   struct SolveTrait< StaticMatrix<T,N,N,SO>, StaticVector<T,N,TF> >
   {
	  using Type = StaticVector<T,N,TF>;
   };
   \endcode

// \n \section solvetrait_examples Examples
//
// The following example demonstrates the use of the SolveTrait template, where depending on
// the two given data types the resulting data type is selected:

   \code
   template< typename T1, typename T2 >    // The two generic types
   typename SolveTrait<T1,T2>::Type        // The resulting generic return type
   solveLSE( const T1& t1, const T2& t2 )  //
   {                                       // The function 'solveLSE' returns the solution
	  return solve( t1, t2 );              // for the LSE represented by the two given values
   }                                       //
   \endcode
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename> // Restricting condition
struct SolveTrait : public decltype(evalSolveTrait(std::declval<T1 &>(), std::declval<T2 &>())){};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the SolveTrait class template.
// \ingroup math_traits
//
// The SolveTrait_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the SolveTrait class template. For instance, given the types \a T1 and \a T2 the following
// two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::SolveTrait<T1,T2>::Type;
   using Type2 = mtrc::numeric::SolveTrait_t<T1,T2>;
   \endcode
*/
template <typename T1, typename T2> using SolveTrait_t = typename SolveTrait<T1, T2>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the SolveTrait type trait.
// \ingroup math_traits
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename> // Restricting condition
struct SolveTraitEval1 : public SolveTraitEval2<T1, T2> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the SolveTrait type trait.
// \ingroup math_traits
*/
template <typename T1 // Type of the left-hand side operand
		  ,
		  typename T2 // Type of the right-hand side operand
		  ,
		  typename> // Restricting condition
struct SolveTraitEval2 {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
