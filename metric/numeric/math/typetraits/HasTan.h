// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASTAN_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASTAN_H
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
/*!\brief Auxiliary helper struct for the HasTan type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasTanHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasTanHelper type trait for types providing the tan() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasTanHelper<T, Void_t<decltype(tan(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the tan() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the tan() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoTan {};  // Definition of a type without the tan() operation

   mtrc::numeric::HasTan< int >::value                  // Evaluates to 1
   mtrc::numeric::HasTan< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasTan< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasTan< NoTan >::value                // Evaluates to 0
   mtrc::numeric::HasTan< NoTan >::Type                 // Results in FalseType
   mtrc::numeric::HasTan< NoTan >                       // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasTan : public HasTanHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasTan type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasTan<T, EnableIf_t<IsVector_v<T>>> : public HasTan<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasTan type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasTan<T, EnableIf_t<IsMatrix_v<T>>> : public HasTan<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasTan type trait.
// \ingroup math_type_traits
//
// The HasTan_v variable template provides a convenient shortcut to access the nested \a value
// of the HasTan class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasTan<T>::value;
   constexpr bool value2 = mtrc::numeric::HasTan_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasTan_v = HasTan<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
