// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASATAN_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASATAN_H
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
/*!\brief Auxiliary helper struct for the HasAtan type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasAtanHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAtanHelper type trait for types providing the atan() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasAtanHelper<T, Void_t<decltype(atan(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the atan() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the atan() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoAtan {};  // Definition of a type without the atan() operation

   mtrc::numeric::HasAtan< int >::value                  // Evaluates to 1
   mtrc::numeric::HasAtan< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasAtan< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasAtan< NoAtan >::value               // Evaluates to 0
   mtrc::numeric::HasAtan< NoAtan >::Type                // Results in FalseType
   mtrc::numeric::HasAtan< NoAtan >                      // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasAtan : public HasAtanHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAtan type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasAtan<T, EnableIf_t<IsVector_v<T>>> : public HasAtan<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAtan type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasAtan<T, EnableIf_t<IsMatrix_v<T>>> : public HasAtan<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasAtan type trait.
// \ingroup math_type_traits
//
// The HasAtan_v variable template provides a convenient shortcut to access the nested \a value
// of the HasAtan class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasAtan<T>::value;
   constexpr bool value2 = mtrc::numeric::HasAtan_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasAtan_v = HasAtan<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
