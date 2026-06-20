# Python Intent Methods

The Python facade is organized around user intent. Start with records and a
metric, then ask what you want to know about the finite metric space.

## Neighbors

```python
from metric import Space
from metric.metrics import Edit

space = Space(["cat", "cot", "coat", "dog"], metric=Edit())
neighbors = space.neighbors("cut", count=2)

for item in neighbors.neighbors:
    print(item.record, item.distance)
```

Use `count` for nearest-neighbor queries and `radius` for range-neighbor
queries. Calling `space.neighbors(count=...)` without a query returns one
neighbor row per source record in `result.rows`.

## Groups

```python
from metric import Space
from metric.metrics import Edit

space = Space(["cat", "cot", "coat", "dog"], metric=Edit())
groups = space.groups(count=2)

print(groups.assignments)
print(groups.medoids)
```

`count` selects deterministic k-medoids grouping. `radius` selects a DBSCAN
style density grouping with explicit noise records.

## Embed

```python
from metric import Space
from metric.metrics import Edit

space = Space(["cat", "cot", "coat", "dog"], metric=Edit())
embedding = space.embed(dimensions=2)

print(embedding.coordinates)
print(embedding.diagnostics.to_dict())
```

Embedding creates a derived coordinate view for visualization and diagnostics.
It does not change the source records and does not make vectors the primary
model.

## Map

```python
from metric import Space

records = [
    {"id": "a", "temperature": 21.0, "pressure": 1.0},
    {"id": "b", "temperature": 22.5, "pressure": 1.1},
    {"id": "c", "temperature": 36.0, "pressure": 2.0},
]


def row_distance(lhs, rhs):
    return abs(lhs["temperature"] - rhs["temperature"]) + abs(
        lhs["pressure"] - rhs["pressure"]
    )


def temperature(record):
    return record["temperature"]


mapped = Space(records, metric=row_distance).map(
    transform=temperature,
    metric=lambda lhs, rhs: abs(lhs - rhs),
)
```

The promoted mapping path is deterministic `transform=...` plus a target metric.
No-argument mapping, learned `target=...` mapping, and strategy-driven mapping
raise explicit METRIC errors until those contracts are promoted.

## Outliers

```python
from metric import Space
from metric.metrics import Edit

space = Space(["cat", "cot", "coat", "doggggg"], metric=Edit())
outliers = space.outliers(count=1)

print(outliers.outliers[0].record_id)
print(outliers.outliers[0].score)
```

The strategy-free default ranks records by nearest-neighbor distance. Pass a
promoted DBSCAN strategy in expert code for density-noise behavior.

## Denoise

```python
from metric import Space
from metric.metrics import Edit

space = Space(["cat", "cot", "coat", "doggggg"], metric=Edit())
clean = space.denoise(count=1)

print(clean.source_record_ids)
print(len(clean.space))
```

Denoising returns a derived space and keeps source-record lineage. It does not
mutate the source space.

## Compare

```python
from metric import Space

records = ["a", "b", "c", "d"]
process = [0.0, 1.0, 2.0, 3.0]
quality = [0.0, 1.0, 4.0, 9.0]


def absolute_distance(lhs, rhs):
    return abs(lhs - rhs)


process_space = Space(process, metric=absolute_distance, ids=records)
quality_space = Space(quality, metric=absolute_distance, ids=records)

comparison = process_space.compare(quality_space, align="ids")
print(comparison.value)
```

Comparison uses distance-profile correlation for the promoted exact path.
Use `align="ids"` when both spaces share stable IDs. The compatibility default
is positional alignment.

## Representations And Runtime

```python
from metric import Space
from metric import RuntimePolicy
from metric.metrics import Edit

space = Space(["cat", "cot", "coat", "dog"], metric=Edit())
policy = RuntimePolicy(exact=True, cache="materialized")

neighbors = space.neighbors("cut", count=2, runtime=policy)
diagnostics = space.runtime_diagnostics(runtime=policy, intent="neighbors")
```

Current Python intent methods execute exact deterministic paths. Passing
`RuntimePolicy(exact=False)` raises `StrategyUnavailableError` instead of
silently returning approximate results.
