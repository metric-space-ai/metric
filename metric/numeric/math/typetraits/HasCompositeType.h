// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASCOMPOSITETYPE_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASCOMPOSITETYPE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/util/typetraits/IsDetected.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for a public nested type alias \a CompositeType.
// \ingroup math_type_traits
//
// This type trait tests whether the given data type \a T provides a public nested type alias
// named \a CompositeType. If it does, the \a value member constant is set to \a true, the nested
// type definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::HasCompositeType< StaticVector<float,3U> >::value      // Evaluates to 1
   mtrc::numeric::HasCompositeType< const CompressedVector<int> >::Type  // Results in TrueType
   mtrc::numeric::HasCompositeType< volatile DynamicMatrix<double> >     // Is derived from TrueType
   mtrc::numeric::HasCompositeType< int >::value                         // Evaluates to 0
   mtrc::numeric::HasCompositeType< const double >::Type                 // Results in FalseType
   mtrc::numeric::HasCompositeType< volatile complex<float> >            // Is derived from FalseType
   \endcode
*/
template <typename T> using HasCompositeType = IsDetected<CompositeType_t, T>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasCompositeType type trait.
// \ingroup math_type_traits
//
// The HasCompositeType_v variable template provides a convenient shortcut to access the nested
// \a value of the HasCompositeType class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasCompositeType<T>::value;
   constexpr bool value2 = mtrc::numeric::HasCompositeType_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasCompositeType_v = HasCompositeType<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
