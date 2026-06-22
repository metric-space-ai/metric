// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASABS_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASABS_H
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
/*!\brief Auxiliary helper struct for the HasAbs type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasAbsHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAbsHelper type trait for types providing the abs() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasAbsHelper<T, Void_t<decltype(abs(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the abs() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the abs() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoAbs {};  // Definition of a type without the abs() operation

   mtrc::numeric::HasAbs< int >::value                  // Evaluates to 1
   mtrc::numeric::HasAbs< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasAbs< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasAbs< NoAbs >::value                // Evaluates to 0
   mtrc::numeric::HasAbs< NoAbs >::Type                 // Results in FalseType
   mtrc::numeric::HasAbs< NoAbs >                       // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasAbs : public HasAbsHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAbs type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasAbs<T, EnableIf_t<IsVector_v<T>>> : public HasAbs<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAbs type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasAbs<T, EnableIf_t<IsMatrix_v<T>>> : public HasAbs<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasAbs type trait.
// \ingroup math_type_traits
//
// The HasAbs_v variable template provides a convenient shortcut to access the nested \a value
// of the HasAbs class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasAbs<T>::value;
   constexpr bool value2 = mtrc::numeric::HasAbs_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasAbs_v = HasAbs<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
