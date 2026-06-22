// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_VALUETRAITS_ISMULTIPLEOF_H
#define METRIC_NUMERIC_UTIL_VALUETRAITS_ISMULTIPLEOF_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for a multiplicative relationship of two integral values.
// \ingroup value_traits
//
// This value trait tests whether the first given integral value \a M is a multiple of the second
// integral value \a N (i.e. if \f$ M = x*N \f$, where x is any positive integer in the range
// \f$ [0..\infty) \f$). In case the value is a multiple of \a N, the \a value member enumeration
// is set to \a true, the nested type definition \a Type is \a TrueType, and the class derives
// from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the
// class derives from \a FalseType.

   \code
   mtrc::numeric::IsMultipleOf<8,2>::value  // Evaluates to 1 (x*2 = 8 for x = 4)
   mtrc::numeric::IsMultipleOf<2,2>::value  // Evaluates to 1 (x*2 = 2 for x = 1)
   mtrc::numeric::IsMultipleOf<0,2>::Type   // Results in TrueType (x*2 = 0 for x = 0)
   mtrc::numeric::IsMultipleOf<0,0>         // Is derived from TrueType (x*0 = 0 for any x)

   mtrc::numeric::IsMultipleOf<5,3>::value  // Evaluates to 0 (5 is no integral multiple of 3)
   mtrc::numeric::IsMultipleOf<2,3>::Type   // Results in TrueType (2 is no integral multiple of 3)
   mtrc::numeric::IsMultipleOf<2,0>         // Is derived from TrueType (2 is no multiple of 0)
   \endcode
*/
template <size_t M, size_t N> struct IsMultipleOf : public BoolConstant<M % N == 0UL> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Partial specialization of the IsMultipleOf value trait for M > 0 and N = 0.
// \ingroup type_traits
*/
template <size_t M> struct IsMultipleOf<M, 0UL> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Partial specialization of the IsMultipleOf value trait for M = 0 and N = 0.
// \ingroup type_traits
*/
template <> struct IsMultipleOf<0, 0> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsMultipleOf value trait.
// \ingroup value_traits
//
// The IsMultipleOf_v variable template provides a convenient shortcut to access the nested
// \a value of the IsMultipleOf class template. For instance, given the compile time constant
// values \a M and \a N the following two statements are identical:

   \code
   constexpr bool value1 = IsMultipleOf<M,N>::value;
   constexpr bool value2 = IsMultipleOf_v<M,N>;
   \endcode
*/
template <size_t M, size_t N> constexpr bool IsMultipleOf_v = IsMultipleOf<M, N>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
