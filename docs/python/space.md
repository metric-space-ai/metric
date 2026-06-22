# Python Space

`Space` is the Python entry point for finite metric-space work. It stores a
finite set of records, an explicit metric callable, stable record IDs, and
small runtime preferences for validation and distance caching.

> **Availability.** Construction, `distance`, `pairwise`, representation views,
> exact neighbors, groups, outliers, denoise, representatives, reduce/compress,
> structure diagnostics, and aligned distance-profile `compare`/`correlate`
> (equal-length, `align="position"`) run in the default core wheel. `embed`
> still raises `StrategyUnavailableError` until its binding is promoted, and
> non-aligned (`align="ids"`) comparison stays native-only.

## Constructor

```python
from metric import Space
from metric.metrics import Edit

records = ["cat", "cot", "coat", "dog"]
space = Space(
    records,
    metric=Edit(),
    ids=None,
    name="words",
    metadata={"source": "demo"},
    validate="sample",
    copy=False,
    cache="auto",
)
```

`records` may be strings, dictionaries, NumPy rows, time-series records, or
domain objects. `metric` is required for arbitrary records and must be callable
as `metric(lhs, rhs)`. `ids` defaults to generated `RecordId` values in input
order. IDs are stable user-facing identifiers, not physical row positions.

`validate` accepts:

| Value | Behavior |
|---|---|
| `"none"` | Skip constructor distance checks. |
| `"sample"` | Check a small prefix for finite, real, non-negative distances. |
| `"strict"` | Check every pair during construction. |

`cache` accepts `"auto"`, `"none"`, `"lazy"`, `"materialized"`, or a
`metric.runtime.CachePolicy`. The default `"auto"` materializes the exact
pairwise matrix for the current Python finite-space representation.

## Records And IDs

```python
from metric import Space
from metric.metrics import Edit

space = Space(["alpha", "alfa", "omega"], metric=Edit(), ids=["a", "b", "c"])

assert space.record("a") == "alpha"
assert space.ids == ["a", "b", "c"]
assert space.distance(0, 1) == 2
```

`space.record(record_id)` resolves through stable IDs. Distance and pairwise
operations use physical positions today, while result objects report stable
source IDs where the promoted result contract exposes them.

## NumPy Distance Matrix

```python
from metric import Space
from metric.metrics import Edit

space = Space(["cat", "cot", "coat", "dog"], metric=Edit())
matrix = space.distance_matrix_numpy()

assert matrix.shape == (len(space), len(space))
assert matrix[0][1] == space.distance(0, 1)
```

`Space.distance_matrix_numpy()` returns the pairwise distance matrix as a square
`numpy.ndarray` of shape `(n, n)` in record order, built from
`pairwise_distances()`. For a symmetric metric the result is symmetric, and for
an identity-respecting metric the diagonal is zero -- both follow from the
underlying pairwise values. The method imports numpy lazily and raises
`OptionalDependencyError` when numpy is not installed.

## DataFrame Rows

```python
from metric import Space
import pandas as pd

df = pd.DataFrame(
    [
        {"sample_id": "a", "temperature": 21.0, "pressure": 1.0},
        {"sample_id": "b", "temperature": 22.5, "pressure": 1.1},
        {"sample_id": "c", "temperature": 36.0, "pressure": 2.0},
    ]
)


def row_distance(lhs, rhs):
    return abs(lhs["temperature"] - rhs["temperature"]) + 8.0 * abs(
        lhs["pressure"] - rhs["pressure"]
    )


space = Space.from_dataframe(df, metric=row_distance, id_column="sample_id")
outliers = space.outliers(count=1)
```

`Space.from_dataframe(...)` reads rows with `to_dict("records")`. When
`id_column` is provided, that column becomes the stable ID set and is removed
from the row dictionaries passed to the metric.

## Vector Convenience

```python
from metric import Space
import numpy as np

vectors = np.array([[0.0, 0.0], [3.0, 4.0], [6.0, 8.0]])
space = Space.vectors(vectors)

assert space.distance(0, 1) == 5.0
```

`Space.vectors(...)` is a vector-specific convenience with a pure Python
Euclidean default. Use `Space(records, metric=...)` for arbitrary records and
for vector records that need a domain-specific metric.

## Representations

```python
from metric import Space
from metric.metrics import Edit

space = Space(["cat", "cot", "coat", "dog"], metric=Edit())
matrix = space.to_matrix()
tree = space.to_tree()
graph = space.to_graph(count=2)

neighbors = space.neighbors("cut", count=2, representation=tree)
```

`to_matrix()` returns an explicit finite matrix-space view. `to_tree()` and
`to_graph(count=...)` expose exact deterministic representation vocabulary for
neighbor and local-structure workflows. Representations capture the source
space version. After `space.touch()`, old representations fail with
`StaleRepresentationError` until rebuilt.

## Compare Aligned Spaces

```python
from metric import Space

absolute = lambda lhs, rhs: abs(lhs - rhs)
process = Space([0.0, 1.0, 2.0, 3.0], metric=absolute, ids=["a", "b", "c", "d"])
quality = Space([0.0, 2.0, 4.0, 6.0], metric=absolute, ids=["a", "b", "c", "d"])

comparison = process.compare(quality)
assert comparison.align == "position"
print(comparison.value)  # Pearson correlation of the two distance profiles
```

`space.compare(other)` and `space.correlate(other)` are promoted for the
equal-length, `align="position"` path. They compute the Pearson correlation of
the two spaces' pairwise distance profiles in native C++ and return a
`CorrelationResult` (`value`, `pair_count`, `align`, `matched_ids`,
`statistic_name`, `diagnostics`). Mismatched record counts raise
`IncompatibleSpaceError`, naming both counts. A degenerate profile (fewer than
two records, or all pairwise distances equal) is undefined and returns
`value=0.0` with `diagnostics["defined"] is False`. `align="ids"` comparison
stays native-only and raises `StrategyUnavailableError`.
