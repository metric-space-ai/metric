// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISVALID_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISVALID_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/InvalidType.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type check.
// \ingroup type_traits
//
// This class tests whether the given template parameter is a valid or invalid data type (i.e.
// if the type is the INVALID_TYPE). If \a T is not the INVALID_TYPE class type, the \a value
// member constant is set to \a true, the nested type definition \a Type is \a TrueType, and
// the class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType, and the class derives from \a FalseType.

   \code
   mtrc::numeric::IsValid<int>::value                // Evaluates to 'true'
   mtrc::numeric::IsValid<float const>::Type         // Results in TrueType
   mtrc::numeric::IsValid<double volatile>           // Is derived from TrueType
   mtrc::numeric::IsValid<INVALID_TYPE>::value       // Evaluates to 'false'
   mtrc::numeric::IsValid<INVALID_TYPE const>::Type  // Results in FalseType
   mtrc::numeric::IsValid<INVALID_TYPE volatile>     // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsValid : public TrueType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsValid type trait for the plain 'INVALID_TYPE' type.
template <> struct IsValid<INVALID_TYPE> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsValid type trait for 'const INVALID_TYPE'.
template <> struct IsValid<const INVALID_TYPE> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsValid type trait for 'volatile INVALID_TYPE'.
template <> struct IsValid<volatile INVALID_TYPE> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsValid type trait for 'const volatile INVALID_TYPE'.
template <> struct IsValid<const volatile INVALID_TYPE> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsValid type trait.
// \ingroup type_traits
//
// The IsValid_v variable template provides a convenient shortcut to access the nested \a value
// of the IsValid class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsValid<T>::value;
   constexpr bool value2 = mtrc::numeric::IsValid_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsValid_v = IsValid<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
