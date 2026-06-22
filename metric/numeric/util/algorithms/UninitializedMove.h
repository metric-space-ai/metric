// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_ALGORITHMS_UNINITIALIZEDMOVE_H
#define METRIC_NUMERIC_UTIL_ALGORITHMS_UNINITIALIZEDMOVE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <iterator>
#include <memory>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/algorithms/Destroy.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  UNINITIALIZED_MOVE ALGORITHMS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Move the elements from the given source range to the uninitialized destination range.
// \ingroup algorithms
//
// \param first Iterator to the first element of the source range.
// \param last Iterator to the element one past the last element of the source range.
// \param dest Iterator to the first element of the destination range.
// \return Output iterator to the element one past the last copied element.
//
// This function moves the elements from the source range \f$ [first,last) \f$ to the specified
// destination range. The destination range is assumed to be uninitialized, i.e. the elements
// are move constructed.
*/
template <typename InputIt, typename ForwardIt>
ForwardIt uninitialized_move(InputIt first, InputIt last, ForwardIt dest)
{
	using T = typename std::iterator_traits<ForwardIt>::value_type;

	ForwardIt current(dest);

	try {
		for (; first != last; ++first, ++current) {
			::new (std::addressof(*current)) T(std::move(*first));
		}
		return current;
	} catch (...) {
		mtrc::numeric::destroy(dest, current);
		throw;
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Move the elements from the given source range to the uninitialized destination range.
// \ingroup algorithms
//
// \param first Iterator to the first element of the source range.
// \param n The number of elements to be moved.
// \param dest Iterator to the first element of the destination range.
// \return Output iterator to the element one past the last copied element.
//
// This function moves the elements from the source range \f$ [first,first+n) \f$ to the specified
// destination range. The destination range is assumed to be uninitialized, i.e. the elements
// are move constructed.
*/
template <typename InputIt, typename ForwardIt> ForwardIt uninitialized_move_n(InputIt first, size_t n, ForwardIt dest)
{
	using T = typename std::iterator_traits<ForwardIt>::value_type;

	ForwardIt current(dest);

	try {
		for (; n > 0UL; (void)++first, (void)++current, --n) {
			::new (std::addressof(*current)) T(std::move(*first));
		}
		return current;
	} catch (...) {
		mtrc::numeric::destroy(dest, current);
		throw;
	}
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
