// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISDECLLOWEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISDECLLOWEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/DeclLowExpr.h>
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
/*!\brief Auxiliary helper functions for the IsDeclLowExpr type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isDeclLowExpr_backend(const volatile DeclLowExpr<MT> *);

FalseType isDeclLowExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a decllow expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a decllow expression
// template. In order to qualify as a valid decllow expression template, the given type has to
// derive publicly from the DeclLowExpr base class. In case the given type is a valid decllow
// expression template, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsDeclLowExpr : public decltype(isDeclLowExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsDeclLowExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsDeclLowExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsDeclLowExpr type trait.
// \ingroup math_type_traits
//
// The IsDeclLowExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsDeclLowExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsDeclLowExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsDeclLowExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsDeclLowExpr_v = IsDeclLowExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
