// METRIC numeric core — curated public facade for the parallel-execution contract.
//
// Small METRIC-owned entry point for the promoted mtrc::numeric shared-memory
// parallel (SMP) surface. The default build is single-threaded: the parallel
// backends are compile-time switches that are off by default, so including this
// header introduces no threading dependency. The contracts are first-party METRIC
// C++ (see metric/numeric/README.md for provenance and licensing). This facade
// declares no symbols of its own; it only re-exports promoted core headers.
//
// Note: the generic Thread / ThreadPool utilities under metric/numeric/util are
// internal machinery with no METRIC caller and are intentionally not promoted here.

#ifndef METRIC_NUMERIC_PARALLEL_HPP
#define METRIC_NUMERIC_PARALLEL_HPP

#include <metric/numeric/math/SMP.h>

#endif
