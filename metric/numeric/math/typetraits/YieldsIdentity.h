// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_YIELDSIDENTITY_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_YIELDSIDENTITY_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/YieldsUniLower.h>
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
// This type trait tests whether or not the given operation \a OP yields an identity matrix when
// applied to several matrices of types \a MT and \a MTs. In case the operation yields an identity
// matrix, the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename OP, typename MT, typename... MTs>
struct YieldsIdentity : public BoolConstant<YieldsUniLower_v<OP, MT, MTs...> && YieldsUniUpper_v<OP, MT, MTs...>> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the YieldsIdentity type trait for const types.
// \ingroup math_type_traits
*/
template <typename OP, typename MT, typename... MTs>
struct YieldsIdentity<const OP, MT, MTs...> : public YieldsIdentity<OP, MT, MTs...> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the YieldsIdentity type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename OP, typename MT, typename... MTs>
struct YieldsIdentity<volatile OP, MT, MTs...> : public YieldsIdentity<OP, MT, MTs...> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the YieldsIdentity type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename OP, typename MT, typename... MTs>
struct YieldsIdentity<const volatile OP, MT, MTs...> : public YieldsIdentity<OP, MT, MTs...> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the YieldsIdentity type trait.
// \ingroup math_type_traits
//
// The YieldsIdentity_v variable template provides a convenient shortcut to access the nested
// \a value of the YieldsIdentity class template. For instance, given the operation \a OP and
// the matrix type \a MT the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::YieldsIdentity<OP,MT>::value;
   constexpr bool value2 = mtrc::numeric::YieldsIdentity_v<OP,MT>;
   \endcode
*/
template <typename OP, typename MT, typename... MTs>
constexpr bool YieldsIdentity_v = YieldsIdentity<OP, MT, MTs...>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
