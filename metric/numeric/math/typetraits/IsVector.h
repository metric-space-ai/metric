// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISVECTOR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/Forward.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper functions for the IsVector type trait.
// \ingroup math_type_traits
*/
template <typename VT, bool TF> TrueType isVector_backend(const volatile Vector<VT, TF> *);

FalseType isVector_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for vector types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a dense or sparse vector
// type (i.e. whether \a T is derived from the Vector base class). In case the type is a vector
// type, the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   using namespace mtrc::numeric;

   IsVector< StaticVector<float,3U> >::value      // Evaluates to 1
   IsVector< const DynamicVector<double> >::Type  // Results in TrueType
   IsVector< volatile CompressedVector<int> >     // Is derived from TrueType
   IsVector< StaticMatrix<double,3U,3U> >::value  // Evaluates to 0
   IsVector< const DynamicMatrix<double> >::Type  // Results in FalseType
   IsVector< volatile CompressedMatrix<int> >     // Is derived from FalseType
   \endcode
*/

template <typename T> struct IsVector : public decltype(isVector_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsVector type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsVector<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsVector type trait.
// \ingroup math_type_traits
//
// The IsVector_v variable template provides a convenient shortcut to access the nested \a value
// of the IsVector class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsVector<T>::value;
   constexpr bool value2 = mtrc::numeric::IsVector_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsVector_v = IsVector<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
