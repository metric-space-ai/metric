// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASERF_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASERF_H
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
/*!\brief Auxiliary helper struct for the HasErf type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasErfHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasErfHelper type trait for types providing the erf() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasErfHelper<T, Void_t<decltype(erf(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the erf() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the erf() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoErf {};  // Definition of a type without the erf() operation

   mtrc::numeric::HasErf< int >::value                  // Evaluates to 1
   mtrc::numeric::HasErf< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasErf< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasErf< NoErf >::value                // Evaluates to 0
   mtrc::numeric::HasErf< NoErf >::Type                 // Results in FalseType
   mtrc::numeric::HasErf< NoErf >                       // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasErf : public HasErfHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasErf type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasErf<T, EnableIf_t<IsVector_v<T>>> : public HasErf<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasErf type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasErf<T, EnableIf_t<IsMatrix_v<T>>> : public HasErf<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasErf type trait.
// \ingroup math_type_traits
//
// The HasErf_v variable template provides a convenient shortcut to access the nested \a value
// of the HasErf class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasErf<T>::value;
   constexpr bool value2 = mtrc::numeric::HasErf_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasErf_v = HasErf<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
