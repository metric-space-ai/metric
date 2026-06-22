// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_ALGORITHMS_DESTROY_H
#define METRIC_NUMERIC_UTIL_ALGORITHMS_DESTROY_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <memory>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/algorithms/DestroyAt.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  DESTROY ALGORITHMS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Destroys the given range of objects .
// \ingroup algorithms
//
// \param first Iterator to the first element to be destroyed.
// \param last Iterator to the element one past the last element to be destroyed.
// \return void
//
// This function explicitly calls the destructor of all object in the given range.
*/
template <typename ForwardIt> void destroy(ForwardIt first, ForwardIt last)
{
	for (; first != last; ++first) {
		mtrc::numeric::destroy_at(std::addressof(*first));
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Destroys the given range of objects .
// \ingroup algorithms
//
// \param first Iterator to the first element to be destroyed.
// \param n The number of elements to be destroyed.
// \return void
//
// This function explicitly calls the destructor of all object in the given range.
*/
template <typename ForwardIt> void destroy_n(ForwardIt first, size_t n)
{
	for (; n > 0UL; (void)++first, --n) {
		mtrc::numeric::destroy_at(std::addressof(*first));
	}
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
