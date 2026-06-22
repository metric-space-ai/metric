// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASSIGN_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASSIGN_H
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
/*!\brief Auxiliary helper struct for the HasSign type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasSignHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasSignHelper type trait for types providing the sign() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasSignHelper<T, Void_t<decltype(sign(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the sign() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the sign() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoSign {};  // Definition of a type without the sign() operation

   mtrc::numeric::HasSign< int >::value                  // Evaluates to 1
   mtrc::numeric::HasSign< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasSign< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasSign< NoSign >::value               // Evaluates to 0
   mtrc::numeric::HasSign< NoSign >::Type                // Results in FalseType
   mtrc::numeric::HasSign< NoSign >                      // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasSign : public HasSignHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasSign type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasSign<T, EnableIf_t<IsVector_v<T>>> : public HasSign<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasSign type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasSign<T, EnableIf_t<IsMatrix_v<T>>> : public HasSign<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasSign type trait.
// \ingroup math_type_traits
//
// The HasSign_v variable template provides a convenient shortcut to access the nested \a value
// of the HasSign class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasSign<T>::value;
   constexpr bool value2 = mtrc::numeric::HasSign_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasSign_v = HasSign<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
