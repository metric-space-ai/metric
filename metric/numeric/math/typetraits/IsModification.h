// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISMODIFICATION_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISMODIFICATION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/Modification.h>
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
/*!\brief Auxiliary helper functions for the IsModification type trait.
// \ingroup math_type_traits
*/
template <typename U> TrueType isModification(const volatile Modification<U> *);

FalseType isModification(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a modification expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether the given type \a Type is a modification expression
// template. In order to qualify as a valid modification expression template, the given type
// has to derive publicly from the Modification base class. In case the given type is a valid
// modification expression template, the \a value member constant is set to \a true, the nested
// type definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsModification : public decltype(isModification(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsModification type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsModification<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsModification type trait.
// \ingroup math_type_traits
//
// The IsModification_v variable template provides a convenient shortcut to access the nested
// \a value of the IsModification class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsModification<T>::value;
   constexpr bool value2 = mtrc::numeric::IsModification_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsModification_v = IsModification<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
