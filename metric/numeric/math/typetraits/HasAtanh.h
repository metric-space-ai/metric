// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASATANH_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASATANH_H
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
/*!\brief Auxiliary helper struct for the HasAtanh type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasAtanhHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAtanhHelper type trait for types providing the atanh() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasAtanhHelper<T, Void_t<decltype(atanh(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the atanh() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the atanh() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoAtanh {};  // Definition of a type without the atanh() operation

   mtrc::numeric::HasAtanh< int >::value                  // Evaluates to 1
   mtrc::numeric::HasAtanh< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasAtanh< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasAtanh< NoAtanh >::value              // Evaluates to 0
   mtrc::numeric::HasAtanh< NoAtanh >::Type               // Results in FalseType
   mtrc::numeric::HasAtanh< NoAtanh >                     // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasAtanh : public HasAtanhHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAtanh type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasAtanh<T, EnableIf_t<IsVector_v<T>>> : public HasAtanh<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAtanh type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasAtanh<T, EnableIf_t<IsMatrix_v<T>>> : public HasAtanh<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasAtanh type trait.
// \ingroup math_type_traits
//
// The HasAtanh_v variable template provides a convenient shortcut to access the nested \a value
// of the HasAtanh class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasAtanh<T>::value;
   constexpr bool value2 = mtrc::numeric::HasAtanh_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasAtanh_v = HasAtanh<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
