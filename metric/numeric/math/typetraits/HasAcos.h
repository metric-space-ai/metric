// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASACOS_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASACOS_H
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
/*!\brief Auxiliary helper struct for the HasAcos type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasAcosHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAcosHelper type trait for types providing the acos() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasAcosHelper<T, Void_t<decltype(acos(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the acos() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the acos() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoAcos {};  // Definition of a type without the acos() operation

   mtrc::numeric::HasAcos< int >::value                  // Evaluates to 1
   mtrc::numeric::HasAcos< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasAcos< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasAcos< NoAcos >::value               // Evaluates to 0
   mtrc::numeric::HasAcos< NoAcos >::Type                // Results in FalseType
   mtrc::numeric::HasAcos< NoAcos >                      // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasAcos : public HasAcosHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAcos type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasAcos<T, EnableIf_t<IsVector_v<T>>> : public HasAcos<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAcos type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasAcos<T, EnableIf_t<IsMatrix_v<T>>> : public HasAcos<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasAcos type trait.
// \ingroup math_type_traits
//
// The HasAcos_v variable template provides a convenient shortcut to access the nested \a value
// of the HasAcos class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasAcos<T>::value;
   constexpr bool value2 = mtrc::numeric::HasAcos_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasAcos_v = HasAcos<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
