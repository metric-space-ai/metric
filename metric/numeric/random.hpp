// METRIC numeric core — curated public facade for the random contracts.
//
// Small METRIC-owned entry point for the promoted mtrc::numeric random surface
// (getSeed/setSeed, rand<T>, randomize, and the Rand<T> type specializations).
// Setting the seed makes the sequence reproducible; the generator state is
// process-global by design. The contracts are first-party METRIC C++ (see
// metric/numeric/README.md for provenance and licensing). This facade declares no
// symbols of its own; it only re-exports promoted core headers.

#ifndef METRIC_NUMERIC_RANDOM_HPP
#define METRIC_NUMERIC_RANDOM_HPP

#include <metric/numeric/util/Random.h>

#endif
