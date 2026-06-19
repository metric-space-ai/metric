# Engine Mental Model

The engine starts with one object:

```text
RecordSet + Metric -> MetricSpace
```

`MetricSpace` owns the finite record set, the metric callable, stable `RecordId` values, and version state. Representations, operators, intents, strategies, mappings, and runtime policies are built around that object instead of replacing it.

## Phase 1 Surface

The first C++ engine skeleton introduces:

- `metric::RecordId`
- `metric::Neighbor<Distance>`
- `metric::metric_traits`
- `metric::MetricCallable`
- `metric::MetricSpaceLike`
- `metric::MetricSpace<Record, Metric>`
- `metric::make_space(records, metric)`

This is intentionally small. It does not migrate legacy algorithms, add Python bindings, or replace existing compatibility classes. It establishes the central object that later representation adapters and intent APIs can use.

## Phase 2 Surface

The first representation adapters live under `metric::representations`:

- `ImplicitDistanceProvider<Space>`
- `MatrixCache<Space>`
- `CoverTreeIndex<Space>`
- `KnnGraphIndex<Space>`
- `GraphTopology<Space>`

They are views or materialized execution structures over the same `MetricSpace`; they do not own or redefine the record set. Each adapter captures the source space version when it is created and exposes `is_stale()` so later mutable space operations can invalidate cached or indexed representations explicitly.

`CoverTreeIndex` and `KnnGraphIndex` currently provide exact deterministic neighbor ordering over the finite space while preserving the intended engine vocabulary. The implementation can be replaced by specialized tree or graph algorithms later without changing the representation role.

## Current Contract

The initial `MetricSpace` owns records by value. `RecordId` is an opaque wrapper over dense internal storage. `version()` exists and starts at `0`; later representation adapters can use it for stale-cache checks when mutating space operations are introduced.

Distance evaluation stays direct:

```cpp
#include <metric/distance.hpp>
#include <metric/engine.hpp>

#include <string>
#include <vector>

std::vector<std::string> records = {"metric", "metrics", "matrix", "tree"};
auto space = metric::make_space(records, metric::Edit<char>{});

auto lhs = space.id(0);
auto rhs = space.id(1);
auto distance = space.distance(lhs, rhs);

metric::representations::MatrixCache<decltype(space)> matrix(space);
auto cached_distance = matrix.distance(lhs, rhs);

metric::representations::CoverTreeIndex<decltype(space)> tree(space);
auto neighbors = tree.knn(std::string("metricks"), 2);
```

The important shift is vocabulary: engine code starts from a metric space and stable record IDs. Algorithm names and representation choices come later as strategies and execution structures. A representation reports stale state when the source space version changes:

```cpp
auto before = matrix.is_stale(); // false
space.touch();
auto after = matrix.is_stale(); // true
```
