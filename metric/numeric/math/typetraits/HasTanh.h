// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASTANH_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASTANH_H
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
/*!\brief Auxiliary helper struct for the HasTanh type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasTanhHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasTanhHelper type trait for types providing the tanh() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasTanhHelper<T, Void_t<decltype(tanh(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the tanh() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the tanh() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoTanh {};  // Definition of a type without the tanh() operation

   mtrc::numeric::HasTanh< int >::value                  // Evaluates to 1
   mtrc::numeric::HasTanh< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasTanh< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasTanh< NoTanh >::value               // Evaluates to 0
   mtrc::numeric::HasTanh< NoTanh >::Type                // Results in FalseType
   mtrc::numeric::HasTanh< NoTanh >                      // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasTanh : public HasTanhHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasTanh type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasTanh<T, EnableIf_t<IsVector_v<T>>> : public HasTanh<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasTanh type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasTanh<T, EnableIf_t<IsMatrix_v<T>>> : public HasTanh<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasTanh type trait.
// \ingroup math_type_traits
//
// The HasTanh_v variable template provides a convenient shortcut to access the nested \a value
// of the HasTanh class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasTanh<T>::value;
   constexpr bool value2 = mtrc::numeric::HasTanh_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasTanh_v = HasTanh<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
