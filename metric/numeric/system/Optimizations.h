// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_OPTIMIZATIONS_H
#define METRIC_NUMERIC_SYSTEM_OPTIMIZATIONS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/config/Optimizations.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  OPTIMIZATION SETTINGS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
constexpr bool usePadding = METRIC_NUMERIC_USE_PADDING;
constexpr bool useStreaming = METRIC_NUMERIC_USE_STREAMING;
constexpr bool useOptimizedKernels = METRIC_NUMERIC_USE_OPTIMIZED_KERNELS;
constexpr bool useDefaultInitialization = METRIC_NUMERIC_USE_DEFAULT_INITIALIZATION;
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
