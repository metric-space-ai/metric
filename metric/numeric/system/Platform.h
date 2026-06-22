// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_PLATFORM_H
#define METRIC_NUMERIC_SYSTEM_PLATFORM_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/StaticAssert.h>

//=================================================================================================
//
//  PLATFORM MACRO DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#if defined(_WIN32)
#define METRIC_NUMERIC_WIN32_PLATFORM 1
#else
#define METRIC_NUMERIC_WIN32_PLATFORM 0
#endif
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#if defined(_WIN64)
#define METRIC_NUMERIC_WIN64_PLATFORM 1
#else
#define METRIC_NUMERIC_WIN64_PLATFORM 0
#endif
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#if defined(__MINGW64_VERSION_MAJOR)
#define METRIC_NUMERIC_MINGW64_PLATFORM 1
#else
#define METRIC_NUMERIC_MINGW64_PLATFORM 0
#endif
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#if defined(__MINGW32__)
#define METRIC_NUMERIC_MINGW32_PLATFORM 1
#else
#define METRIC_NUMERIC_MINGW32_PLATFORM 0
#endif
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  COMPILE TIME CONSTRAINTS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
namespace {

METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_WIN64_PLATFORM ||
							 (!METRIC_NUMERIC_MINGW32_PLATFORM && !METRIC_NUMERIC_MINGW64_PLATFORM));
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_MINGW32_PLATFORM ||
							 (!METRIC_NUMERIC_WIN64_PLATFORM && !METRIC_NUMERIC_MINGW64_PLATFORM));
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_MINGW64_PLATFORM ||
							 (!METRIC_NUMERIC_WIN64_PLATFORM && !METRIC_NUMERIC_MINGW32_PLATFORM));

} // namespace
/*! \endcond */
//*************************************************************************************************

#endif
