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
The engine workflow fixture [cross_space_mgc.cpp](../../examples/engine/cross_space_mgc.cpp)
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

The intent-first Python shape is the target API. The current core wheel does not promote MGC bindings yet. Note that `result.p_value` belongs to that target shape: the native C++ MGC core computes **only the sample statistic**, not a significance test (see the statistical-semantics note below). The [Cross-Space Dependency (Hero)](cross-space-dependency.md) example shows how to realise `result.p_value` today — a seeded permutation test built entirely on top of the public `compare()` intent, without touching MGC internals or treating MGC as a distance.

## Current C++ Shape

```cpp
#include <metric/correlation.hpp>
#include <metric/metric/catalog.hpp>

#include <array>
#include <deque>
#include <vector>

using FirstRecord = std::vector<double>;
using SecondRecord = std::array<float, 1>;

std::vector<FirstRecord> first = {{0.0}, {1.0}, {2.0}, {3.0}};
std::deque<SecondRecord> second = {{0.0F}, {1.2F}, {1.9F}, {3.1F}};

mtrc::MGC<
    FirstRecord,
    mtrc::Euclidean<double>,
    SecondRecord,
    mtrc::Manhattan<float>
> mgc;

double score = mgc(first, second);
```

The two datasets do not need to share a record type or distance function. They only need paired observations so the operator can compare the two induced metric spaces.

## Statistical Semantics

MGC is a **dependence statistic, not a metric**. The native C++ result carries a single sample MGC statistic, with these properties (pinned by `tests/core_smoke/metric_mgc_properties_smoke.cpp`):

- The statistic lies in `[-1, 1]`: `~1` indicates strong (possibly nonlinear or monotone) dependence, `~0` indicates no detected dependence. Any exact monotone transform of one side keeps the statistic at `1`.
- A constant (zero-distance) space yields `0`.
- It is symmetric in practice (`mgc(X, Y) == mgc(Y, X)` for the tested fixtures), though exact symmetry is an observed property of the implementation rather than a guarantee of its region-selection step.
- **No p-value or significance test is computed.** `CorrelationResult` (C++) exposes only the statistic; there is no permutation test in the core. Significance testing is a separate, not-yet-promoted concern.
- Both spaces must be aligned by observation and contain the same number of records (at least two). Very small samples (`n <= 4`) make the significant-region heuristic degenerate, so the statistic falls back to the maximal-scale correlation and should be treated as weak.
