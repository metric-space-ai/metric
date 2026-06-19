# Exact Graph Edge Fixtures

Exact graph helpers emit directed graph-construction results over a finite metric space. The result form carries directed edge tuples plus metadata for validating graph construction semantics before symmetrization or weighting policies are promoted.

The edge tuple shape is `(source_index, target_index, distance)`. Self-loops are excluded. kNN ties are resolved by target record order.

C++ shape:

```cpp
#include <metric/operators.hpp>

#include <cstdlib>
#include <vector>

struct AbsoluteDistance {
    auto operator()(int lhs, int rhs) const -> int
    {
        return std::abs(lhs - rhs);
    }
};

std::vector<int> records = {0, 1, 2, 3, 4};

auto knn_graph = metric::operators::exact_knn_graph(records, AbsoluteDistance{}, 2);
auto radius_graph = metric::operators::exact_radius_graph(records, AbsoluteDistance{}, 1);

auto knn_edges = knn_graph.edges;
auto radius_edges = radius_graph.edges;
```

Python shape:

```python
from metric import exact_knn_graph, exact_radius_graph

records = [0, 1, 2, 3, 4]

def absolute_distance(lhs, rhs):
    return abs(lhs - rhs)

knn_graph = exact_knn_graph(records, absolute_distance, k=2)
radius_graph = exact_radius_graph(records, absolute_distance, radius=1)

knn_edges = knn_graph.edges
radius_edges = radius_graph.edges
```

`knn_graph.metadata` records strategy `exact_knn`, `record_count`, `edge_count`, `directed=True`, `self_loops=False`, `exact=True`, `k`, `edge_payload="metric_distance"`, `symmetrization="none"`, `normalization="none"`, and the tie-breaking rule. `radius_graph.metadata` records the same policy fields with `radius` instead of `k`.

For `records = [0, 1, 2, 3, 4]`, the exact radius graph with radius `1` has directed edges:

```text
(0, 1, 1)
(1, 0, 1)
(1, 2, 1)
(2, 1, 1)
(2, 3, 1)
(3, 2, 1)
(3, 4, 1)
(4, 3, 1)
```

These helpers are exact because they evaluate the pairwise distances needed for every source record. The `*_edges` helpers remain available for callers that only need edge tuples. `metric::KNNGraph` remains an approximate compatibility representation and should not be used as evidence for exact graph construction unless its edge set is compared against dense pairwise distances for the fixture.
