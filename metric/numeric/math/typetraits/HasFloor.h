// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASFLOOR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASFLOOR_H
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
/*!\brief Auxiliary helper struct for the HasFloor type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasFloorHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasFloorHelper type trait for types providing the floor() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasFloorHelper<T, Void_t<decltype(floor(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the floor() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the floor() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoFloor {};  // Definition of a type without the floor() operation

   mtrc::numeric::HasFloor< int >::value                  // Evaluates to 1
   mtrc::numeric::HasFloor< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasFloor< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasFloor< NoFloor >::value              // Evaluates to 0
   mtrc::numeric::HasFloor< NoFloor >::Type               // Results in FalseType
   mtrc::numeric::HasFloor< NoFloor >                     // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasFloor : public HasFloorHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasFloor type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasFloor<T, EnableIf_t<IsVector_v<T>>> : public HasFloor<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasFloor type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasFloor<T, EnableIf_t<IsMatrix_v<T>>> : public HasFloor<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasFloor type trait.
// \ingroup math_type_traits
//
// The HasFloor_v variable template provides a convenient shortcut to access the nested \a value
// of the HasFloor class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasFloor<T>::value;
   constexpr bool value2 = mtrc::numeric::HasFloor_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasFloor_v = HasFloor<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
