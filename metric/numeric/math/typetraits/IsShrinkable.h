// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSHRINKABLE_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSHRINKABLE_H
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
template <typename T> using ShrinkToFit_t = decltype(std::declval<T &>().shrinkToFit());
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for shrinkable data types.
// \ingroup math_type_traits
//
// This type trait tests whether the given data type is a shrinkable data type. In case the
// data type can be shrunk (via the shrinkToFit() function), the \a value member constant is
// set to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType. Examples:

   \code
   mtrc::numeric::IsShrinkable< DynamicVector<double> >::value    // Evaluates to 1
   mtrc::numeric::IsShrinkable< CompressedVector<double> >::Type  // Results in TrueType
   mtrc::numeric::IsShrinkable< DynamicMatrix<int> >              // Is derived from TrueType
   mtrc::numeric::IsShrinkable< int >::value                      // Evaluates to 0
   mtrc::numeric::IsShrinkable< StaticVector<float,3UL> >::Type   // Results in FalseType
   mtrc::numeric::IsShrinkable< const DynamicVector<float> >      // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsShrinkable : public IsDetected<ShrinkToFit_t, RemoveAdaptor_t<T>> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsShrinkable type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsShrinkable<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsShrinkable type trait.
// \ingroup math_type_traits
//
// The IsShrinkable_v variable template provides a convenient shortcut to access the nested
// \a value of the IsShrinkable class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsShrinkable<T>::value;
   constexpr bool value2 = mtrc::numeric::IsShrinkable_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsShrinkable_v = IsShrinkable<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
