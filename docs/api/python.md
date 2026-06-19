# Python API

The current Python core API exposes metric constructors, a minimal `Space` facade, finite-space helpers, and small operator helpers. It is intentionally small while the broader engine facade is restored.

The stable entry point is `Space`: a finite record set plus a metric with cached pairwise distances and intent-named neighbor helpers. `FiniteMetricSpace` and `MatrixSpace` remain available for explicit representation vocabulary.

## Basic Use

```python
from metric import Edit, Space

records = ["cat", "cot", "coat", "dog"]
space = Space(records, Edit())

print(space.distance(0, 1))
print(space.neighbors("cut", k=2))
```

The records are strings. Edit distance defines the geometry without an embedding step.

## Core Objects

- `metrics`: standard metric constructors
- `Space`: minimal intent-first finite metric space facade
- `FiniteMetricSpace`: finite metric space over records
- `MatrixSpace`: compatibility alias for `FiniteMetricSpace`
- `operators`: small helpers for pairwise distances, nearest neighbors, and range neighbors
- `mappings`: beta compatibility bridge for installed mapping bindings
- `transforms`: beta compatibility bridge for installed transform bindings

## Core Methods

```python
len(space)
space[index]
space.distance(lhs_index, rhs_index)
space.pairwise_distances()
space.neighbors(query, k=10)
space.nearest(query)
space.within_radius(query, radius=1)
space.knn(query, k=10)
space.nn(query)
space.rnn(query, radius=1)
```

## Operators

```python
from metric.operators import nearest_neighbors, pairwise_distance_matrix, range_neighbors

distances = pairwise_distance_matrix(records, Edit())
neighbors = nearest_neighbors(records, Edit(), "cut", k=2)
close = range_neighbors(records, Edit(), "cut", radius=1)
```

## Custom Metrics

```python
def padded_hamming(lhs: str, rhs: str) -> int:
    width = max(len(lhs), len(rhs))
    return sum(
        (lhs[i] if i < len(lhs) else "_") != (rhs[i] if i < len(rhs) else "_")
        for i in range(width)
    )

space = Space(["red", "reed", "road", "blue"], padded_hamming)
print(space.neighbors("read", k=2))
```

## NumPy Records

NumPy arrays are accepted as records when the supplied metric callable accepts the array objects it receives. The revived Python facade does not force records into vectors or apply an implicit embedding step:

```python
import numpy as np
from metric import Space

records = np.array([[0.0, 0.0], [3.0, 4.0], [6.0, 8.0]])

def euclidean(lhs, rhs) -> float:
    return float(np.linalg.norm(lhs - rhs))

space = Space(records, euclidean)
print(space.distance(0, 1))
```

`pairwise_distances()` and `metric.operators.pairwise_distance_matrix()` currently return Python lists of lists. Use `np.asarray(...)` in user code when an ndarray result is needed.

## Engine Roadmap

The implemented facade currently covers neighbor access. Additional intent names such as `groups`, `embed`, `map`, `reduce`, `denoise`, `outliers`, and `compare` describe the public direction and should be promoted only when they are backed by stable strategies, result objects, examples, and CI.

## Compatibility

Compatibility modules remain importable for existing code. New Python examples use `metric.metrics`, `metric.spaces`, and `metric.operators`.

`metric.mappings` and `metric.transforms` are stable import locations for beta compatibility surfaces:

```python
import metric

print(metric.mappings.STABILITY)
print(metric.mappings.available())
print(metric.transforms.available())
```

They intentionally do not promote mapping or transform algorithms into the core wheel contract. They report the public names that the installed wheel exposes and raise `ImportError` from `legacy_module()` when a legacy binding is not present.
