// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_STANDARD_H
#define METRIC_NUMERIC_SYSTEM_STANDARD_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/StaticAssert.h>

//=================================================================================================
//
//  C++ STANDARD MACRO DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#if (__cplusplus >= 201402L && __cplusplus < 201703L) || (_MSVC_LANG >= 201402L && _MSVC_LANG < 201703L)
#define METRIC_NUMERIC_CPP14_MODE 1
#else
#define METRIC_NUMERIC_CPP14_MODE 0
#endif
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#if (__cplusplus >= 201703L) || (_MSVC_LANG >= 201703L)
#define METRIC_NUMERIC_CPP17_MODE 1
#else
#define METRIC_NUMERIC_CPP17_MODE 0
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

METRIC_NUMERIC_STATIC_ASSERT(METRIC_NUMERIC_CPP14_MODE || METRIC_NUMERIC_CPP17_MODE);
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_CPP14_MODE || !METRIC_NUMERIC_CPP17_MODE);

} // namespace
/*! \endcond */
//*************************************************************************************************

#endif
