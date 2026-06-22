// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASCOS_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASCOS_H
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
/*!\brief Auxiliary helper struct for the HasCos type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasCosHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasCosHelper type trait for types providing the cos() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasCosHelper<T, Void_t<decltype(cos(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the cos() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the cos() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoCos {};  // Definition of a type without the cos() operation

   mtrc::numeric::HasCos< int >::value                  // Evaluates to 1
   mtrc::numeric::HasCos< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasCos< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasCos< NoCos >::value                // Evaluates to 0
   mtrc::numeric::HasCos< NoCos >::Type                 // Results in FalseType
   mtrc::numeric::HasCos< NoCos >                       // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasCos : public HasCosHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasCos type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasCos<T, EnableIf_t<IsVector_v<T>>> : public HasCos<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasCos type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasCos<T, EnableIf_t<IsMatrix_v<T>>> : public HasCos<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasCos type trait.
// \ingroup math_type_traits
//
// The HasCos_v variable template provides a convenient shortcut to access the nested \a value
// of the HasCos class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasCos<T>::value;
   constexpr bool value2 = mtrc::numeric::HasCos_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasCos_v = HasCos<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
