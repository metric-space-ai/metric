# Engine Metric Space

The engine starts with a finite record set and a metric callable:

```text
RecordSet + Metric -> MetricSpace
```

`MetricSpace` is the canonical C++ engine object. It owns records by value, assigns stable `RecordId` values, exposes the metric, and carries a `SpaceVersion` counter for representation staleness checks.

Python exposes the same model through `Space`: a finite record set plus a metric callable with cached pairwise distances and intent-named helpers.

## C++

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
```

The record type is not required to be a vector. A metric can operate on strings, histograms, event sequences, process curves, structured records, or vectors.

## Python

```python
from metric import Edit, Space
from metric.core import make_space

records = ["metric", "metrics", "matrix", "tree"]
space = Space(records, Edit())
same_space = make_space(records, Edit())

distance = space.distance(0, 1)
matrix = space.to_matrix()
```

`metric.core` is the Python namespace for the central engine building blocks: `Metric`, `Space`, `FiniteMetricSpace`, runtime policy metadata, and core metric-space errors. The top-level `metric.Space` import remains the shortest path for examples, while `metric.core` gives contributors a stable namespace that mirrors the engine model.

`to_matrix()` makes materialization explicit in Python. It returns an independent finite matrix-space view with the same records and metric.

## Current Contract

- `RecordId` is opaque in C++ but currently backed by dense internal storage.
- `SpaceVersion` is the public C++ version type for mutation and representation freshness checks.
- C++ `MetricSpace` owns records by value in the first engine version.
- Python `Space` stores records as a list and eagerly caches pairwise distances.
- Mutating C++ space state should call `touch()` so cached representations can report stale state.
- Mathematical metric laws are documented and tested by examples, not enforced by the type system.
