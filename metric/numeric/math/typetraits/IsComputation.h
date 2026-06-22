// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISCOMPUTATION_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISCOMPUTATION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/Computation.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/IsBaseOf.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a computational expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a computational
// expression template (i.e. a mathematical operation such as an addition, a subtraction,
// a multiplication, a division, an absolute value calculation, ...). In order to qualify
// as a valid computational expression template, the given type has to derive (publicly or
// privately) from the Computation base class. In case the given type is a valid computational
// expression template, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T>
struct IsComputation : public BoolConstant<IsBaseOf_v<Computation, T> && !IsBaseOf_v<T, Computation>> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsComputation type trait.
// \ingroup math_type_traits
//
// The IsComputation_v variable template provides a convenient shortcut to access the nested
// \a value of the IsComputation class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsComputation<T>::value;
   constexpr bool value2 = mtrc::numeric::IsComputation_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsComputation_v = IsComputation<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
