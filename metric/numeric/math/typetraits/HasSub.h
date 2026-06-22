// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASSUB_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASSUB_H
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
/*!\brief Auxiliary helper struct for the IsSubHelper type trait.
// \ingroup math_type_traits
*/
template <typename T1, typename T2, typename = void> struct HasSubHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasSubHelper type trait for types providing a subtraction operator.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
struct HasSubHelper<T1, T2, Void_t<decltype(std::declval<T1>() - std::declval<T2>())>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of a subtraction operator for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether a subtraction operator (i.e. operator+) exists
// for the two given data types \a T1 and \a T2 (taking the cv-qualifiers into account). In case
// the operator is available, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::HasSub< int, int >::value                         // Evaluates to 1
   mtrc::numeric::HasSub< complex<float>, complex<float> >::Type    // Results in TrueType
   mtrc::numeric::HasSub< DynamicVector<int>, DynamicVector<int> >  // Is derived from TrueType
   mtrc::numeric::HasSub< int, complex<float> >::value              // Evaluates to 0
   mtrc::numeric::HasSub< complex<int>, complex<float> >::Type      // Results in FalseType
   mtrc::numeric::HasSub< DynamicMatrix<int>, DynamicVector<int> >  // Is derived from FalseType
   \endcode
*/
template <typename T1, typename T2, typename = void> struct HasSub : public HasSubHelper<T1, T2> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasSub type trait for vector/vector subtractions.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
struct HasSub<T1, T2, EnableIf_t<IsVector_v<T1> && IsVector_v<T2>>>
	: public HasSub<typename T1::ElementType, typename T2::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasSub type trait for matrix/matrix subtractions.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
struct HasSub<T1, T2, EnableIf_t<IsMatrix_v<T1> && IsMatrix_v<T2>>>
	: public HasSub<typename T1::ElementType, typename T2::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasSub type trait.
// \ingroup math_type_traits
//
// The HasSub_v variable template provides a convenient shortcut to access the nested \a value
// of the HasSub class template. For instance, given the types \a T1 and \a T2 the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasSub<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::HasSub_v<T1,T2>;
   \endcode
*/
template <typename T1, typename T2> constexpr bool HasSub_v = HasSub<T1, T2>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
