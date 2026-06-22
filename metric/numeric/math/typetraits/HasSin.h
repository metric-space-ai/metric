// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASSIN_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASSIN_H
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
/*!\brief Auxiliary helper struct for the HasSin type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasSinHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasSinHelper type trait for types providing the sin() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasSinHelper<T, Void_t<decltype(sin(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the sin() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the sin() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoSin {};  // Definition of a type without the sin() operation

   mtrc::numeric::HasSin< int >::value                  // Evaluates to 1
   mtrc::numeric::HasSin< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasSin< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasSin< NoSin >::value                // Evaluates to 0
   mtrc::numeric::HasSin< NoSin >::Type                 // Results in FalseType
   mtrc::numeric::HasSin< NoSin >                       // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasSin : public HasSinHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasSin type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasSin<T, EnableIf_t<IsVector_v<T>>> : public HasSin<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasSin type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasSin<T, EnableIf_t<IsMatrix_v<T>>> : public HasSin<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasSin type trait.
// \ingroup math_type_traits
//
// The HasSin_v variable template provides a convenient shortcut to access the nested \a value
// of the HasSin class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasSin<T>::value;
   constexpr bool value2 = mtrc::numeric::HasSin_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasSin_v = HasSin<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
