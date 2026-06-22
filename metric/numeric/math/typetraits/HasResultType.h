// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASRESULTTYPE_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASRESULTTYPE_H
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
/*!\brief Compile time check for a public nested type alias \a ResultType.
// \ingroup math_type_traits
//
// This type trait tests whether the given data type \a T provides a public nested type alias
// named \a ResultType. If it does, the \a value member constant is set to \a true, the nested
// type definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::HasResultType< StaticVector<float,3U> >::value      // Evaluates to 1
   mtrc::numeric::HasResultType< const CompressedVector<int> >::Type  // Results in TrueType
   mtrc::numeric::HasResultType< volatile DynamicMatrix<double> >     // Is derived from TrueType
   mtrc::numeric::HasResultType< int >::value                         // Evaluates to 0
   mtrc::numeric::HasResultType< const double >::Type                 // Results in FalseType
   mtrc::numeric::HasResultType< volatile complex<float> >            // Is derived from FalseType
   \endcode
*/
template <typename T> using HasResultType = IsDetected<ResultType_t, T>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasResultType type trait.
// \ingroup math_type_traits
//
// The HasResultType_v variable template provides a convenient shortcut to access the nested
// \a value of the HasResultType class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasResultType<T>::value;
   constexpr bool value2 = mtrc::numeric::HasResultType_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasResultType_v = HasResultType<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
