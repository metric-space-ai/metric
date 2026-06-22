// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISCLEARABLE_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISCLEARABLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/RemoveAdaptor.h>
#include <metric/numeric/util/IntegralConstant.h>
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
/*!\brief Auxiliary helper for the IsClearable type trait.
// \ingroup math_type_traits
*/
template <typename T> using Clear_t = decltype(std::declval<T &>().clear());
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for clearable data types.
// \ingroup math_type_traits
//
// This type trait tests whether the given data type is a clearable data type. In case the
// data type can be cleared (via the clear() function), the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType. Examples:

   \code
   mtrc::numeric::IsClearable< DynamicVector<double> >::value    // Evaluates to 1
   mtrc::numeric::IsClearable< CompressedVector<double> >::Type  // Results in TrueType
   mtrc::numeric::IsClearable< ZeroMatrix<int> >                 // Is derived from TrueType
   mtrc::numeric::IsClearable< int >::value                      // Evaluates to 0
   mtrc::numeric::IsClearable< StaticVector<float,3UL> >::Type   // Results in FalseType
   mtrc::numeric::IsClearable< const DynamicVector<float> >      // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsClearable : public IsDetected<Clear_t, RemoveAdaptor_t<T>> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsClearable type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsClearable<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsClearable type trait.
// \ingroup math_type_traits
//
// The IsClearable_v variable template provides a convenient shortcut to access the nested
// \a value of the IsClearable class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsClearable<T>::value;
   constexpr bool value2 = mtrc::numeric::IsClearable_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsClearable_v = IsClearable<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
