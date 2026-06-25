# Python Intent Methods

The Python facade is organized around user intent. Start with records and a
metric, then ask what you want to know about the finite metric space.

> **Availability.** The default core wheel runs construction, `distance`,
> `pairwise`, representation views, exact neighbors, groups, outliers,
> density filtering, representatives, reduce/compress, distribution-preserving
> thinning, uniform-density thinning, structure diagnostics, and aligned
> distance-profile `compare`/`correlate` (equal-length, `align="position"`)
> through native bindings. `embed` still raises `StrategyUnavailableError` until
> its binding is promoted, and non-aligned (`align="ids"`) comparison stays
> native-only.

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
style density grouping with explicit unassigned records.

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
object.

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
No-argument mapping, derived `target=...` mapping, and strategy-driven mapping
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
promoted DBSCAN strategy in expert code when density-unassigned records are the
desired singularity signal.

## Density Filter

```python
from metric import Space
from metric.metrics import Edit

space = Space(["cat", "cot", "coat", "doggggg"], metric=Edit())
clean = space.density_filter(count=1)

print(clean.source_record_ids)
print(len(clean.space))
```

Density filtering returns a derived space and keeps source-record lineage. It
does not mutate the source space and does not apply inverse metric dynamics.

## Compress, Thin, And Equalize

```python
from metric import Space
from metric.metrics import Edit
from metric.strategies import PreserveDistribution, UniformDensity

space = Space(["cat", "cot", "coat", "dog", "dogs"], metric=Edit())

summary = space.compress(count=2)
panel = space.thin(3, strategy=PreserveDistribution())
net = space.thin(strategy=UniformDensity(radius=1))
equalized = space.equalize(radius=1)
```

Compression selects source representatives, assigns every source record to a
representative, and reports representative multiplicities and normalized
weights. Thinning keeps a subset without pretending to reconstruct removed
records. `PreserveDistribution` keeps empirical density representative by
regular source-order sampling, while `UniformDensity` builds a maximal metric
radius net and intentionally flattens sampling density. `equalize(...)` uses the
same metric-only radius-net construction but names the density-normalization
intent explicitly.

Choose the modification route by the finite-space objective:

| Objective | Use | Result contract |
|---|---|---|
| Preserve source density | `space.thin(count, strategy=PreserveDistribution(...))` | Retained source records are a regular empirical sample; removed records are not collapsed. |
| Flatten source density | `space.thin(strategy=UniformDensity(radius=...))` or `space.equalize(radius=...)` | Retained records form a metric radius net; diagnostics report coverage, nearest-neighbor drift, and local-volume drift. |
| Minimize maximum assignment error | `space.compress(count, strategy=Coverage(...))`, `KCenter(...)`, or `RadiusCoverage(...)` | Every source record is assigned to a representative; multiplicities and weights make the result a metric-measure summary. |
| Minimize average assignment error | `space.compress(strategy=KMedoids(groups=...))` | Representatives are source medoids, so arbitrary record types stay valid. |
| Preserve neighbor behavior | Use promoted neighbor diagnostics, then research-only `metric.experimental.knn_recall_sketch(...)` for candidate sketches. | This is a diagnostic objective until promoted; it compares source kNN behavior with a subset or graph. |
| Preserve distance distribution or graph structure | Use research-only `distance_distribution_sketch(...)`, `distance_distribution_drift(...)`, or `metric_graph_spanner(...)`. | These are experimental finite-metric observables and are intentionally not re-exported from `metric.__all__`. |

See
[`python/examples/engine/metric_space_modification_objectives.py`](../../python/examples/engine/metric_space_modification_objectives.py)
for a runnable objective-by-objective example, and
[Customer Focus Groups As A Finite Metric Space](../examples/customer-focus-groups-metric-space.md)
for the mixed customer-record/persona workflow.

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

comparison = process_space.compare(quality_space)
print(comparison.value)
```

Comparison uses distance-profile correlation (Pearson) for the promoted exact
path: two equal-length spaces compared by position (`align="position"`). The
`CorrelationResult` carries `value`, `pair_count`, `matched_ids`, `align`, and a
`diagnostics` dict. Mismatched record counts raise `IncompatibleSpaceError`,
naming both counts; a degenerate (zero-variance or empty) profile returns
`value=0.0` with `diagnostics["defined"] is False`. `correlate(...)` is an alias
of `compare(...)` for this path. ID-based alignment (`align="ids"`) stays
native-only and raises `StrategyUnavailableError`.

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

When an intent method is promoted it executes an exact deterministic path; the
methods not yet promoted raise `StrategyUnavailableError` regardless of policy
(see the availability note at the top). Passing `RuntimePolicy(exact=False)`
likewise raises `StrategyUnavailableError` instead of silently returning
approximate results.
