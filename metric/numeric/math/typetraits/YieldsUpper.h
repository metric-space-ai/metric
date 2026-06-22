// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_YIELDSUPPER_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_YIELDSUPPER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/YieldsStrictlyUpper.h>
#include <metric/numeric/math/typetraits/YieldsUniUpper.h>
#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for operations on matrices.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given operation \a OP yields an upper matrix when
// applied to several matrices of types \a MT and \a MTs. In case the operation yields an upper
// matrix, the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename OP, typename MT, typename... MTs>
struct YieldsUpper : public BoolConstant<YieldsUniUpper_v<OP, MT, MTs...> || YieldsStrictlyUpper_v<OP, MT, MTs...>> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the YieldsUpper type trait for const types.
// \ingroup math_type_traits
*/
template <typename OP, typename MT, typename... MTs>
struct YieldsUpper<const OP, MT, MTs...> : public YieldsUpper<OP, MT, MTs...> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the YieldsUpper type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename OP, typename MT, typename... MTs>
struct YieldsUpper<volatile OP, MT, MTs...> : public YieldsUpper<OP, MT, MTs...> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the YieldsUpper type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename OP, typename MT, typename... MTs>
struct YieldsUpper<const volatile OP, MT, MTs...> : public YieldsUpper<OP, MT, MTs...> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the YieldsUpper type trait.
// \ingroup math_type_traits
//
// The YieldsUpper_v variable template provides a convenient shortcut to access the nested
// \a value of the YieldsUpper class template. For instance, given the operation \a OP and
// the matrix type \a MT the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::YieldsUpper<OP,MT>::value;
   constexpr bool value2 = mtrc::numeric::YieldsUpper_v<OP,MT>;
   \endcode
*/
template <typename OP, typename MT, typename... MTs> constexpr bool YieldsUpper_v = YieldsUpper<OP, MT, MTs...>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
