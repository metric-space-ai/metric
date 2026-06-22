// METRIC numeric core — curated public facade for the dense vector contracts.
//
// Small METRIC-owned entry point that groups the promoted mtrc::numeric dense
// vector containers (dynamic, fixed-size, hybrid, custom, uniform, zero, and
// initializer vectors) so callers can include exactly the surface they need. The
// contracts are first-party METRIC C++ (see metric/numeric/README.md for
// provenance and licensing). This facade declares no symbols of its own; it only
// re-exports promoted core headers.

#ifndef METRIC_NUMERIC_VECTOR_HPP
#define METRIC_NUMERIC_VECTOR_HPP

#include <metric/numeric/math/TransposeFlag.h>
#include <metric/numeric/math/AlignmentFlag.h>
#include <metric/numeric/math/PaddingFlag.h>
#include <metric/numeric/math/DynamicVector.h>
#include <metric/numeric/math/StaticVector.h>
#include <metric/numeric/math/HybridVector.h>
#include <metric/numeric/math/CustomVector.h>
#include <metric/numeric/math/UniformVector.h>
#include <metric/numeric/math/ZeroVector.h>
#include <metric/numeric/math/InitializerVector.h>

#endif
