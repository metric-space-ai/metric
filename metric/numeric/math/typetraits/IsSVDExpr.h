// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSVDEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSVDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/SVDExpr.h>
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
/*!\brief Auxiliary helper functions for the IsSVDExpr type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isSVDExpr_backend(const volatile SVDExpr<MT> *);

FalseType isSVDExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a singular value expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a singular value expression
// template. In order to qualify as a valid singular value expression template, the given type has
// to derive publicly from the SVDExpr base class. In case the given type is a valid singular value
// expression template, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsSVDExpr : public decltype(isSVDExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSVDExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsSVDExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsSVDExpr type trait.
// \ingroup math_type_traits
//
// The IsSVDExpr_v variable template provides a convenient shortcut to access the nested \a value
// of the IsSVDExpr class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsSVDExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsSVDExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsSVDExpr_v = IsSVDExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
