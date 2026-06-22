// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISDECLARATION_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISDECLARATION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/Declaration.h>
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
/*!\brief Auxiliary helper functions for the IsDeclaration type trait.
// \ingroup math_type_traits
*/
template <typename U> TrueType isDeclaration_backend(const volatile Declaration<U> *);

FalseType isDeclaration_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a declaration expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether the given type \a Type is a declaration expression
// template. In order to qualify as a valid declaration expression template, the given type
// has to derive publicly from the Declaration base class. In case the given type is a valid
// declaration expression template, the \a value member constant is set to \a true, the nested
// type definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsDeclaration : public decltype(isDeclaration_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsDeclaration type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsDeclaration<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsDeclaration type trait.
// \ingroup math_type_traits
//
// The IsDeclaration_v variable template provides a convenient shortcut to access the nested
// \a value of the IsDeclaration class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsDeclaration<T>::value;
   constexpr bool value2 = mtrc::numeric::IsDeclaration_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsDeclaration_v = IsDeclaration<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
