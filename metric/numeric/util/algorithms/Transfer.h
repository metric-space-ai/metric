// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_ALGORITHMS_TRANSFER_H
#define METRIC_NUMERIC_UTIL_ALGORITHMS_TRANSFER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <algorithm>
#include <iterator>
#include <metric/numeric/util/typetraits/IsAssignable.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  TRANSFER ALGORITHM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Transfers the elements from the given source range to the destination range.
// \ingroup algorithms
//
// \param first Iterator to the first element of the source range.
// \param last Iterator to the element one past the last element of the source range.
// \param dest Iterator to the first element of the destination range.
// \return Output iterator to the element one past the last copied element.
//
// This function transfers the elements in the range \f$ [first,last) \f$ to the specified
// destination range. In case the elements provide a no-throw move assignment, the transfer
// operation is handled via move. Else the elements are copied.
*/
template <typename InputIterator, typename OutputIterator>
OutputIterator transfer(InputIterator first, InputIterator last, OutputIterator dest)
{
	using T = typename std::iterator_traits<InputIterator>::value_type;

	if (IsNothrowMoveAssignable_v<T>) {
		return std::move(first, last, dest);
	} else {
		return std::copy(first, last, dest);
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Transfers the elements from the given source range to the destination range.
// \ingroup algorithms
//
// \param first Iterator to the first element of the source range.
// \param n The number of elements to be moved.
// \param dest Iterator to the first element of the destination range.
// \return Output iterator to the element one past the last copied element.
//
// This function transfers the elements in the range \f$ [first,first+n) \f$ to the specified
// destination range. In case the elements provide a no-throw move assignment, the transfer
// operation is handled via move. Else the elements are copied.
*/
template <typename InputIterator, typename OutputIterator>
OutputIterator transfer_n(InputIterator first, size_t n, OutputIterator dest)
{
	using T = typename std::iterator_traits<InputIterator>::value_type;

	if (IsNothrowMoveAssignable_v<T>) {
		return std::copy_n(std::make_move_iterator(first), n, dest);
	} else {
		return std::copy_n(first, n, dest);
	}
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
