// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASLOG2_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASLOG2_H
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
/*!\brief Auxiliary helper struct for the HasLog2 type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasLog2Helper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasLog2Helper type trait for types providing the log2() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasLog2Helper<T, Void_t<decltype(log2(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the log2() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the log2() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoLog2 {};  // Definition of a type without the log2() operation

   mtrc::numeric::HasLog2< int >::value                  // Evaluates to 1
   mtrc::numeric::HasLog2< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasLog2< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasLog2< NoLog2 >::value               // Evaluates to 0
   mtrc::numeric::HasLog2< NoLog2 >::Type                // Results in FalseType
   mtrc::numeric::HasLog2< NoLog2 >                      // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasLog2 : public HasLog2Helper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasLog2 type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasLog2<T, EnableIf_t<IsVector_v<T>>> : public HasLog2<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasLog2 type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasLog2<T, EnableIf_t<IsMatrix_v<T>>> : public HasLog2<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasLog2 type trait.
// \ingroup math_type_traits
//
// The HasLog2_v variable template provides a convenient shortcut to access the nested \a value
// of the HasLog2 class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasLog2<T>::value;
   constexpr bool value2 = mtrc::numeric::HasLog2_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasLog2_v = HasLog2<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
