// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISUNIFORM_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISUNIFORM_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsExpression.h>
#include <metric/numeric/math/typetraits/IsZero.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/IsSame.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> struct IsUniform;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the IsUniform type trait.
// \ingroup math_traits
*/
template <typename T, typename = void> struct IsUniformHelper : public IsZero<T> {};

template <typename T> // Type of the operand
struct IsUniformHelper<T, EnableIf_t<IsExpression_v<T> && !IsSame_v<T, typename T::ResultType>>>
	: public IsUniform<typename T::ResultType>::Type {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for uniform vectors and matrices.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a uniform vector or
// matrix type (i.e. a data type that is guaranteed to be a uniform vector or matrix at compile
// time). In case the type is a uniform vector or matrix type, the \a value member constant is
// set to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.

   \code
   mtrc::numeric::IsUniform< UniformVector<int> >::value          // Evaluates to 1
   mtrc::numeric::IsUniform< const UniformMatrix<float> >::Type   // Results in TrueType
   mtrc::numeric::IsUniform< volatile UniformVector<double> >     // Is derived from TrueType
   mtrc::numeric::IsUniform< DynamicVector<int> >::value          // Evaluates to 0
   mtrc::numeric::IsUniform< const DynamicMatrix<float> >::Type   // Results in FalseType
   mtrc::numeric::IsUniform< volatile CompressedVector<double> >  // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsUniform : public IsUniformHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsUniform type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsUniform<const T> : public IsUniform<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsUniform type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsUniform<volatile T> : public IsUniform<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsUniform type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsUniform<const volatile T> : public IsUniform<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsUniform type trait.
// \ingroup math_type_traits
//
// The IsUniform_v variable template provides a convenient shortcut to access the nested
// \a value of the IsUniform class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsUniform<T>::value;
   constexpr bool value2 = mtrc::numeric::IsUniform_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsUniform_v = IsUniform<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
