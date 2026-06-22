// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_INTEGERSEQUENCE_H
#define METRIC_NUMERIC_UTIL_INTEGERSEQUENCE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/MaybeUnused.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  TYPE DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\class mtrc::numeric::integer_sequence
// \brief Integer sequence type of the Metric numeric library.
// \ingroup util
*/
using std::integer_sequence;
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::index_sequence
// \brief Index sequence type of the Metric numeric library.
// \ingroup util
*/
using std::index_sequence;
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::make_integer_sequence
// \brief Import of the std::make_integer_sequence alias template into the Metric numeric namespace.
// \ingroup util
*/
using std::make_integer_sequence;
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::make_index_sequence
// \brief Import of the std::make_index_sequence alias template into the Metric numeric namespace.
// \ingroup util
*/
using std::make_index_sequence;
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Equality operator for the comparison of two index sequences.
// \ingroup util
//
// \param lhs The left-hand side index sequence for the comparison.
// \param rhs The right-hand side index sequence for the comparison.
// \return \a true if the two index sequences are equal, \a false if not.
*/
template <size_t... I1s, size_t... I2s> //
constexpr bool operator==(index_sequence<I1s...> lhs, index_sequence<I2s...> rhs) noexcept
{
	MAYBE_UNUSED(lhs, rhs);

	return false;
}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Equality operator for the comparison of two identical index sequences.
// \ingroup util
//
// \param lhs The left-hand side index sequence for the comparison.
// \param rhs The right-hand side index sequence for the comparison.
// \return \a true.
*/
template <size_t... I1s> //
constexpr bool operator==(index_sequence<I1s...> lhs, index_sequence<I1s...> rhs) noexcept
{
	MAYBE_UNUSED(lhs, rhs);

	return true;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Inequality operator for the comparison of two index sequences.
// \ingroup util
//
// \param lhs The left-hand side index sequence for the comparison.
// \param rhs The right-hand side index sequence for the comparison.
// \return \a true if the two index sequences are not equal, \a false if they are equal.
*/
template <size_t... I1s, size_t... I2s>
constexpr bool operator!=(index_sequence<I1s...> lhs, index_sequence<I2s...> rhs) noexcept
{
	MAYBE_UNUSED(lhs, rhs);

	return !(lhs == rhs);
}
//*************************************************************************************************

//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Shifts the given index sequence by a given offset.
// \ingroup util
//
// \param sequence The given index sequence
// \return The shifted index sequence.
*/
template <size_t Offset // The offset for the shift operation
		  ,
		  size_t... Is> // The sequence of indices
constexpr decltype(auto) shift(std::index_sequence<Is...> sequence)
{
	MAYBE_UNUSED(sequence);

	return std::index_sequence<(Is + Offset)...>();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creates a subsequence from the given index sequence.
// \ingroup util
//
// \param sequence The given index sequence
// \return The resulting subsequence.
*/
template <size_t... Is1 // The indices to be selected
		  ,
		  size_t... Is2> // The sequence of indices
constexpr decltype(auto) subsequence(std::index_sequence<Is2...> sequence)
{
	MAYBE_UNUSED(sequence);

	constexpr size_t indices[] = {Is2...};
	return std::index_sequence<indices[Is1]...>();
}
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ALIAS DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the setup of shifted index sequences.
// \ingroup util
//
// The make_shifted_index_sequence alias template provides a convenient way to create index
// sequences with specific initial index and a specific number of indices. The following code
// example demonstrates the use of make_shifted_index_sequence:

   \code
   // Creating the index sequence <2,3,4,5,6>
   using Type = make_shifted_index_sequence<2UL,5UL>;
   \endcode
*/
template <size_t Offset // The offset of the index sequence
		  ,
		  size_t N> // The total number of indices in the index sequence
using make_shifted_index_sequence = decltype(shift<Offset>(make_index_sequence<N>()));
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the setup of shifted index subsequences.
// \ingroup util
//
// The make_shifted_index_subsequence alias template provides a convenient way to create a
// subsequence of an index sequences with specific initial index and a specific number of indices.
// The following code example demonstrates the use of make_shifted_index_subsequence:

   \code
   // Creating the subsequence <3,6,8> from the index sequence <2,3,4,5,6,7,8>
   using Type = make_shifted_index_subsequence<2UL,7UL,1UL,4UL,6UL>;
   \endcode
*/
template <size_t Offset // The offset of the index sequence
		  ,
		  size_t N // The total number of indices in the index sequence
		  ,
		  size_t... Is> // The indices to be selected
using make_shifted_index_subsequence = decltype(subsequence<Is...>(shift<Offset>(make_index_sequence<N>())));
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
