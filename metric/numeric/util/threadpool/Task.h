// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_THREADPOOL_TASK_H
#define METRIC_NUMERIC_UTIL_THREADPOOL_TASK_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <functional>

namespace mtrc::numeric {

namespace threadpool {

//=================================================================================================
//
//  TYPE DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Handle for a single, executable task.
// \ingroup threads
*/
using Task = std::function<void(void)>;
//*************************************************************************************************

} // namespace threadpool

} // namespace mtrc::numeric

#endif
