// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_ALGORITHMS_UNINITIALIZEDVALUECONSTRUCT_H
#define METRIC_NUMERIC_UTIL_ALGORITHMS_UNINITIALIZEDVALUECONSTRUCT_H
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
//  UNINITIALIZED_VALUE_CONSTRUCT ALGORITHMS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Value constructs elements in the given range.
// \ingroup algorithms
//
// \param first Iterator to the first element of the range.
// \param last Iterator to the element one past the last element of the range.
// \return void
//
// This function value constructs elements in the given range \f$ [first,last) \f$. The range
// is assumed to be uninitialized.
*/
template <typename ForwardIt> void uninitialized_value_construct(ForwardIt first, ForwardIt last)
{
	using T = typename std::iterator_traits<ForwardIt>::value_type;

	ForwardIt current(first);

	try {
		for (; current != last; ++current) {
			::new (std::addressof(*current)) T();
		}
	} catch (...) {
		mtrc::numeric::destroy(first, current);
		throw;
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Value constructs elements in the given range.
// \ingroup algorithms
//
// \param first Iterator to the first element of the range.
// \param n The number of elements to be constructed.
// \return void
//
// This function value constructs elements in the given range \f$ [first,first+n) \f$. The range
// is assumed to be uninitialized.
*/
template <typename ForwardIt> void uninitialized_value_construct_n(ForwardIt first, size_t n)
{
	using T = typename std::iterator_traits<ForwardIt>::value_type;

	ForwardIt current(first);

	try {
		for (; n > 0UL; (void)++current, --n) {
			::new (std::addressof(*current)) T();
		}
	} catch (...) {
		mtrc::numeric::destroy(first, current);
		throw;
	}
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
