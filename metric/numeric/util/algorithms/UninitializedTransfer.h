// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_ALGORITHMS_UNINITIALIZEDTRANSFER_H
#define METRIC_NUMERIC_UTIL_ALGORITHMS_UNINITIALIZEDTRANSFER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <algorithm>
#include <iterator>
#include <metric/numeric/util/algorithms/UninitializedMove.h>
#include <metric/numeric/util/typetraits/IsAssignable.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  UNINITIALIZED_TRANSFER ALGORITHMS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Transfers the elements from the given source range to the uninitialized destination range.
// \ingroup algorithms
//
// \param first Iterator to the first element of the source range.
// \param last Iterator to the element one past the last element of the source range.
// \param dest Iterator to the first element of the destination range.
// \return Output iterator to the element one past the last copied element.
//
// This function transfers the elements in the range \f$ [first,last) \f$ to the specified
// destination range. The destination range is assumed to be uninitialized. In case the elements
// provide a no-throw move assignment operator, the transfer operation is handled via move
// construction, else the elements are copied constructed.
*/
template <typename InputIt, typename ForwardIt>
ForwardIt uninitialized_transfer(InputIt first, InputIt last, ForwardIt dest)
{
	using T = typename std::iterator_traits<InputIt>::value_type;

	if (IsNothrowMoveAssignable_v<T>) {
		return mtrc::numeric::uninitialized_move(first, last, dest);
	} else {
		return std::uninitialized_copy(first, last, dest);
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Transfers the elements from the given source range to the uninitialized destination range.
// \ingroup algorithms
//
// \param first Iterator to the first element of the source range.
// \param n The number of elements to be moved.
// \param dest Iterator to the first element of the destination range.
// \return Output iterator to the element one past the last copied element.
//
// This function transfers the elements in the range \f$ [first,first+n) \f$ to the specified
// destination range. The destination range is assumed to be uninitialized. In case the elements
// provide a no-throw move assignment operator, the transfer operation is handled via move
// construction, else the elements are copied constructed.
*/
template <typename InputIt, typename ForwardIt>
ForwardIt uninitialized_transfer_n(InputIt first, size_t n, ForwardIt dest)
{
	using T = typename std::iterator_traits<InputIt>::value_type;

	if (IsNothrowMoveAssignable_v<T>) {
		return mtrc::numeric::uninitialized_move_n(first, n, dest);
	} else {
		return std::uninitialized_copy_n(first, n, dest);
	}
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
