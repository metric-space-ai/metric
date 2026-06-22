// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_BLOCKING_H
#define METRIC_NUMERIC_SYSTEM_BLOCKING_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Debugging.h>
#include <metric/numeric/util/StaticAssert.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  BLOCKING SETTINGS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
constexpr size_t DEFAULT_BLOCK_SIZE = 256UL;

constexpr size_t MMM_DEFAULT_OUTER_BLOCK_SIZE = 112UL;
constexpr size_t MMM_DEFAULT_INNER_BLOCK_SIZE = 96UL;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
constexpr size_t DEBUG_BLOCK_SIZE = 8UL;

constexpr size_t MMM_DEBUG_OUTER_BLOCK_SIZE = 16UL;
constexpr size_t MMM_DEBUG_INNER_BLOCK_SIZE = 16UL;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
constexpr size_t BLOCK_SIZE = (METRIC_NUMERIC_DEBUG_MODE ? DEBUG_BLOCK_SIZE : DEFAULT_BLOCK_SIZE);

constexpr size_t MMM_OUTER_BLOCK_SIZE =
	(METRIC_NUMERIC_DEBUG_MODE ? MMM_DEBUG_OUTER_BLOCK_SIZE : MMM_DEFAULT_OUTER_BLOCK_SIZE);
constexpr size_t MMM_INNER_BLOCK_SIZE =
	(METRIC_NUMERIC_DEBUG_MODE ? MMM_DEBUG_INNER_BLOCK_SIZE : MMM_DEFAULT_INNER_BLOCK_SIZE);
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

//=================================================================================================
//
//  COMPILE TIME CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
namespace {

METRIC_NUMERIC_STATIC_ASSERT(mtrc::numeric::BLOCK_SIZE >= 4UL);

METRIC_NUMERIC_STATIC_ASSERT(mtrc::numeric::MMM_OUTER_BLOCK_SIZE >= 16UL &&
							 mtrc::numeric::MMM_OUTER_BLOCK_SIZE % 16UL == 0UL);
METRIC_NUMERIC_STATIC_ASSERT(mtrc::numeric::MMM_INNER_BLOCK_SIZE >= 16UL &&
							 mtrc::numeric::MMM_INNER_BLOCK_SIZE % 16UL == 0UL);

} // namespace
/*! \endcond */
//*************************************************************************************************

#endif
