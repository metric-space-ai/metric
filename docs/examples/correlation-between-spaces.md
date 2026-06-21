# Correlation Between Metric Spaces

METRIC can compare paired observations from two different metric spaces without forcing both sides into the same vector representation first. This is useful when two modalities share observation IDs but need different native metrics.

Examples:

- process curves compared with a time-series metric
- event sequences compared with edit distance
- image or histogram records compared with a structured metric
- numeric sensor summaries compared with a norm-induced metric

## Pattern

1. Keep the paired datasets aligned by observation.
2. Choose one metric for the first record type.
3. Choose another metric for the second record type.
4. Run `compare` with a cross-space strategy such as MGC.

The promoted C++ example [metric_space_correlation.cpp](../../examples/core/metric_space_correlation.cpp) runs in the core CI path and compares paired observations from two different record containers and metric types.
The engine hero fixture [cross_space_mgc.cpp](../../examples/engine/cross_space_mgc.cpp)
adds a raw cross-modal vector-nearest-neighbor pairing baseline, where the
baseline misses most nonlinear paired records while MGC still detects the
cross-space dependency.

## Target Python Shape

```python
from metric import Space, metrics

curves = Space(process_curves, metric=metrics.TWED())
events = Space(event_sequences, metric=metrics.Edit())

result = curves.compare(events, method="mgc")
print(result.score)
print(result.p_value)
```

The intent-first Python shape is the target API. The current core wheel does not promote MGC bindings yet.

## Current C++ Shape

```cpp
#include <metric/correlation.hpp>
#include <metric/distance.hpp>

#include <array>
#include <deque>
#include <vector>

using FirstRecord = std::vector<double>;
using SecondRecord = std::array<float, 1>;

std::vector<FirstRecord> first = {{0.0}, {1.0}, {2.0}, {3.0}};
std::deque<SecondRecord> second = {{0.0F}, {1.2F}, {1.9F}, {3.1F}};

metric::MGC<
    FirstRecord,
    metric::Euclidean<double>,
    SecondRecord,
    metric::Manhattan<float>
> mgc;

double score = mgc(first, second);
```

The two datasets do not need to share a record type or distance function. They only need paired observations so the operator can compare the two induced metric spaces.
