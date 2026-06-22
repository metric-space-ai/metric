// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASINVCBRT_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASINVCBRT_H
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
/*!\brief Auxiliary helper struct for the HasInvCbrt type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasInvCbrtHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasInvCbrtHelper type trait for types providing the invcbrt()
//        operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasInvCbrtHelper<T, Void_t<decltype(invcbrt(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the invcbrt() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the invcbrt() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoInvCbrt {};  // Definition of a type without the invcbrt() operation

   mtrc::numeric::HasInvCbrt< int >::value                  // Evaluates to 1
   mtrc::numeric::HasInvCbrt< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasInvCbrt< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasInvCbrt< NoInvCbrt >::value            // Evaluates to 0
   mtrc::numeric::HasInvCbrt< NoInvCbrt >::Type             // Results in FalseType
   mtrc::numeric::HasInvCbrt< NoInvCbrt >                   // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasInvCbrt : public HasInvCbrtHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasInvCbrt type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasInvCbrt<T, EnableIf_t<IsVector_v<T>>> : public HasInvCbrt<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasInvCbrt type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasInvCbrt<T, EnableIf_t<IsMatrix_v<T>>> : public HasInvCbrt<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasInvCbrt type trait.
// \ingroup math_type_traits
//
// The HasInvCbrt_v variable template provides a convenient shortcut to access the nested \a value
// of the HasInvCbrt class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasInvCbrt<T>::value;
   constexpr bool value2 = mtrc::numeric::HasInvCbrt_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasInvCbrt_v = HasInvCbrt<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
