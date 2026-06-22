// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_USE_DEBUG_MODE
#define METRIC_NUMERIC_USE_DEBUG_MODE 0
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for function traces.
// \ingroup config
//
// This compilation switch triggers the use of function traces. In case the switch is set to
// 1, function traces via the METRIC_NUMERIC_FUNCTION_TRACE are enabled and trace information is written
// to the console via \c std::cerr.
//
// Possible settings for the function trace switch:
//  - Deactivated: \b 0 (default)
//  - Activated  : \b 1
//
// \note It is possible to (de-)activate function traces via command line or by defining this
// symbol manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_USE_FUNCTION_TRACES=1 ...
   \endcode

   \code
   #define METRIC_NUMERIC_USE_FUNCTION_TRACES 1
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_USE_FUNCTION_TRACES
#define METRIC_NUMERIC_USE_FUNCTION_TRACES 0
#endif
//*************************************************************************************************
