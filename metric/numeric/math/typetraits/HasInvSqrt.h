// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASINVSQRT_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASINVSQRT_H
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
/*!\brief Auxiliary helper struct for the HasInvSqrt type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasInvSqrtHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasInvSqrtHelper type trait for types providing the invsqrt()
//        operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasInvSqrtHelper<T, Void_t<decltype(invsqrt(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the invsqrt() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the invsqrt() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoInvSqrt {};  // Definition of a type without the invsqrt() operation

   mtrc::numeric::HasInvSqrt< int >::value                  // Evaluates to 1
   mtrc::numeric::HasInvSqrt< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasInvSqrt< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasInvSqrt< NoInvSqrt >::value            // Evaluates to 0
   mtrc::numeric::HasInvSqrt< NoInvSqrt >::Type             // Results in FalseType
   mtrc::numeric::HasInvSqrt< NoInvSqrt >                   // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasInvSqrt : public HasInvSqrtHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasInvSqrt type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasInvSqrt<T, EnableIf_t<IsVector_v<T>>> : public HasInvSqrt<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasInvSqrt type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasInvSqrt<T, EnableIf_t<IsMatrix_v<T>>> : public HasInvSqrt<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasInvSqrt type trait.
// \ingroup math_type_traits
//
// The HasInvSqrt_v variable template provides a convenient shortcut to access the nested \a value
// of the HasInvSqrt class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasInvSqrt<T>::value;
   constexpr bool value2 = mtrc::numeric::HasInvSqrt_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasInvSqrt_v = HasInvSqrt<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
