// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_RANK_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_RANK_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for array ranks.
// \ingroup type_traits
//
// This type trait determines the rank of the given template argument. In case the given type
// is an array type, the nested \a value member constant is set to the number of dimensions
// of \a T. Otherwise \a value is set to 0.

   \code
   mtrc::numeric::Rank< int[] >::value               // Evaluates to 1
   mtrc::numeric::Rank< int[3] >::value              // Evaluates to 1
   mtrc::numeric::Rank< const int[2][3][4] >::value  // Evaluates to 3
   mtrc::numeric::Rank< int[][3] >::value            // Evaluates to 2
   mtrc::numeric::Rank< int const* >::value          // Evaluates to 0
   mtrc::numeric::Rank< std::vector<int> >::value    // Evaluates to 0
   \endcode
*/
template <typename T> struct Rank : public IntegralConstant<size_t, 0UL> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the Rank type trait for empty arrays.
template <typename T> struct Rank<T[]> : public IntegralConstant<size_t, 1UL + Rank<T>::value> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the Rank type trait for non-empty arrays.
template <typename T, unsigned int N> struct Rank<T[N]> : public IntegralConstant<size_t, 1UL + Rank<T>::value> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the Rank type trait.
// \ingroup type_traits
//
// The Rank_v variable template provides a convenient shortcut to access the nested \a value of
// the Rank class template. For instance, given the type \a T the following two statements are
// identical:

   \code
   constexpr size_t value1 = mtrc::numeric::Rank<T>::value;
   constexpr size_t value2 = mtrc::numeric::Rank_v<T>;
   \endcode
*/
template <typename T> constexpr size_t Rank_v = Rank<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
