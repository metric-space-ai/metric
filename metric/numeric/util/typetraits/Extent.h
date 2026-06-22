// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_EXTENT_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_EXTENT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for the size of array bounds.
// \ingroup type_traits
//
// Via this type trait it is possible to query at compile time for the size of a particular
// array extent. In case the given template argument is an array type with a rank greater
// than N, the \a value member constant is set to the number of elements of the N'th array
// dimension. In all other cases, and especially in case the N'th array dimension is
// incomplete, \a value is set to 0.

   \code
   mtrc::numeric::Extent< int[4], 0UL >::value            // Evaluates to 4
   mtrc::numeric::Extent< int[2][3][4], 0UL >::value      // Evaluates to 2
   mtrc::numeric::Extent< int[2][3][4], 1UL >::value      // Evaluates to 3
   mtrc::numeric::Extent< int[2][3][4], 2UL >::value      // Evaluates to 4
   mtrc::numeric::Extent< int[][2], 0UL >::value          // Evaluates to 0
   mtrc::numeric::Extent< int[][2], 1UL >::value          // Evaluates to 2
   mtrc::numeric::Extent< int*, 0UL >::value              // Evaluates to 0
   mtrc::numeric::Extent< std::vector<int>, 0UL >::value  // Evaluates to 0 (std::vector is NOT an array type)
   \endcode
*/
template <typename T, size_t N> struct Extent : public IntegralConstant<size_t, 0UL> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Partial specialization of the Extent type trait for empty array extents.
template <typename T, size_t N> struct Extent<T[], N> : public IntegralConstant<size_t, Extent<T, N - 1UL>::value> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Partial specialization of the Extent type trait for non-empty array extents.
template <typename T, size_t N, size_t E>
struct Extent<T[E], N> : public IntegralConstant<size_t, Extent<T, N - 1UL>::value> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Terminating partial specialization of the Extent type trait for empty array extents.
template <typename T> struct Extent<T[], 0UL> : public IntegralConstant<size_t, 0UL> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Terminating partial specialization of the Extent type trait for non-empty array extents.
template <typename T, size_t E> struct Extent<T[E], 0UL> : public IntegralConstant<size_t, E> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the Extent type trait.
// \ingroup type_traits
//
// The Extent_v variable template provides a convenient shortcut to access the nested \a value
// of the Extent class template. For instance, given the type \a T and the compile time constant
// \a N the following two statements are identical:

   \code
   constexpr size_t value1 = mtrc::numeric::Extent<T,N>::value;
   constexpr size_t value2 = mtrc::numeric::Extent_v<T,N>;
   \endcode
*/
template <typename T, size_t N> constexpr size_t Extent_v = Extent<T, N>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
