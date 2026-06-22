// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASACOSH_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASACOSH_H
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
/*!\brief Auxiliary helper struct for the HasAcosh type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasAcoshHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAcoshHelper type trait for types providing the acosh() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasAcoshHelper<T, Void_t<decltype(acosh(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the acosh() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the acosh() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoAcosh {};  // Definition of a type without the acosh() operation

   mtrc::numeric::HasAcosh< int >::value                  // Evaluates to 1
   mtrc::numeric::HasAcosh< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasAcosh< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasAcosh< NoAcosh >::value              // Evaluates to 0
   mtrc::numeric::HasAcosh< NoAcosh >::Type               // Results in FalseType
   mtrc::numeric::HasAcosh< NoAcosh >                     // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasAcosh : public HasAcoshHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAcosh type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasAcosh<T, EnableIf_t<IsVector_v<T>>> : public HasAcosh<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAcosh type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasAcosh<T, EnableIf_t<IsMatrix_v<T>>> : public HasAcosh<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasAcosh type trait.
// \ingroup math_type_traits
//
// The HasAcosh_v variable template provides a convenient shortcut to access the nested \a value
// of the HasAcosh class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasAcosh<T>::value;
   constexpr bool value2 = mtrc::numeric::HasAcosh_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasAcosh_v = HasAcosh<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
