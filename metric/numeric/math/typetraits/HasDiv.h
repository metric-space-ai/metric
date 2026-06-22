// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASDIV_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASDIV_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsDenseVector.h>
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
/*!\brief Auxiliary helper struct for the IsDivHelper type trait.
// \ingroup math_type_traits
*/
template <typename T1, typename T2, typename = void> struct HasDivHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasDivHelper type trait for types providing a division operator.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
struct HasDivHelper<T1, T2, Void_t<decltype(std::declval<T1>() / std::declval<T2>())>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of a division operator for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether a division operator (i.e. operator/) exists
// for the two given data types \a T1 and \a T2 (taking the cv-qualifiers into account). In case
// the operator is available, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::HasDiv< int, int >::value                         // Evaluates to 1
   mtrc::numeric::HasDiv< complex<float>, complex<float> >::Type    // Results in TrueType
   mtrc::numeric::HasDiv< DynamicVector<int>, DynamicVector<int> >  // Is derived from TrueType
   mtrc::numeric::HasDiv< int, complex<float> >::value              // Evaluates to 0
   mtrc::numeric::HasDiv< complex<int>, complex<float> >::Type      // Results in FalseType
   mtrc::numeric::HasDiv< DynamicMatrix<int>, DynamicVector<int> >  // Is derived from FalseType
   \endcode
*/
template <typename T1, typename T2, typename = void> struct HasDiv : public HasDivHelper<T1, T2> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasDiv type trait for vector/vector divisions.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
struct HasDiv<T1, T2, EnableIf_t<IsVector_v<T1> && IsDenseVector_v<T2>>>
	: public HasDiv<typename T1::ElementType, typename T2::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasDiv type trait.
// \ingroup math_type_traits
//
// The HasDiv_v variable template provides a convenient shortcut to access the nested \a value
// of the HasDiv class template. For instance, given the types \a T1 and \a T2 the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasDiv<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::HasDiv_v<T1,T2>;
   \endcode
*/
template <typename T1, typename T2> constexpr bool HasDiv_v = HasDiv<T1, T2>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
