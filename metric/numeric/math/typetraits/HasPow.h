// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASPOW_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASPOW_H
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
/*!\brief Auxiliary helper struct for the HasPow type trait.
// \ingroup math_type_traits
*/
template <typename T1, typename T2, typename = void> struct HasPowHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasPowHelper type trait for types providing the pow() operation.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
struct HasPowHelper<T1, T2, Void_t<decltype(pow(std::declval<T1>(), std::declval<T2>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the pow() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the pow() operation exists for the two given
// data types \a T1 and \a T2 (taking the cv-qualifiers into account). In case the operation is
// available, the \a value member constant is set to \a true, the nested type definition \a Type
// is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::HasPow< int, int >::value                         // Evaluates to 1
   mtrc::numeric::HasPow< complex<float>, complex<float> >::Type    // Results in TrueType
   mtrc::numeric::HasPow< DynamicVector<int>, DynamicVector<int> >  // Is derived from TrueType
   mtrc::numeric::HasPow< int, complex<float> >::value              // Evaluates to 0
   mtrc::numeric::HasPow< complex<int>, complex<float> >::Type      // Results in FalseType
   mtrc::numeric::HasPow< DynamicMatrix<int>, DynamicVector<int> >  // Is derived from FalseType
   \endcode
*/
template <typename T1, typename T2, typename = void> struct HasPow : public HasPowHelper<T1, T2> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasPow type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
struct HasPow<T1, T2, EnableIf_t<IsVector_v<T1> && IsVector_v<T2>>>
	: public HasPow<typename T1::ElementType, typename T2::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasPow type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
struct HasPow<T1, T2, EnableIf_t<IsMatrix_v<T1> && IsMatrix_v<T2>>>
	: public HasPow<typename T1::ElementType, typename T2::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasPow type trait.
// \ingroup math_type_traits
//
// The HasPow_v variable template provides a convenient shortcut to access the nested \a value
// of the HasPow class template. For instance, given the types \a T1 and \a T2 the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasPow<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::HasPow_v<T1,T2>;
   \endcode
*/
template <typename T1, typename T2> constexpr bool HasPow_v = HasPow<T1, T2>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
