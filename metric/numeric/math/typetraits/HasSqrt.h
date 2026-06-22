// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASSQRT_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASSQRT_H
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
/*!\brief Auxiliary helper struct for the HasSqrt type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasSqrtHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasSqrtHelper type trait for types providing the sqrt() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasSqrtHelper<T, Void_t<decltype(sqrt(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the sqrt() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the sqrt() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoSqrt {};  // Definition of a type without the sqrt() operation

   mtrc::numeric::HasSqrt< int >::value                  // Evaluates to 1
   mtrc::numeric::HasSqrt< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasSqrt< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasSqrt< NoSqrt >::value               // Evaluates to 0
   mtrc::numeric::HasSqrt< NoSqrt >::Type                // Results in FalseType
   mtrc::numeric::HasSqrt< NoSqrt >                      // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasSqrt : public HasSqrtHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasSqrt type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasSqrt<T, EnableIf_t<IsVector_v<T>>> : public HasSqrt<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasSqrt type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasSqrt<T, EnableIf_t<IsMatrix_v<T>>> : public HasSqrt<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasSqrt type trait.
// \ingroup math_type_traits
//
// The HasSqrt_v variable template provides a convenient shortcut to access the nested \a value
// of the HasSqrt class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasSqrt<T>::value;
   constexpr bool value2 = mtrc::numeric::HasSqrt_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasSqrt_v = HasSqrt<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
