# Exact Graph Edge Fixtures

Exact graph helpers emit graph-construction results over a finite metric space. The result form carries edge tuples plus metadata for validating construction, symmetrization, and reciprocal weighting semantics.

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

auto knn_graph = metric::operators::exact_knn_graph(records, AbsoluteDistance{}, 1);
auto radius_graph = metric::operators::exact_radius_graph(records, AbsoluteDistance{}, 1);

auto knn_edges = knn_graph.edges;
auto radius_edges = radius_graph.edges;
auto degree_info = metric::operators::graph_degree_diagnostics(knn_graph);
auto undirected = metric::operators::symmetrize_graph(knn_graph, "union", "minimum_distance");
auto pruned = metric::operators::prune_graph_out_degree(
    metric::operators::exact_knn_graph(records, AbsoluteDistance{}, 2),
    1);
```

Python shape:

```python
from metric import exact_knn_graph, exact_radius_graph, graph_degree_diagnostics, prune_graph_out_degree, symmetrize_graph

records = [0, 1, 2, 3, 4]

def absolute_distance(lhs, rhs):
    return abs(lhs - rhs)

knn_graph = exact_knn_graph(records, absolute_distance, k=1)
radius_graph = exact_radius_graph(records, absolute_distance, radius=1)

knn_edges = knn_graph.edges
radius_edges = radius_graph.edges
degree_info = graph_degree_diagnostics(knn_graph)
undirected = symmetrize_graph(knn_graph, policy="union", weighting="minimum_distance")
pruned = prune_graph_out_degree(exact_knn_graph(records, absolute_distance, k=2), max_out_degree=1)
```

`knn_graph.metadata` records strategy `exact_knn`, `record_count`, `edge_count`, `directed=True`, `self_loops=False`, `exact=True`, `k`, `edge_payload="metric_distance"`, `weighting="none"`, `sparsification="none"`, `symmetrization="none"`, `normalization="none"`, and the tie-breaking rule. `radius_graph.metadata` records the same policy fields with `radius` instead of `k`. `undirected.metadata` records `directed=False`, `symmetrization="union"`, and `weighting="minimum_distance"`. `pruned.metadata` records `sparsification="out_degree"` and `max_out_degree=1` while preserving the source graph construction strategy.

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

For `records = [0, 1, 2, 3, 4]`, symmetrizing the exact kNN graph with `k=1` and the `union` policy produces undirected edges:

```text
(0, 1, 1)
(1, 2, 1)
(2, 3, 1)
(3, 4, 1)
```

The directed `k=1` graph has out-degrees `(1, 1, 1, 1, 1)`, in-degrees `(1, 2, 1, 1, 0)`, endpoint degrees `(2, 3, 2, 2, 1)`, max degree `3`, average degree `2.0`, and degree policy `directed_in_out`.

For the same records, pruning the exact kNN graph from `k=2` to `max_out_degree=1` produces directed edges:

```text
(0, 1, 1)
(1, 0, 1)
(2, 1, 1)
(3, 2, 1)
(4, 3, 1)
```
