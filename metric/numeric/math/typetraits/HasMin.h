// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASMIN_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASMIN_H
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
/*!\brief Auxiliary helper struct for the IsMinHelper type trait.
// \ingroup math_type_traits
*/
template <typename T1, typename T2, typename = void> struct HasMinHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasMinHelper type trait for types providing a min() function.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
struct HasMinHelper<T1, T2, Void_t<decltype(min(std::declval<T1>(), std::declval<T2>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of a min() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether a binary min() operation exists for the
// two given data types \a T1 and \a T2 (taking the cv-qualifiers into account). In case a
// binary min() operation is available, the \a value member constant is set to \a true, the
// nested type definition \a Type is \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class derives
// from \a FalseType.

   \code
   mtrc::numeric::HasMin< int, int >::value                         // Evaluates to 1
   mtrc::numeric::HasMin< float, double >::Type                     // Results in TrueType
   mtrc::numeric::HasMin< DynamicVector<int>, DynamicVector<int> >  // Is derived from TrueType
   mtrc::numeric::HasMin< int, unsigned int >::value                // Evaluates to 0
   mtrc::numeric::HasMin< complex<float>, complex<float> >::Type    // Results in FalseType
   mtrc::numeric::HasMin< DynamicMatrix<int>, DynamicVector<int> >  // Is derived from FalseType
   \endcode
*/
template <typename T1, typename T2, typename = void> struct HasMin : public HasMinHelper<T1, T2> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasMin type trait for complex numbers.
// \ingroup math_type_traits
*/
template <typename T> struct HasMin<complex<T>, complex<T>> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasMin type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
struct HasMin<T1, T2, EnableIf_t<IsVector_v<T1> && IsVector_v<T2>>>
	: public HasMin<typename T1::ElementType, typename T2::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasMin type trait.
// \ingroup math_type_traits
//
// The HasMin_v variable template provides a convenient shortcut to access the nested \a value
// of the HasMin class template. For instance, given the types \a T1 and \a T2 the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasMin<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::HasMin_v<T1,T2>;
   \endcode
*/
template <typename T1, typename T2> constexpr bool HasMin_v = HasMin<T1, T2>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
