// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISEIGENEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISEIGENEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/EigenExpr.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper functions for the IsEigenExpr type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isEigenExpr_backend(const volatile EigenExpr<MT> *);

FalseType isEigenExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is an eigenvalue expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is an eigenvalue expression
// template. In order to qualify as a valid eigenvalue expression template, the given type has
// to derive publicly from the EigenExpr base class. In case the given type is a valid eigenvalue
// expression template, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsEigenExpr : public decltype(isEigenExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsEigenExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsEigenExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsEigenExpr type trait.
// \ingroup math_type_traits
//
// The IsEigenExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsEigenExpr class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsEigenExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsEigenExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsEigenExpr_v = IsEigenExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
