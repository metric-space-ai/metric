// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_FUNCTIONTRACE_H
#define METRIC_NUMERIC_UTIL_FUNCTIONTRACE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Debugging.h>
#if METRIC_NUMERIC_USE_FUNCTION_TRACES
#include <metric/numeric/util/functiontrace/FunctionTrace.h>
#endif

//=================================================================================================
//
//  METRIC_NUMERIC_FUNCTION_TRACE MACRO
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Function trace macro.
// \ingroup util
//
// This macro can be used to reliably trace function calls. In case function tracing is
// activated, function traces are written to the console via \c std::cerr. The following
// short example demonstrates how the function trace macro is used:

   \code
   int main( int argc, char** argv )
   {
	  METRIC_NUMERIC_FUNCTION_TRACE;

	  // ...
   }
   \endcode

// The macro should be used as the very first statement inside the function in order to
// guarantee that writing the function trace is the very first and last action of the
// function call.\n
// Function tracing can be enabled or disabled via the METRIC_NUMERIC_USE_FUNCTION_TRACES macro.
// If function tracing is activated, trace information of the following form will be written
// to \c std::cerr:

   \code
   + [Thread 0] Entering function 'int main()' in file 'TraceDemo.cpp'
   - [Thread 0] Leaving function 'int main()' in file 'TraceDemo.cpp'
   \endcode

// In case function tracing is deactivated, all function trace functionality is completely
// removed from the code, i.e. no function traces are logged and no overhead results from
// the METRIC_NUMERIC_FUNCTION_TRACE macro.
*/
#if METRIC_NUMERIC_USE_FUNCTION_TRACES
#define METRIC_NUMERIC_FUNCTION_TRACE                                                                                  \
	mtrc::numeric::FunctionTrace METRIC_NUMERIC_FUNCTION_TRACE_OBJECT(__FILE__, __func__)
#else
#define METRIC_NUMERIC_FUNCTION_TRACE
#endif
//*************************************************************************************************

#endif
