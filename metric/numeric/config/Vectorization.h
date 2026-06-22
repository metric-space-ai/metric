// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_USE_VECTORIZATION
#define METRIC_NUMERIC_USE_VECTORIZATION 1
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for enabling/disabling vectorization by means of the Sleef library.
// \ingroup config
//
// For several complex operations Metric numeric can make use of the Sleef library for vectorization
// (https://github.com/shibatch/sleef). This compilation switch enables/disables the vectorization
// by means of Sleef. In case the switch is set to 1, Metric numeric uses Sleef for instance for the
// vectorized computation of trigonometric functions (i.e. \c sin(), \c cos(), \c tan(), etc.)
// and exponential functions (i.e. \c exp(), \c log(), ...).
//
// Possible settings for the Sleef switch:
//  - Deactivated: \b 0 (default)
//  - Activated  : \b 1
//
// \note It is possible to enable/disable Sleef vectorization via command line or by defining
// this symbol manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_USE_SLEEF=1 ...
   \endcode

   \code
   #define METRIC_NUMERIC_USE_SLEEF 1
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_USE_SLEEF
#define METRIC_NUMERIC_USE_SLEEF 0
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for enabling/disabling vectorization by means of the XSIMD library.
// \ingroup config
//
// For several complex operations Metric numeric can make use of the XSIMD library for vectorization
// (https://github.com/xtensor-stack/xsimd). This compilation switch enables/disables the
// vectorization by means of XSIMD. In case the switch is set to 1, Metric numeric uses XSIMD for instance
// for the vectorized computation of trigonometric functions (i.e. \c sin(), \c cos(), \c tan(),
// etc.) and exponential functions (i.e. \c exp(), \c log(), ...).
//
// Possible settings for the XSIMD switch:
//  - Deactivated: \b 0 (default)
//  - Activated  : \b 1
//
// \note It is possible to enable/disable XSIMD vectorization via command line or by defining
// this symbol manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_USE_XSIMD=1 ...
   \endcode

   \code
   #define METRIC_NUMERIC_USE_XSIMD 1
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_USE_XSIMD
#define METRIC_NUMERIC_USE_XSIMD 0
#endif
//*************************************************************************************************
