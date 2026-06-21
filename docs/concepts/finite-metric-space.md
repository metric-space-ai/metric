# Finite Metric Spaces

A finite metric space is a finite set of records together with a metric that assigns a distance to every pair of records.

In METRIC terms:

- a **record** is one item in the dataset
- a **metric** is the domain-specific distance function for records of that type
- a **space** is the record set plus that metric
- an **operator** computes on the space
- a **strategy** selects a concrete algorithm
- a **representation** stores or indexes derived structure

The record type does not have to be a vector. It may be a string, a time series, a histogram, an image descriptor, a graph, or a structured industrial record. The metric is the part that makes comparisons meaningful.

## Definition

Given records `X = {x0, x1, ..., xn}` and a metric `d`, the pair `(X, d)` is a finite metric space when `d` satisfies the requirements needed by the operator:

- non-negativity: `d(x, y) >= 0`
- identity: `d(x, x) = 0`
- symmetry: `d(x, y) = d(y, x)`
- triangle inequality: `d(x, z) <= d(x, y) + d(y, z)`

Operators that support pseudo-metrics or weaker dissimilarities document the interpretation of their results.

## Implicit and Explicit Spaces

The simplest representation is implicit: keep the records and compute `d(x, y)` on demand. This is flexible and useful when the metric is cheap or the workflow is exploratory.

METRIC also provides explicit representations:

- `metric::MatrixSpace`: full pairwise distance matrix
- `metric::GraphSpace`: sparse nearest-neighbor graph
- `metric::TreeSpace`: search/index representation for neighbor access

These representations trade memory, update cost, and query speed differently. A matrix gives direct pairwise distance lookup. A graph gives sparse local structure. A tree supports nearest-neighbor access without storing every pairwise distance.

## C++ Example

```cpp
#include <metric/distance.hpp>
#include <metric/space.hpp>

#include <string>
#include <vector>

int main()
{
    std::vector<std::string> records = {"cat", "cot", "coat", "dog"};

    auto space = metric::Space::from_records(records, metric::Edit<std::string>{});

    auto nearest = space.neighbors(std::string("cut"), 2);
    return nearest.empty();
}
```

This is a finite metric space over strings. The strings are not embedded into vectors first; edit distance defines the geometry.

## Python Binding Example

```python
from metric.metrics import Edit
from metric.spaces import FiniteMetricSpace

space = FiniteMetricSpace(["cat", "cot", "coat", "dog"], Edit())

print(space.distance(0, 1))
print(space.knn("cut", k=2))
```

The Python object adapts user data to the native engine surface. The metric-space model and promoted algorithms belong to the C++ implementation.

## Why This Is Broader Than Vector Search

Vector-search libraries assume the record is already a vector, or that a separate model has embedded it into one. METRIC treats vectors as one valid record type among many. When the domain has a better native metric than an embedding distance, METRIC computes directly on that metric space.

Related concepts:

- [Metric Spaces](metric-space.md)
- [Metrics as Recoding Costs](metrics-as-recoding-costs.md)
- [Vector Space as a Special Case](vector-space-as-special-case.md)
- [Explicit Representations](explicit-representations.md)
