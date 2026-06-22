// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_INTERNAL_ASSERTION
#define METRIC_NUMERIC_INTERNAL_ASSERTION 0
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for user assertions.
// \ingroup config
//
// This compilation switch triggers user assertions, which are used to check user specified
// function parameters and values. The user assertions can also be deactivated by defining
// \a NDEBUG during the compilation.
//
// Possible settings for the user assertion switch:
//  - Deactivated: \b 0
//  - Activated  : \b 1
//
// \note It is possible to (de-)activate user assertions via command line or by defining this
// symbol manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_USER_ASSERTION=1 ...
   \endcode

   \code
   #define METRIC_NUMERIC_USER_ASSERTION 1
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_USER_ASSERTION
#define METRIC_NUMERIC_USER_ASSERTION 0
#endif
//*************************************************************************************************
