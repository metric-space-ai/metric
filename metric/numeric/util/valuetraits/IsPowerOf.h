// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_VALUETRAITS_ISPOWEROF_H
#define METRIC_NUMERIC_UTIL_VALUETRAITS_ISPOWEROF_H
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
/*!\brief Compile time check for a power relationship of integral values to a given base.
// \ingroup value_traits
//
// This value trait tests whether the given integral value \a N is a power of the base \a B
// according to the equation \f$ B^x = N \f$, where x is any positive integer in the range
// \f$ [0..\infty) \f$. In case the value is a power of \a B, the \a value member enumeration
// is set to \a true, the nested type definition \a Type is \a TrueType, and the class derives
// from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the
// class derives from \a FalseType.

   \code
   mtrc::numeric::IsPowerOf<2,8>::value   // Evaluates to 1 (2^3 = 8)
   mtrc::numeric::IsPowerOf<3,27>::value  // Evaluates to 1 (3^3 = 27)
   mtrc::numeric::IsPowerOf<5,1>::value   // Evaluates to 1 (5^0 = 1)
   mtrc::numeric::IsPowerOf<1,1>::Type    // Results in TrueType (1^x = 1)
   mtrc::numeric::IsPowerOf<0,0>          // Is derived from TrueType (0^x = 0)
   mtrc::numeric::IsPowerOf<2,14>::value  // Evaluates to 0
   mtrc::numeric::IsPowerOf<1,5>::value   // Evaluates to 0
   mtrc::numeric::IsPowerOf<0,5>::Type    // Results in FalseType
   mtrc::numeric::IsPowerOf<2,0>          // Is derived from FalseType
   \endcode
*/
template <size_t B, size_t N> struct IsPowerOf : public BoolConstant<IsPowerOf<B, N / B>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Partial specialization of the IsPowerOf value trait.
// \ingroup value_traits
//
// This class ia a partial specialization of the IsPowerOf value trait for any value \a N to
// the base 2. In case \a N is a power of 2, the \a value member enumeration is set to 1, the
// nested type definition \a Type is \a TrueType, and the class derives from \a TrueType. If
// not, \a value is set to 0, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <size_t N> struct IsPowerOf<2, N> : public BoolConstant<(N & (N - 1)) == 0UL> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Partial specialization of the IsPowerOf value trait.
// \ingroup value_traits
//
// This class ia a partial specialization of the IsPowerOf value trait for the value 0 to the
// base 2. Since 0 is no power of 2, this specialization sets the \a value member enumeration
// to 0, the nested type definition \a Type to \a FalseType, and it derives from \a FalseType.
*/
template <> struct IsPowerOf<2, 0> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Partial specialization of the IsPowerOf value trait.
// \ingroup value_traits
//
// This class ia a partial specialization of the IsPowerOf value trait for the value 1 to any
// given base \a B larger than 1. According to the equation \f$ B^0 = 1 \f$ this specialization
// always sets the \a value member enumeration to 1, the nested type definition \a Type to
// \a TrueType, and it derives from \a TrueType.
*/
template <size_t B> struct IsPowerOf<B, 1> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Partial specialization of the IsPowerOf value trait.
// \ingroup value_traits
//
// This class ia a partial specialization of the IsPowerOf value trait for any value \a N larger
// than 1 to the base 1. Since N is no power of 1, this specialization always sets the \a value
// member enumeration to 0, the nested type definition \a Type to \a FalseType, and it derives
// from \a FalseType.
*/
template <size_t N> struct IsPowerOf<1, N> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Partial specialization of the IsPowerOf value trait.
// \ingroup value_traits
//
// This class ia a partial specialization of the IsPowerOf value trait for the value 1 to
// the base 1. Since 1 is a power of 1, this specialization always sets the \a value member
// enumeration to 1, the nested type definition \a Type to \a TrueType, and it derives from
// \a TrueType.
*/
template <> struct IsPowerOf<1, 1> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Partial specialization of the IsPowerOf value trait.
// \ingroup value_traits
//
// This class ia a partial specialization of the IsPowerOf value trait for the value 0 to the
// base \a B. Since 0 is no power of \a B, this specialization always sets the \a value member
// enumeration to 0, the nested type definition \a Type to \a FalseType, and it derives from
// \a FalseType.
*/
template <size_t B> struct IsPowerOf<B, 0> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Partial specialization of the IsPowerOf value trait.
// \ingroup value_traits
//
// This class ia a partial specialization of the IsPowerOf value trait for any value \a N to
// the base 0. Since N is no power of 0, this specialization always sets the \a value member
// enumeration to 0, the nested type definition \a Type to \a FalseType, and it derives from
// \a FalseType.
*/
template <size_t N> struct IsPowerOf<0, N> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Partial specialization of the IsPowerOf value trait.
// \ingroup value_traits
//
// This class ia a partial specialization of the IsPowerOf value trait for the value 0 to
// the base 0. Since 0 is a power of 0 (\f$ 0^x = 0 \f$), this specialization always sets
// the \a value member enumeration to 1, the nested type definition \a Type to \a TrueType,
// and it derives from \a TrueType.
*/
template <> struct IsPowerOf<0, 0> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsPowerOf value trait.
// \ingroup value_traits
//
// The IsPowerOf_v variable template provides a convenient shortcut to access the nested \a value
// of the IsPowerOf class template. For instance, given the compile time constant values \a B and
// \a N the following two statements are identical:

   \code
   constexpr bool value1 = IsPowerOf<B,N>::value;
   constexpr bool value2 = IsPowerOf_v<B,N>;
   \endcode
*/
template <size_t B, size_t N> constexpr bool IsPowerOf_v = IsPowerOf<B, N>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
