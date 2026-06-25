# Intrinsic Dimension Diagnostic

Intrinsic-dimension diagnostics estimate how quickly neighborhoods grow inside a finite metric space. They are useful when deciding whether a tree, graph, or dense matrix representation is likely to match the geometry.

The revived core exposes a small expansion-dimension estimate:

```text
max_x,r log2(|B(x, 2r)| / |B(x, r)|)
```

This is a finite-space diagnostic, not an exact manifold dimension. It depends on the metric, sample density, duplicate records, and the radii available in the dataset.

The promoted C++ example [metric_space_intrinsic_dimension.cpp](../../examples/core/metric_space_intrinsic_dimension.cpp) runs in the core CI path and asserts a deterministic regression value for a small line metric.

## Current C++ Shape

```cpp
#include <metric/space/index/operators.hpp>

#include <vector>

struct AbsoluteDistance {
    auto operator()(int lhs, int rhs) const -> int
    {
        return lhs > rhs ? lhs - rhs : rhs - lhs;
    }
};

std::vector<int> records = {0, 1, 2, 3, 4};
double estimate = mtrc::space::index::intrinsic_dimension(records, AbsoluteDistance{});
```

## Current Python Shape

```python
from metric import intrinsic_dimension

records = [0, 1, 2, 3, 4]

def absolute_distance(lhs, rhs):
    return abs(lhs - rhs)

estimate = intrinsic_dimension(records, absolute_distance)
```
