// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_USE_PADDING
#define METRIC_NUMERIC_USE_PADDING 1
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Configuration of the streaming behavior.
// \ingroup config
//
// Via this compilation switch streaming (i.e. non-temporal stores) can be (de-)activated. For
// large vectors and matrices non-temporal stores can provide a significant performance advantage
// of about 20%. However, this advantage is only in effect in case the memory bandwidth of the
// target architecture is maxed out. If the target architecture's memory bandwidth cannot be
// exhausted the use of non-temporal stores can decrease performance instead of increasing it.
//
// Possible settings for streaming:
//  - Disabled: \b 0
//  - Enabled : \b 1
//
// \note It is possible to (de-)activate streaming via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_USE_STREAMING=1 ...
   \endcode

   \code
   #define METRIC_NUMERIC_USE_STREAMING 1
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_USE_STREAMING
#define METRIC_NUMERIC_USE_STREAMING 1
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Configuration switch for optimized kernels.
// \ingroup config
//
// This configuration switch enables/disables all optimized compute kernels of the Metric numeric library,
// including all vectorized and data type depending kernels. In case the switch is set to 1 the
// optimized kernels are used whenever possible. In case the switch is set to 0 all optimized
// kernels are not used, even if it would be possible.
//
// Possible settings for the optimized kernels:
//  - Disabled: \b 0
//  - Enabled : \b 1
//
// \warning Note that disabling the optimized kernels causes a severe performance limitiation
// to nearly all operations!
//
// \note It is possible to (de-)activate the optimized kernels via command line or by defining
// this symbol manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_USE_OPTIMIZED_KERNELS=1 ...
   \endcode

   \code
   #define METRIC_NUMERIC_USE_OPTIMIZED_KERNELS 1
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_USE_OPTIMIZED_KERNELS
#define METRIC_NUMERIC_USE_OPTIMIZED_KERNELS 1
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Configuration switch for the initialization in default constructors.
// \ingroup config
//
// This configuration switch enables/disables the element initialization in the default
// constructors of the \a StaticVector and \a StaticMatrix class templates. In case the switch
// is set to 1 all elements are initialized to their respective default. In case the switch is
// set to 0 the default initialization is skipped and the elements are not initialized. Please
// note that this switch is only effective in case the elements are of fundamental type (i.e.
// integral or floating point). In case the elements are of class type, this switch has no effect.
//
// Possible settings for the default initialization:
//  - Disabled: \b 0
//  - Enabled : \b 1
//
// \note It is possible to (de-)activate the default initialization via command line or by
// defining this symbol manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_USE_DEFAULT_INITIALIZATION=1 ...
   \endcode

   \code
   #define METRIC_NUMERIC_USE_DEFAULT_INITIALIZATION 1
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_USE_DEFAULT_INITIALIZATION
#define METRIC_NUMERIC_USE_DEFAULT_INITIALIZATION 1
#endif
//*************************************************************************************************
