// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASTRUNC_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASTRUNC_H
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
/*!\brief Auxiliary helper struct for the HasTrunc type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasTruncHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasTruncHelper type trait for types providing the trunc() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasTruncHelper<T, Void_t<decltype(trunc(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the trunc() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the trunc() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoTrunc {};  // Definition of a type without the trunc() operation

   mtrc::numeric::HasTrunc< int >::value                  // Evaluates to 1
   mtrc::numeric::HasTrunc< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasTrunc< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasTrunc< NoTrunc >::value              // Evaluates to 0
   mtrc::numeric::HasTrunc< NoTrunc >::Type               // Results in FalseType
   mtrc::numeric::HasTrunc< NoTrunc >                     // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasTrunc : public HasTruncHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasTrunc type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasTrunc<T, EnableIf_t<IsVector_v<T>>> : public HasTrunc<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasTrunc type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasTrunc<T, EnableIf_t<IsMatrix_v<T>>> : public HasTrunc<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasTrunc type trait.
// \ingroup math_type_traits
//
// The HasTrunc_v variable template provides a convenient shortcut to access the nested \a value
// of the HasTrunc class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasTrunc<T>::value;
   constexpr bool value2 = mtrc::numeric::HasTrunc_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasTrunc_v = HasTrunc<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
