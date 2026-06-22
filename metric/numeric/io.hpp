// METRIC numeric core — curated public facade for the io / serialization contracts.
//
// Small METRIC-owned entry point for the promoted mtrc::numeric serialization
// surface (the self-contained Archive plus the dense matrix/vector serializers).
// The serialization core is dependency-free: it uses no external library. The
// contracts are first-party METRIC C++ (see metric/numeric/README.md for
// provenance and licensing). This facade declares no symbols of its own; it only
// re-exports promoted core headers.

#ifndef METRIC_NUMERIC_IO_HPP
#define METRIC_NUMERIC_IO_HPP

#include <metric/numeric/util/Serialization.h>
#include <metric/numeric/math/serialization/MatrixSerializer.h>
#include <metric/numeric/math/serialization/VectorSerializer.h>

#endif
