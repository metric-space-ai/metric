# Histogram Space With EMD

Histograms and empirical distributions are often better compared by transport cost than by coordinate-wise vector distance. Earth mover distance models how much mass must move between bins and how expensive those bin-to-bin moves are.

METRIC exposes two transport surfaces. `mtrc::Wasserstein<V>` is the **strict true metric** (`metric_law::metric`): it admits only a metric ground cost and equal nonnegative mass, so it can be routed into metric-only algorithms such as cover-tree indexing. `mtrc::EMD<V>` is the older **permissive compatibility** route (`metric_law::distance`): it accepts an arbitrary ground cost and unbalanced mass via an extra-mass penalty, which does not preserve the metric axioms, so it must not be used as a metric. Prefer `mtrc::Wasserstein<V>` for new code; see the [Metric Quarantine Inventory](../metrics/quarantine-inventory.md) for the split rationale.

The promoted C++ example [histogram_emd_space.cpp](../../examples/core/histogram_emd_space.cpp) uses a small one-dimensional ground-cost matrix, then builds a `mtrc::MatrixSpace` over histogram records.

The promoted Python example [histogram_transport_space.py](../../python/examples/metric_space/histogram_transport_space.py)
uses the same metric-space pattern with a deterministic one-dimensional transport callable. It covers the core wheel path without requiring the broader compiled EMD binding.

The CI-tested C++ engine fixture [histogram_transport_space.cpp](../../examples/engine/histogram_transport_space.cpp)
adds the corresponding native transport-vs-vector baseline. The report
[Histogram Transport Baseline](histogram-transport-baseline.md) records 4/4
transport hits and 4/4 raw-vector misses on shifted-mass queries with split-mass
decoys.

Core shape:

```cpp
#include <metric/metric/catalog.hpp>
#include <metric/space.hpp>

#include <vector>

using Histogram = std::vector<double>;

std::vector<std::vector<double>> ground_cost = {
    {0.0, 1.0, 2.0, 3.0},
    {1.0, 0.0, 1.0, 2.0},
    {2.0, 1.0, 0.0, 1.0},
    {3.0, 2.0, 1.0, 0.0},
};

mtrc::EMD<double> emd(ground_cost, 3.0);

std::vector<Histogram> records = {
    {1.0, 0.0, 0.0, 0.0},
    {0.0, 1.0, 0.0, 0.0},
};

mtrc::MatrixSpace<Histogram, mtrc::EMD<double>> space(records, emd);
```

This is still a vector container at the storage level, but the geometry is not a norm on coordinates. The metric defines movement of mass across bins before METRIC builds the finite metric space.

## Strict metric variant

The `ground_cost` above is the line cost `C[i][j] = |i - j|`, which is a true
metric, and the records carry equal unit mass. That is exactly the admitted
domain of `mtrc::Wasserstein<V>`, so the same space can be built with the strict
metric and routed into metric-only algorithms:

```cpp
#include <metric/metric/catalog.hpp>
#include <metric/space.hpp>

#include <vector>

using Histogram = std::vector<double>;

// Validates that the ground cost is a metric (square, zero diagonal, positive
// off-diagonal, symmetric, triangle) at construction.
mtrc::Wasserstein<double> wasserstein(std::vector<std::vector<double>>{
    {0.0, 1.0, 2.0, 3.0},
    {1.0, 0.0, 1.0, 2.0},
    {2.0, 1.0, 0.0, 1.0},
    {3.0, 2.0, 1.0, 0.0},
});

// Equivalent canonical line ground cost C[i][j] = |i - j| for n bins:
auto line_wasserstein = mtrc::Wasserstein<double>::on_line(4);

std::vector<Histogram> records = {
    {1.0, 0.0, 0.0, 0.0},
    {0.0, 1.0, 0.0, 0.0},
};

mtrc::MatrixSpace<Histogram, mtrc::Wasserstein<double>> space(records, wasserstein);
```

The strict route rejects unequal-mass, negative, or non-finite inputs and any
non-metric ground cost with `std::invalid_argument`. There is no extra-mass
penalty: equal mass is required instead. Because it is immutable and declares
`metric_law::metric`, it is safe to evaluate concurrently and to use with
cover-tree indexing and triangle-pruning search.

The value type must be floating point (use `double`/`float`; convert integer
count histograms first). Construction validates the ground metric in `O(n^3)`,
so prefer the line ground cost or small explicit matrices over a large dense
2-D grid cost.
