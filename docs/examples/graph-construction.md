# Exact Graph Edge Fixtures

Exact graph helpers emit directed edge lists over a finite metric space. They are small fixtures for validating graph construction semantics before higher-level graph result objects are promoted.

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

auto knn_edges = metric::operators::exact_knn_graph_edges(records, AbsoluteDistance{}, 2);
auto radius_edges = metric::operators::exact_radius_graph_edges(records, AbsoluteDistance{}, 1);
```

Python shape:

```python
from metric import exact_knn_graph_edges, exact_radius_graph_edges

records = [0, 1, 2, 3, 4]

def absolute_distance(lhs, rhs):
    return abs(lhs - rhs)

knn_edges = exact_knn_graph_edges(records, absolute_distance, k=2)
radius_edges = exact_radius_graph_edges(records, absolute_distance, radius=1)
```

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

These helpers are exact because they evaluate the pairwise distances needed for every source record. `metric::KNNGraph` remains an approximate compatibility representation and should not be used as evidence for exact graph construction unless its edge set is compared against dense pairwise distances for the fixture.
