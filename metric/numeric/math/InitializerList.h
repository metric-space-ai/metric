// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_INITIALIZERLIST_H
#define METRIC_NUMERIC_MATH_INITIALIZERLIST_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/shims/IsDefault.h>
#include <metric/numeric/util/InitializerList.h>
#include <metric/numeric/util/algorithms/Max.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Determines the number of non-zero elements contained in the given initializer list.
// \ingroup math
//
// \param list The given initializer list
// \return The number of non-zeros elements.
*/
template <typename Type> inline size_t nonZeros(initializer_list<Type> list) noexcept
{
	size_t nonzeros(0UL);

	for (const Type &element : list) {
		if (!isDefault<strict>(element))
			++nonzeros;
	}

	return nonzeros;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Determines the number of non-zero elements contained in the given initializer list.
// \ingroup math
//
// \param list The given initializer list
// \return The number of non-zeros elements.
*/
template <typename Type> inline size_t nonZeros(initializer_list<initializer_list<Type>> list) noexcept
{
	size_t nonzeros(0UL);

	for (const auto &rowList : list) {
		nonzeros += nonZeros(rowList);
	}

	return nonzeros;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Determines the maximum number of columns specified by the given initializer list.
// \ingroup math
//
// \param list The given initializer list
// \return The maximum number of columns.
*/
template <typename Type> constexpr size_t determineColumns(initializer_list<initializer_list<Type>> list) noexcept
{
	size_t cols(0UL);

	for (const auto &rowList : list)
		cols = max(cols, rowList.size());

	return cols;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
