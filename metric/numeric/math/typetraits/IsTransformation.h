// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISTRANSFORMATION_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISTRANSFORMATION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/Transformation.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/IsBaseOf.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a transformation expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether the given type \a Type is a transformation expression
// template (e.g. a transpose operation). In order to qualify as a valid transformation expression
// template, the given type has to derive (publicly or privately) from the Transformation base
// class. In case the given type is a valid transformation expression template, the \a value
// member constant is set to \a true, the nested type definition \a Type is \a TrueType, and
// the class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType, and the class derives from \a FalseType.
*/
template <typename T>
struct IsTransformation : public BoolConstant<IsBaseOf_v<Transformation, T> && !IsBaseOf_v<T, Transformation>> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsTransformation type trait.
// \ingroup math_type_traits
//
// The IsTransformation_v variable template provides a convenient shortcut to access the nested
// \a value of the IsTransformation class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsTransformation<T>::value;
   constexpr bool value2 = mtrc::numeric::IsTransformation_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsTransformation_v = IsTransformation<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
