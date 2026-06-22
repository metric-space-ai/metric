// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASASINH_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASASINH_H
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
/*!\brief Auxiliary helper struct for the HasAsinh type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasAsinhHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAsinhHelper type trait for types providing the asinh() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasAsinhHelper<T, Void_t<decltype(asinh(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the asinh() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the asinh() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoAsinh {};  // Definition of a type without the asinh() operation

   mtrc::numeric::HasAsinh< int >::value                  // Evaluates to 1
   mtrc::numeric::HasAsinh< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasAsinh< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasAsinh< NoAsinh >::value              // Evaluates to 0
   mtrc::numeric::HasAsinh< NoAsinh >::Type               // Results in FalseType
   mtrc::numeric::HasAsinh< NoAsinh >                     // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasAsinh : public HasAsinhHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAsinh type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasAsinh<T, EnableIf_t<IsVector_v<T>>> : public HasAsinh<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAsinh type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasAsinh<T, EnableIf_t<IsMatrix_v<T>>> : public HasAsinh<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasAsinh type trait.
// \ingroup math_type_traits
//
// The HasAsinh_v variable template provides a convenient shortcut to access the nested \a value
// of the HasAsinh class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasAsinh<T>::value;
   constexpr bool value2 = mtrc::numeric::HasAsinh_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasAsinh_v = HasAsinh<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
