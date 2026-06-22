// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASMAX_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASMAX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsVector.h>
#include <metric/numeric/util/Complex.h>
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
/*!\brief Auxiliary helper struct for the IsMaxHelper type trait.
// \ingroup math_type_traits
*/
template <typename T1, typename T2, typename = void> struct HasMaxHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasMaxHelper type trait for types providing a max() function.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
struct HasMaxHelper<T1, T2, Void_t<decltype(max(std::declval<T1>(), std::declval<T2>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of a max() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether a binary max() operation exists for the
// two given data types \a T1 and \a T2 (taking the cv-qualifiers into account). In case a
// binary max() operation is available, the \a value member constant is set to \a true, the
// nested type definition \a Type is \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class derives
// from \a FalseType.

   \code
   mtrc::numeric::HasMax< int, int >::value                         // Evaluates to 1
   mtrc::numeric::HasMax< float, double >::Type                     // Results in TrueType
   mtrc::numeric::HasMax< DynamicVector<int>, DynamicVector<int> >  // Is derived from TrueType
   mtrc::numeric::HasMax< int, unsigned int >::value                // Evaluates to 0
   mtrc::numeric::HasMax< complex<float>, complex<float> >::Type    // Results in FalseType
   mtrc::numeric::HasMax< DynamicMatrix<int>, DynamicVector<int> >  // Is derived from FalseType
   \endcode
*/
template <typename T1, typename T2, typename = void> struct HasMax : public HasMaxHelper<T1, T2> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasMax type trait for complex numbers.
// \ingroup math_type_traits
*/
template <typename T> struct HasMax<complex<T>, complex<T>> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasMax type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
struct HasMax<T1, T2, EnableIf_t<IsVector_v<T1> && IsVector_v<T2>>>
	: public HasMax<typename T1::ElementType, typename T2::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasMax type trait.
// \ingroup math_type_traits
//
// The HasMax_v variable template provides a convenient shortcut to access the nested \a value
// of the HasMax class template. For instance, given the types \a T1 and \a T2 the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasMax<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::HasMax_v<T1,T2>;
   \endcode
*/
template <typename T1, typename T2> constexpr bool HasMax_v = HasMax<T1, T2>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
