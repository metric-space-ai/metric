// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASROUND_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASROUND_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/IsVector.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/IntegralConstant.h>
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
/*!\brief Auxiliary helper struct for the HasRound type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasRoundHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasRoundHelper type trait for types providing the round() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasRoundHelper<T, Void_t<decltype(round(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the round() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the round() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoRound {};  // Definition of a type without the round() operation

   mtrc::numeric::HasRound< int >::value                  // Evaluates to 1
   mtrc::numeric::HasRound< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasRound< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasRound< NoRound >::value              // Evaluates to 0
   mtrc::numeric::HasRound< NoRound >::Type               // Results in FalseType
   mtrc::numeric::HasRound< NoRound >                     // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasRound : public HasRoundHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasRound type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasRound<T, EnableIf_t<IsVector_v<T>>> : public HasRound<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasRound type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasRound<T, EnableIf_t<IsMatrix_v<T>>> : public HasRound<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasRound type trait.
// \ingroup math_type_traits
//
// The HasRound_v variable template provides a convenient shortcut to access the nested \a value
// of the HasRound class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasRound<T>::value;
   constexpr bool value2 = mtrc::numeric::HasRound_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasRound_v = HasRound<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
