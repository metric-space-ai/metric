// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISOPERATION_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISOPERATION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/Operation.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/IsBaseOf.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check whether the given type is an operational expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is either a transformation
// or a computational expression template. In order to qualify as a valid operational expression
// template, the given type has to derive (publicly or privately) from the Operation base class.
// In case the given type is a valid operational expression template, the \a value member
// constant is set to \a true, the nested type definition \a Type is \a TrueType, and the class
// derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and
// the class derives from \a FalseType.
*/
template <typename T>
struct IsOperation : public BoolConstant<IsBaseOf_v<Operation, T> && !IsBaseOf_v<T, Operation>> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsOperation type trait.
// \ingroup math_type_traits
//
// The IsOperation_v variable template provides a convenient shortcut to access the nested
// \a value of the IsOperation class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsOperation<T>::value;
   constexpr bool value2 = mtrc::numeric::IsOperation_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsOperation_v = IsOperation<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
