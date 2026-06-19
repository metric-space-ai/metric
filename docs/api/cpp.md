# C++ API

The current C++ core API exposes metric constructors, a minimal `metric::Metric` wrapper for custom callables, a minimal `metric::Space` facade, and explicit finite-space representations. The broader intent facade is part of the engine roadmap; promoted examples use the CI-tested core surface.

Recommended includes:

```cpp
#include <metric/concepts.hpp>
#include <metric/distance.hpp>
#include <metric/operators.hpp>
#include <metric/space.hpp>
```

## Finite Metric Space

```cpp
#include <metric/distance.hpp>
#include <metric/space.hpp>

#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> records = {"cat", "cot", "coat", "dog"};
auto space = metric::Space::from_records(records, metric::Edit<std::string>{});

auto nearest = space.neighbors(std::string("cut"), 2);
std::cout << records[nearest[0].first] << ": " << nearest[0].second << "\n";
```

The record type is not required to be a vector. The metric defines the geometry.

## Metric Callables

Any callable accepted by the target operator can be used as a metric. The revived C++ API also exposes a small `metric::Metric<Record, Callable>` wrapper when code wants a named metric object with explicit record and distance types:

```cpp
#include <metric/concepts.hpp>

struct PaddedHamming {
    auto operator()(const std::string &lhs, const std::string &rhs) const -> std::size_t;
};

static_assert(metric::is_metric_callable_v<PaddedHamming, std::string>);

auto distance = metric::make_metric<std::string>(PaddedHamming{});
auto space = metric::Space::from_records(records, distance);
```

`metric::Metric<Record, Callable>` is a typed callable adapter. It does not validate the mathematical axioms at compile time; runtime tests and documented operator assumptions still define whether non-negativity, identity, symmetry, or the triangle inequality are required.

## Operators

Use the free operator helpers when a workflow does not need to keep a `Space` object:

```cpp
#include <metric/distance.hpp>
#include <metric/operators.hpp>

#include <string>
#include <vector>

std::vector<std::string> records = {"cat", "cot", "coat", "dog"};

auto distances = metric::operators::pairwise_distance_matrix(records, metric::Edit<std::string>{});
auto nearest = metric::operators::nearest_neighbors(records, metric::Edit<std::string>{}, std::string("cut"), 2);
auto close = metric::operators::range_neighbors(records, metric::Edit<std::string>{}, std::string("cut"), 1);
auto selected_ids = metric::operators::representative_indices(records, metric::Edit<std::string>{}, 2);
auto selected_records = metric::operators::representatives(records, metric::Edit<std::string>{}, 2);
auto dimension = metric::operators::intrinsic_dimension(records, metric::Edit<std::string>{});
```

The promoted C++ operator helpers are `pairwise_distance_matrix`, `nearest_neighbors`, `range_neighbors`, `representative_indices`, `representatives`, and `intrinsic_dimension`.

`representative_indices` and `representatives` use deterministic farthest-first traversal over the finite metric space. They select existing records rather than vector centroids, start from `seed_index=0` by default, and resolve equal-distance ties by record order.

`intrinsic_dimension` returns an expansion-dimension estimate based on neighborhood growth. It is a finite-space diagnostic, not an exact manifold dimension.

## Custom Metrics

A custom metric is any callable object accepted by the target operator:

```cpp
struct PaddedHamming {
    auto operator()(const std::string &lhs, const std::string &rhs) const -> double;
};

auto space = metric::Space::from_records(records, PaddedHamming{});
```

Algorithms that require metric-space guarantees assume non-negativity, identity, symmetry, and the triangle inequality. Operators that can work with weaker distances document those assumptions.

## Representations

The core can materialize explicit representations when a workflow needs control over memory, speed, approximation, or reproducibility.

- `metric::MatrixSpace<Record, Metric>`
- `metric::GraphSpace<Record, Metric>`
- `metric::TreeSpace<Record, Metric>`

`metric::MatrixSpace` stores the full pairwise matrix. `metric::GraphSpace` stores a sparse nearest-neighbor graph. `metric::TreeSpace` provides tree-based neighbor access.

## Engine Roadmap

The implemented C++ facade currently covers finite-space construction, neighbor access through `metric::Space::from_records` and `neighbors`, and free operator helpers for pairwise distances and neighbor queries. Additional intent names such as `groups`, `embed`, `map`, `reduce`, `denoise`, `outliers`, and `compare` describe the public direction and should be promoted only when they are backed by stable strategies, result objects, examples, and CI.

## Compatibility Names

The following names remain available for existing users:

- `metric::Matrix<Record, Metric>`
- `metric::Tree<Record, Metric>`
- `metric::KNNGraph<Record, Metric>`
- `metric::Manhatten<T>`

New code uses the engine vocabulary and `*Space` names unless it intentionally targets the lower-level compatibility surface.
