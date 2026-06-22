// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_RESTRICT_H
#define METRIC_NUMERIC_SYSTEM_RESTRICT_H
//=================================================================================================
//
//  RESTRICT SETTINGS
//
//=================================================================================================

#include <metric/numeric/config/Restrict.h>

//=================================================================================================
//
//  RESTRICT KEYWORD
//
//=================================================================================================

//*************************************************************************************************
/*!\def METRIC_NUMERIC_RESTRICT
// \brief Platform dependent setup of the restrict keyword.
// \ingroup system
*/
#if METRIC_NUMERIC_USE_RESTRICT

// Intel compiler
#if defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) || defined(__ECC)
#define METRIC_NUMERIC_RESTRICT __restrict

// GNU compiler
#elif defined(__GNUC__)
#define METRIC_NUMERIC_RESTRICT __restrict

// Microsoft visual studio
#elif defined(_MSC_VER)
#define METRIC_NUMERIC_RESTRICT __restrict

// All other compilers
#else
#define METRIC_NUMERIC_RESTRICT

#endif
#else
#define METRIC_NUMERIC_RESTRICT
#endif
//*************************************************************************************************

#endif
