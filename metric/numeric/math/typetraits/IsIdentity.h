// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISIDENTITY_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISIDENTITY_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsUniLower.h>
#include <metric/numeric/math/typetraits/IsUniUpper.h>
#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for identity matrices.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is an identity matrix type
// (i.e. a matrix type that is guaranteed to be an identity matrix at compile time). In case the
// type is an identity matrix type, the \a value member constant is set to \a true, the nested
// type definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   using DiagonalMatrixType = mtrc::numeric::DiagonalMatrix< StaticMatrix<float,3UL,3UL> >;

   mtrc::numeric::IsIdentity< IdentityMatrix<int> >::value         // Evaluates to 1
   mtrc::numeric::IsIdentity< const IdentityMatrix<float> >::Type  // Results in TrueType
   mtrc::numeric::IsIdentity< volatile IdentityMatrix<double> >    // Is derived from TrueType
   mtrc::numeric::IsIdentity< DynamicMatrix<int> >::value          // Evaluates to 0
   mtrc::numeric::IsIdentity< const DiagonalMatrixType >::Type     // Results in FalseType
   mtrc::numeric::IsIdentity< volatile CompressedMatrix<double> >  // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsIdentity : public BoolConstant<IsUniLower_v<T> && IsUniUpper_v<T>> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsIdentity type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct IsIdentity<const T> : public IsIdentity<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsIdentity type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct IsIdentity<volatile T> : public IsIdentity<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsIdentity type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct IsIdentity<const volatile T> : public IsIdentity<T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsIdentity type trait.
// \ingroup math_type_traits
//
// The IsIdentity_v variable template provides a convenient shortcut to access the nested
// \a value of the IsIdentity class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsIdentity<T>::value;
   constexpr bool value2 = mtrc::numeric::IsIdentity_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsIdentity_v = IsIdentity<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
