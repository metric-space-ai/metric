// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_SMP_H
#define METRIC_NUMERIC_SYSTEM_SMP_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/config/SMP.h>

// METRIC currently exposes a deterministic serial numeric core. Parallel
// execution must be introduced through a native METRIC execution policy, not by
// auto-selecting inherited backend branches from compiler macros.
#define METRIC_NUMERIC_OPENMP_PARALLEL_MODE 0
#define METRIC_NUMERIC_HPX_PARALLEL_MODE 0
#define METRIC_NUMERIC_CPP_THREADS_PARALLEL_MODE 0
#define METRIC_NUMERIC_BOOST_THREADS_PARALLEL_MODE 0

#endif
