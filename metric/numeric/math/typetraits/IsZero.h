// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISZERO_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISZERO_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsExpression.h>
#include <metric/numeric/math/typetraits/IsStrictlyLower.h>
#include <metric/numeric/math/typetraits/IsStrictlyUpper.h>
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
template <typename T> struct IsZero;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the IsZero type trait.
// \ingroup math_traits
*/
template <typename T, typename = void>
struct IsZeroHelper : public BoolConstant<IsStrictlyLower_v<T> && IsStrictlyUpper_v<T>> {};

template <typename T> // Type of the operand
struct IsZeroHelper<T, EnableIf_t<IsExpression_v<T> && !IsSame_v<T, typename T::ResultType>>>
	: public IsZero<typename T::ResultType>::Type {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for zero vectors or matrices.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a zero vector or matrix
// type (i.e. a vector or matrix type that is guaranteed to be zero at compile time). In case the
// type is a zero vector or matrix type, the \a value member constant is set to \a true, the nested
// type definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   using DiagonalMatrixType = mtrc::numeric::DiagonalMatrix< StaticMatrix<float,3UL,3UL> >;

   mtrc::numeric::IsZero< ZeroVector<int> >::value          // Evaluates to 1
   mtrc::numeric::IsZero< const ZeroMatrix<float> >::Type   // Results in TrueType
   mtrc::numeric::IsZero< volatile ZeroMatrix<double> >     // Is derived from TrueType
   mtrc::numeric::IsZero< DynamicMatrix<int> >::value       // Evaluates to 0
   mtrc::numeric::IsZero< const DiagonalMatrixType >::Type  // Results in FalseType
   mtrc::numeric::IsZero< volatile CompressedMatrix<int> >  // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsZero : public IsZeroHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsZero type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsZero<const T> : public IsZero<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsZero type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsZero<volatile T> : public IsZero<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsZero type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsZero<const volatile T> : public IsZero<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsZero type trait.
// \ingroup math_type_traits
//
// The IsZero_v variable template provides a convenient shortcut to access the nested \a value
// of the IsZero class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsZero<T>::value;
   constexpr bool value2 = mtrc::numeric::IsZero_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsZero_v = IsZero<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
