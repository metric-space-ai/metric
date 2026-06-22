// METRIC numeric core — curated public facade for the scalar contracts.
//
// Small METRIC-owned entry point that groups the promoted mtrc::numeric scalar
// contracts (numeric accuracy constants, element shims, and complex helpers) so
// callers can include exactly the surface they need. The contracts are first-party
// METRIC C++ (see metric/numeric/README.md for provenance and licensing). This
// facade declares no symbols of its own; it only re-exports promoted core headers.

#ifndef METRIC_NUMERIC_SCALAR_HPP
#define METRIC_NUMERIC_SCALAR_HPP

#include <metric/numeric/math/Accuracy.h>
#include <metric/numeric/math/Epsilon.h>
#include <metric/numeric/math/Infinity.h>
#include <metric/numeric/math/Shims.h>
#include <metric/numeric/math/Functors.h>
#include <metric/numeric/util/Complex.h>

#endif
