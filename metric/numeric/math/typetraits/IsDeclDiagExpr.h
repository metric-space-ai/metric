// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISDECLDIAGEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISDECLDIAGEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/DeclDiagExpr.h>
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
/*!\brief Auxiliary helper functions for the IsDeclDiagExpr type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isDeclDiagExpr_backend(const volatile DeclDiagExpr<MT> *);

FalseType isDeclDiagExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a decldiag expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a decldiag expression
// template. In order to qualify as a valid decldiag expression template, the given type has to
// derive publicly from the DeclDiagExpr base class. In case the given type is a valid decldiag
// expression template, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsDeclDiagExpr : public decltype(isDeclDiagExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsDeclDiagExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsDeclDiagExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsDeclDiagExpr type trait.
// \ingroup math_type_traits
//
// The IsDeclDiagExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsDeclDiagExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsDeclDiagExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsDeclDiagExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsDeclDiagExpr_v = IsDeclDiagExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
