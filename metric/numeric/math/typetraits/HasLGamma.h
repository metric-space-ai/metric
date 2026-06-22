// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASLGAMMA_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASLGAMMA_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/IsVector.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/Void.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the HasLGamma type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasLGammaHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasLGammaHelper type trait for types providing the lgamma() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasLGammaHelper<T, Void_t<decltype(lgamma(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the lgamma() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the lgamma() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoLGamma {};  // Definition of a type without the lgamma() operation

   mtrc::numeric::HasLGamma< int >::value                  // Evaluates to 1
   mtrc::numeric::HasLGamma< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasLGamma< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasLGamma< NoLGamma >::value             // Evaluates to 0
   mtrc::numeric::HasLGamma< NoLGamma >::Type              // Results in FalseType
   mtrc::numeric::HasLGamma< NoLGamma >                    // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasLGamma : public HasLGammaHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasLGamma type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasLGamma<T, EnableIf_t<IsVector_v<T>>> : public HasLGamma<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasLGamma type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasLGamma<T, EnableIf_t<IsMatrix_v<T>>> : public HasLGamma<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasLGamma type trait.
// \ingroup math_type_traits
//
// The HasLGamma_v variable template provides a convenient shortcut to access the nested \a value
// of the HasLGamma class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasLGamma<T>::value;
   constexpr bool value2 = mtrc::numeric::HasLGamma_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasLGamma_v = HasLGamma<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
