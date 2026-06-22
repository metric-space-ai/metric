// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASERFC_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASERFC_H
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
/*!\brief Auxiliary helper struct for the HasErfc type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasErfcHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasErfcHelper type trait for types providing the erfc() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasErfcHelper<T, Void_t<decltype(erfc(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the erfc() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the erfc() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoErfc {};  // Definition of a type without the erfc() operation

   mtrc::numeric::HasErfc< int >::value                  // Evaluates to 1
   mtrc::numeric::HasErfc< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasErfc< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasErfc< NoErfc >::value               // Evaluates to 0
   mtrc::numeric::HasErfc< NoErfc >::Type                // Results in FalseType
   mtrc::numeric::HasErfc< NoErfc >                      // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasErfc : public HasErfcHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasErfc type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasErfc<T, EnableIf_t<IsVector_v<T>>> : public HasErfc<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasErfc type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasErfc<T, EnableIf_t<IsMatrix_v<T>>> : public HasErfc<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasErfc type trait.
// \ingroup math_type_traits
//
// The HasErfc_v variable template provides a convenient shortcut to access the nested \a value
// of the HasErfc class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasErfc<T>::value;
   constexpr bool value2 = mtrc::numeric::HasErfc_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasErfc_v = HasErfc<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
