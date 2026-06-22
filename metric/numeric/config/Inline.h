// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_USE_STRONG_INLINE
#define METRIC_NUMERIC_USE_STRONG_INLINE 1
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for an enforced inline keyword.
// \ingroup config
//
// Although the METRIC_NUMERIC_STRONG_INLINE keyword improves the likelihood of a function being inlined it
// does not provide a 100% guarantee. Depending on the availability of an according keyword and/or
// modifier on a specific platform, this guarantee is provided by the METRIC_NUMERIC_ALWAYS_INLINE keyword,
// which uses platform-specific functionality to enforce the inlining of a function.
//
// This compilation switch enables/disables the METRIC_NUMERIC_ALWAYS_INLINE keyword. When disabled or in
// case the platform does not provide a keyword and/or modifier for a 100% inline guarantee, the
// METRIC_NUMERIC_ALWAYS_INLINE keyword uses the METRIC_NUMERIC_STRONG_INLINE keyword as fallback. Possible settings
// for the switch are:
//  - Deactivated: \b 0
//  - Activated  : \b 1
//
// Note that it is possible to (de-)activate the enforced inline keyword via command line or by
// defining this symbol manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_USE_ALWAYS_INLINE=1 ...
   \endcode

   \code
   #define METRIC_NUMERIC_USE_ALWAYS_INLINE 1
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_USE_ALWAYS_INLINE
#define METRIC_NUMERIC_USE_ALWAYS_INLINE 1
#endif
//*************************************************************************************************
