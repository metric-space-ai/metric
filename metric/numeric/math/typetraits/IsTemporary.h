// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISTEMPORARY_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISTEMPORARY_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsExpression.h>
#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/IsVector.h>
#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a temporary vector or matrix type.
// \ingroup math_type_traits
//
// This type trait class tests whether the given type is a temporary vector or matrix type,
// i.e. can be used for a temporary vector or matrix. In case the given type can be used as
// temporary, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set
// to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T>
struct IsTemporary : public BoolConstant<(IsVector_v<T> || IsMatrix_v<T>) && !IsExpression_v<T>> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsTemporary type trait.
// \ingroup math_type_traits
//
// The IsTemporary_v variable template provides a convenient shortcut to access the nested
// \a value of the IsTemporary class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsTemporary<T>::value;
   constexpr bool value2 = mtrc::numeric::IsTemporary_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsTemporary_v = IsTemporary<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
