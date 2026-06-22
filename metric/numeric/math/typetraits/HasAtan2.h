// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASATAN2_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASATAN2_H
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
/*!\brief Auxiliary helper struct for the HasAtan2 type trait.
// \ingroup math_type_traits
*/
template <typename T1, typename T2, typename = void> struct HasAtan2Helper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAtan2Helper type trait for types providing the atan2() operation.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
struct HasAtan2Helper<T1, T2, Void_t<decltype(atan2(std::declval<T1>(), std::declval<T2>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the atan2() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the atan2() operation exists for the two given
// data types \a T1 and \a T2 (taking the cv-qualifiers into account). In case the operation is
// available, the \a value member constant is set to \a true, the nested type definition \a Type
// is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::HasAtan2< int, int >::value                         // Evaluates to 1
   mtrc::numeric::HasAtan2< complex<float>, complex<float> >::Type    // Results in TrueType
   mtrc::numeric::HasAtan2< DynamicVector<int>, DynamicVector<int> >  // Is derived from TrueType
   mtrc::numeric::HasAtan2< int, complex<float> >::value              // Evaluates to 0
   mtrc::numeric::HasAtan2< complex<int>, complex<float> >::Type      // Results in FalseType
   mtrc::numeric::HasAtan2< DynamicMatrix<int>, DynamicVector<int> >  // Is derived from FalseType
   \endcode
*/
template <typename T1, typename T2, typename = void> struct HasAtan2 : public HasAtan2Helper<T1, T2> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAtan2 type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
struct HasAtan2<T1, T2, EnableIf_t<IsVector_v<T1> && IsVector_v<T2>>>
	: public HasAtan2<typename T1::ElementType, typename T2::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAtan2 type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
struct HasAtan2<T1, T2, EnableIf_t<IsMatrix_v<T1> && IsMatrix_v<T2>>>
	: public HasAtan2<typename T1::ElementType, typename T2::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasAtan2 type trait.
// \ingroup math_type_traits
//
// The HasAtan2_v variable template provides a convenient shortcut to access the nested \a value
// of the HasAtan2 class template. For instance, given the types \a T1 and \a T2 the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasAtan2<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::HasAtan2_v<T1,T2>;
   \endcode
*/
template <typename T1, typename T2> constexpr bool HasAtan2_v = HasAtan2<T1, T2>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
