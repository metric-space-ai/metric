// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TIMING_TIMING_H
#define METRIC_NUMERIC_UTIL_TIMING_TIMING_H
namespace mtrc::numeric {

//=================================================================================================
//
//  DOXYGEN DOCUMENTATION
//
//=================================================================================================

//*************************************************************************************************
//! Namespace for the time measurement module.
namespace timing {
}
//*************************************************************************************************

//*************************************************************************************************
/*!\defgroup timing Time measurement
// \ingroup util
//
// \image html clock.png
// \image latex clock.eps "Timing submodule" width=200pt
//
// The timing submodule offers the necessary functionality for timing and benchmarking purposes.
// The central element of the timing module is the Timer class. Depending on a chosen timing
// policy, this class offers the possibility to measure both single times and time series. In
// order to make time measurement as easy as possible, the Metric numeric library offers the two classes
// WcTimer and CpuTimer (both using the Timer class) to measure both wall clock and CPU time.
// The following example gives an impression on how time measurement for a single time works
// with the the Metric numeric library. Note that in this example the WcTimer could be easily replaced
// with the CpuTimer if instead of the wall clock time the CPU time was to be measured.

   \code
   // Creating a new wall clock timer immediately starts a new time measurement
   WcTimer timer;

   ...  // Programm or code fragment to be measured

   // Stopping the time measurement
   timer.end();

   // Evaluation of the measured time
   double time = timer.last();
   \endcode

// As already mentioned, it is also possible to start several time measurements with a single
// timer to evaluate for instance the minimal, the maximal or the average time of a specific
// task. The next example demonstrates a possible setup for such a series of time measurements:

   \code
   // Creating a new wall clock timer
   WcTimer timer;

   ...  // Additional setup code

   // Starting 10 wall clock time measurements
   for( unsigned int i=0; i<10; ++i ) {
	  timer.start();
	  ...  // Programm or code fragment to be measured
	  timer.end();
   }

   // After the measurements, the desired timing results can be calculated, as for instance the
   // average wall clock time
   double average = timer.average();
   \endcode
*/
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
