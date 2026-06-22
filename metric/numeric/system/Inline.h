// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_INLINE_H
#define METRIC_NUMERIC_SYSTEM_INLINE_H
//=================================================================================================
//
//  INLINE SETTINGS
//
//=================================================================================================

#include <metric/numeric/config/Inline.h>

//=================================================================================================
//
//  METRIC_NUMERIC_STRONG_INLINE KEYWORD
//
//=================================================================================================

//*************************************************************************************************
/*!\def METRIC_NUMERIC_STRONG_INLINE
// \brief Platform dependent setup of a strengthened inline keyword.
// \ingroup system
*/
#if METRIC_NUMERIC_USE_STRONG_INLINE && (defined(_MSC_VER) || defined(__INTEL_COMPILER))
#define METRIC_NUMERIC_STRONG_INLINE __forceinline
#else
#define METRIC_NUMERIC_STRONG_INLINE inline
#endif
//*************************************************************************************************

//=================================================================================================
//
//  METRIC_NUMERIC_ALWAYS_INLINE KEYWORD
//
//=================================================================================================

//*************************************************************************************************
/*!\def METRIC_NUMERIC_ALWAYS_INLINE
// \brief Platform dependent setup of an enforced inline keyword.
// \ingroup system
*/
#if METRIC_NUMERIC_USE_ALWAYS_INLINE && defined(__GNUC__)
#define METRIC_NUMERIC_ALWAYS_INLINE __attribute__((always_inline)) inline
#else
#define METRIC_NUMERIC_ALWAYS_INLINE METRIC_NUMERIC_STRONG_INLINE
#endif
//*************************************************************************************************

#endif
