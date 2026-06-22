// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SMP_DEFAULT_FUNCTIONS_H
#define METRIC_NUMERIC_MATH_SMP_DEFAULT_FUNCTIONS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Inline.h>
#include <metric/numeric/system/SMP.h>
#include <metric/numeric/util/MaybeUnused.h>
#include <metric/numeric/util/StaticAssert.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  SMP UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name SMP utility functions */
//@{
METRIC_NUMERIC_ALWAYS_INLINE size_t getNumThreads();
METRIC_NUMERIC_ALWAYS_INLINE void setNumThreads(size_t number);
METRIC_NUMERIC_ALWAYS_INLINE void shutDownThreads();
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the number of threads used for thread parallel operations.
// \ingroup smp
//
// \return The number of threads used for thread parallel operations.
//
// Via this function the number of threads used for thread parallel operations can be queried.
// Note that in case no parallelization is active the function will always return 1.
*/
METRIC_NUMERIC_ALWAYS_INLINE size_t getNumThreads() { return 1UL; }
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Sets the number of threads to be used for thread parallel operations.
// \ingroup smp
//
// \param number The given number of threads \f$[1..\infty)\f$.
// \return void
// \exception std::invalid_argument Invalid number of threads.
//
// Via this function the maximum number of threads for thread parallel operations can be specified.
// Note that the given \a number must be in the range \f$[1..\infty)\f$. In case an invalid
// number of threads is specified, a \a std::invalid_argument exception is thrown. Also note that
// in case no parallelization is active, the function has no effect.
*/
METRIC_NUMERIC_ALWAYS_INLINE void setNumThreads(size_t number) { MAYBE_UNUSED(number); }
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Provides a reliable shutdown of C++11 threads for Visual Studio compilers.
// \ingroup smp
//
// \return void
//
// There is a known issue in Visual Studio 2012 and 2013 that may cause C++11 threads to hang
// if their destructor is executed after the \c main() function:
//
//    http://connect.microsoft.com/VisualStudio/feedback/details/747145
//
// This function, which has only an effect for Visual Studio compilers, provides a reliable way
// to circumvent this problem. If called directly before the end of the \c main() function it
// blocks until all threads have been destroyed:

   \code
   int main()
   {
	  // ... Using the C++11 thread parallelization of Metric numeric

	  shutDownThreads();
   }
   \endcode
*/
METRIC_NUMERIC_ALWAYS_INLINE void shutDownThreads() {}
//*************************************************************************************************

//=================================================================================================
//
//  COMPILE TIME CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
namespace {

METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_HPX_PARALLEL_MODE);
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_CPP_THREADS_PARALLEL_MODE);
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_BOOST_THREADS_PARALLEL_MODE);
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_OPENMP_PARALLEL_MODE);

} // namespace
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
