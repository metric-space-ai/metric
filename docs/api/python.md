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
- `operators`: small helpers for pairwise distances, nearest neighbors, range neighbors, representative selection, medoids, and intrinsic-dimension diagnostics
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
from metric.operators import (
    coverage_representative_indices,
    coverage_representatives,
    intrinsic_dimension,
    medoid,
    medoid_index,
    nearest_neighbors,
    pairwise_distance_matrix,
    range_neighbors,
    representative_indices,
    representatives,
)

distances = pairwise_distance_matrix(records, Edit())
neighbors = nearest_neighbors(records, Edit(), "cut", k=2)
close = range_neighbors(records, Edit(), "cut", radius=1)
selected_ids = representative_indices(records, Edit(), k=2)
selected_records = representatives(records, Edit(), k=2)
center_id = medoid_index(records, Edit())
center_record = medoid(records, Edit())
covered_ids = coverage_representative_indices(records, Edit(), radius=1)
covered_records = coverage_representatives(records, Edit(), radius=1)
dimension = intrinsic_dimension(records, Edit())
```

`representative_indices` and `representatives` use deterministic farthest-first traversal over the finite metric space. They select existing records rather than vector centroids, start from `seed_index=0` by default, and resolve equal-distance ties by record order.

`medoid_index` and `medoid` select the existing record with the smallest total distance to all records. Equal total-distance ties are resolved by record order.

`coverage_representative_indices` and `coverage_representatives` use deterministic greedy radius coverage. They scan records in order, choose the first uncovered record as a representative, and mark every record within `radius` as covered.

`intrinsic_dimension` returns an expansion-dimension estimate based on finite-space neighborhood growth. Treat it as a diagnostic that depends on the metric, sample density, duplicates, and available radii.

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
