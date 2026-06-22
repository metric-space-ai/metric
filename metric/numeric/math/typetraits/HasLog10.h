// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASLOG10_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASLOG10_H
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
/*!\brief Auxiliary helper struct for the HasLog10 type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasLog10Helper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasLog10Helper type trait for types providing the log10() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasLog10Helper<T, Void_t<decltype(log10(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the log10() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the log10() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoLog10 {};  // Definition of a type without the log10() operation

   mtrc::numeric::HasLog10< int >::value                  // Evaluates to 1
   mtrc::numeric::HasLog10< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasLog10< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasLog10< NoLog10 >::value              // Evaluates to 0
   mtrc::numeric::HasLog10< NoLog10 >::Type               // Results in FalseType
   mtrc::numeric::HasLog10< NoLog10 >                     // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasLog10 : public HasLog10Helper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasLog10 type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasLog10<T, EnableIf_t<IsVector_v<T>>> : public HasLog10<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasLog10 type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasLog10<T, EnableIf_t<IsMatrix_v<T>>> : public HasLog10<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasLog10 type trait.
// \ingroup math_type_traits
//
// The HasLog10_v variable template provides a convenient shortcut to access the nested \a value
// of the HasLog10 class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasLog10<T>::value;
   constexpr bool value2 = mtrc::numeric::HasLog10_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasLog10_v = HasLog10<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
