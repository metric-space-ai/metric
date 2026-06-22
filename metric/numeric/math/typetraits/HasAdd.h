// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASADD_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASADD_H
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
/*!\brief Auxiliary helper struct for the IsAddHelper type trait.
// \ingroup math_type_traits
*/
template <typename T1, typename T2, typename = void> struct HasAddHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAddHelper type trait for types providing an addition operator.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
struct HasAddHelper<T1, T2, Void_t<decltype(std::declval<T1>() + std::declval<T2>())>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of an addition operator for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether an addition operator (i.e. operator+) exists
// for the two given data types \a T1 and \a T2 (taking the cv-qualifiers into account). In case
// the operator is available, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::HasAdd< int, int >::value                         // Evaluates to 1
   mtrc::numeric::HasAdd< complex<float>, complex<float> >::Type    // Results in TrueType
   mtrc::numeric::HasAdd< DynamicVector<int>, DynamicVector<int> >  // Is derived from TrueType
   mtrc::numeric::HasAdd< int, complex<float> >::value              // Evaluates to 0
   mtrc::numeric::HasAdd< complex<int>, complex<float> >::Type      // Results in FalseType
   mtrc::numeric::HasAdd< DynamicMatrix<int>, DynamicVector<int> >  // Is derived from FalseType
   \endcode
*/
template <typename T1, typename T2, typename = void> struct HasAdd : public HasAddHelper<T1, T2> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAdd type trait for vector/vector additions.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
struct HasAdd<T1, T2, EnableIf_t<IsVector_v<T1> && IsVector_v<T2>>>
	: public HasAdd<typename T1::ElementType, typename T2::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAdd type trait for matrix/matrix additions.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
struct HasAdd<T1, T2, EnableIf_t<IsMatrix_v<T1> && IsMatrix_v<T2>>>
	: public HasAdd<typename T1::ElementType, typename T2::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasAdd type trait.
// \ingroup math_type_traits
//
// The HasAdd_v variable template provides a convenient shortcut to access the nested \a value
// of the HasAdd class template. For instance, given the types \a T1 and \a T2 the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasAdd<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::HasAdd_v<T1,T2>;
   \endcode
*/
template <typename T1, typename T2> constexpr bool HasAdd_v = HasAdd<T1, T2>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
