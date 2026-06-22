// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_CACHESIZE_H
#define METRIC_NUMERIC_SYSTEM_CACHESIZE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/config/CacheSize.h>
#include <metric/numeric/util/StaticAssert.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CACHE SIZE
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
constexpr size_t cacheSize = METRIC_NUMERIC_CACHE_SIZE;
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

METRIC_NUMERIC_STATIC_ASSERT(mtrc::numeric::cacheSize > 100000UL && mtrc::numeric::cacheSize < 100000000UL);

}
/*! \endcond */
//*************************************************************************************************

#endif
