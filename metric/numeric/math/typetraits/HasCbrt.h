// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASCBRT_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASCBRT_H
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
/*!\brief Auxiliary helper struct for the HasCbrt type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasCbrtHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasCbrtHelper type trait for types providing the cbrt() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasCbrtHelper<T, Void_t<decltype(cbrt(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the cbrt() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the cbrt() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoCbrt {};  // Definition of a type without the cbrt() operation

   mtrc::numeric::HasCbrt< int >::value                  // Evaluates to 1
   mtrc::numeric::HasCbrt< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasCbrt< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasCbrt< NoCbrt >::value               // Evaluates to 0
   mtrc::numeric::HasCbrt< NoCbrt >::Type                // Results in FalseType
   mtrc::numeric::HasCbrt< NoCbrt >                      // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasCbrt : public HasCbrtHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasCbrt type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasCbrt<T, EnableIf_t<IsVector_v<T>>> : public HasCbrt<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasCbrt type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasCbrt<T, EnableIf_t<IsMatrix_v<T>>> : public HasCbrt<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasCbrt type trait.
// \ingroup math_type_traits
//
// The HasCbrt_v variable template provides a convenient shortcut to access the nested \a value
// of the HasCbrt class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasCbrt<T>::value;
   constexpr bool value2 = mtrc::numeric::HasCbrt_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasCbrt_v = HasCbrt<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
