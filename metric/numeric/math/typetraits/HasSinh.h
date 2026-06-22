// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASSINH_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASSINH_H
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
/*!\brief Auxiliary helper struct for the HasSinh type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasSinhHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasSinhHelper type trait for types providing the sinh() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasSinhHelper<T, Void_t<decltype(sinh(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the sinh() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the sinh() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoSinh {};  // Definition of a type without the Sinh() operation

   mtrc::numeric::HasSinh< int >::value                  // Evaluates to 1
   mtrc::numeric::HasSinh< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasSinh< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasSinh< NoSinh >::value               // Evaluates to 0
   mtrc::numeric::HasSinh< NoSinh >::Type                // Results in FalseType
   mtrc::numeric::HasSinh< NoSinh >                      // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasSinh : public HasSinhHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasSinh type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasSinh<T, EnableIf_t<IsVector_v<T>>> : public HasSinh<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasSinh type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasSinh<T, EnableIf_t<IsMatrix_v<T>>> : public HasSinh<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasSinh type trait.
// \ingroup math_type_traits
//
// The HasSinh_v variable template provides a convenient shortcut to access the nested \a value
// of the HasSinh class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasSinh<T>::value;
   constexpr bool value2 = mtrc::numeric::HasSinh_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasSinh_v = HasSinh<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
