# Time-Series Space With TWED

Time-series records often need an alignment-aware metric. A shifted or stretched process curve can still be close to a baseline curve, while a flat or structurally different curve should be farther away.

The promoted C++ example [time_series_twed_space.cpp](../../examples/core/time_series_twed_space.cpp) uses `mtrc::TWED` to compare small curves, then builds a `mtrc::MatrixSpace` over those time-series records.

The promoted Python example [time_series_alignment_space.py](../../python/examples/metric_space/time_series_alignment_space.py)
uses the same finite-space pattern with a small alignment-aware callable. It stays on the core wheel path while the compiled TWED binding remains part of the broader legacy/full Python surface.

Core shape:

```cpp
#include <metric/metric/catalog.hpp>
#include <metric/space.hpp>

std::vector<double> baseline = {0, 1, 1, 1, 1, 1, 2, 3};
std::vector<double> shifted = {1, 1, 1, 1, 1, 2, 3, 4};

mtrc::TWED<double> distance(0, 1);
auto value = distance(baseline, shifted);

std::vector<std::vector<double>> records = {baseline, shifted};
mtrc::MatrixSpace<std::vector<double>, mtrc::TWED<double>> space(records, distance);
```

This is a vector-valued record type, but the key point is not Euclidean geometry. The metric defines time-elastic alignment behavior before METRIC builds the finite metric space.
