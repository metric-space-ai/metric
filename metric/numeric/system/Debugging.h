// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_DEBUGGING_H
#define METRIC_NUMERIC_SYSTEM_DEBUGGING_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/config/Debugging.h>

//=================================================================================================
//
//  OPENMP MODE CONFIGURATION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#if METRIC_NUMERIC_USE_DEBUG_MODE && !defined(NDEBUG)
#define METRIC_NUMERIC_DEBUG_MODE 1
#else
#define METRIC_NUMERIC_DEBUG_MODE 0
#endif
/*! \endcond */
//*************************************************************************************************

#endif
