# Time-Series Space With TWED

Time-series records often need an alignment-aware metric. A shifted or stretched process curve can still be close to a baseline curve, while a flat or structurally different curve should be farther away.

The promoted C++ example [time_series_twed_space.cpp](../../examples/core/time_series_twed_space.cpp) uses `metric::TWED` to compare small curves, then builds a `metric::MatrixSpace` over those time-series records.

Core shape:

```cpp
#include <metric/distance.hpp>
#include <metric/space.hpp>

std::vector<double> baseline = {0, 1, 1, 1, 1, 1, 2, 3};
std::vector<double> shifted = {1, 1, 1, 1, 1, 2, 3, 4};

metric::TWED<double> distance(0, 1);
auto value = distance(baseline, shifted);

std::vector<std::vector<double>> records = {baseline, shifted};
metric::MatrixSpace<std::vector<double>, metric::TWED<double>> space(records, distance);
```

This is a vector-valued record type, but the key point is not Euclidean geometry. The metric defines time-elastic alignment behavior before METRIC builds the finite metric space.
