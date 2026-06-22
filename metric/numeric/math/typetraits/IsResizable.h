// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISRESIZABLE_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISRESIZABLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/IsVector.h>
#include <metric/numeric/math/typetraits/RemoveAdaptor.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/typetraits/IsDetected.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper for the IsResizable type trait.
// \ingroup math_type_traits
*/
template <typename T> using Resizable1_t = decltype(std::declval<T &>().resize(std::declval<size_t>()));
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper for the IsResizable type trait.
// \ingroup math_type_traits
*/
template <typename T>
using Resizable2_t = decltype(std::declval<T &>().resize(std::declval<size_t>(), std::declval<size_t>()));
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for resizable data types.
// \ingroup math_type_traits
//
// This type trait tests whether the given data type is a resizable data type. In case the
// data type can be resized (via the resize() function), the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType. Examples:

   \code
   mtrc::numeric::IsResizable< DynamicVector<double> >::value    // Evaluates to 1
   mtrc::numeric::IsResizable< CompressedVector<double> >::Type  // Results in TrueType
   mtrc::numeric::IsResizable< ZeroMatrix<int> >                 // Is derived from TrueType
   mtrc::numeric::IsResizable< int >::value                      // Evaluates to 0
   mtrc::numeric::IsResizable< StaticVector<float,3UL> >::Type   // Results in FalseType
   mtrc::numeric::IsResizable< const DynamicVector<float> >      // Is derived from FalseType
   \endcode
*/
template <typename T>
struct IsResizable : public BoolConstant<(IsVector_v<T> && IsDetected_v<Resizable1_t, RemoveAdaptor_t<T>>) ||
										 (IsMatrix_v<T> && IsDetected_v<Resizable2_t, RemoveAdaptor_t<T>>)> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsResizable type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsResizable<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsResizable type trait.
// \ingroup math_type_traits
//
// The IsResizable_v variable template provides a convenient shortcut to access the nested
// \a value of the IsResizable class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsResizable<T>::value;
   constexpr bool value2 = mtrc::numeric::IsResizable_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsResizable_v = IsResizable<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
