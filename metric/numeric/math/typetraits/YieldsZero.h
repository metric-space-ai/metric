// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_YIELDSZERO_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_YIELDSZERO_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for operations on vectors and matrices.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given operation \a OP yields a zero vector or zero
// matrix when applied to several vectors or matrices of types \a T and \a Ts. In case the
// operation yields a zero vector or matrix, the \a value member constant is set to \a true,
// the nested type definition \a Type is \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class derives from
// \a FalseType.
*/
template <typename OP, typename T, typename... Ts> struct YieldsZero : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the YieldsZero type trait for const types.
// \ingroup math_type_traits
*/
template <typename OP, typename T, typename... Ts>
struct YieldsZero<const OP, T, Ts...> : public YieldsZero<OP, T, Ts...> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the YieldsZero type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename OP, typename T, typename... Ts>
struct YieldsZero<volatile OP, T, Ts...> : public YieldsZero<OP, T, Ts...> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the YieldsZero type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename OP, typename T, typename... Ts>
struct YieldsZero<const volatile OP, T, Ts...> : public YieldsZero<OP, T, Ts...> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the YieldsZero type trait.
// \ingroup math_type_traits
//
// The YieldsZero_v variable template provides a convenient shortcut to access the nested
// \a value of the YieldsZero class template. For instance, given the operation \a OP and
// the vector or matrix type \a T the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::YieldsZero<OP,T>::value;
   constexpr bool value2 = mtrc::numeric::YieldsZero_v<OP,T>;
   \endcode
*/
template <typename OP, typename T, typename... Ts> constexpr bool YieldsZero_v = YieldsZero<OP, T, Ts...>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
