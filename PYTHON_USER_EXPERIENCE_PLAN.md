# Python User Experience Plan

## Purpose

Python should be the easiest way to understand and use METRIC on real data.

The public Python API must present METRIC as an engine for finite metric spaces,
not as a catalog of algorithm wrappers. A user should start with records and a
metric, then ask for an intent:

```python
from metric import Space
from metric.metrics import EditDistance

records = ["metric", "metrics", "matrix", "tree"]
space = Space(records, metric=EditDistance())

neighbors = space.neighbors("metricks", count=2)
groups = space.groups(count=2)
embedding = space.embed(dimensions=2)
outliers = space.outliers(count=2)
```

Algorithm names belong under `metric.strategies`. They may be used by experts to
override execution, but they must not be the first API normal users learn.

## Design Principles

### 1. Records First

Users bring records. A record may be a string, row, vector, histogram, time
series, image descriptor, graph, or domain object. The metric defines the
geometry. Python examples must start from records, not from precomputed vectors.

### 2. Intent Names First

The user-facing methods describe what the user wants:

- `space.neighbors`
- `space.groups`
- `space.embed`
- `space.map`
- `space.outliers`
- `space.denoise`
- `space.compare`

The public teaching path must not start with names such as DBSCAN, k-medoids,
MGC, PCFA, SOM, KOC, or DSPCC. Those names are strategy objects under
`metric.strategies`.

### 3. Named Results, Not Tuples

Every user-facing method returns a named result object with stable fields,
diagnostics, and conversion helpers. Tuple returns remain compatibility API only.

### 4. Exact By Default

Default behavior should be correct, deterministic, and explainable. Approximate
methods, aggressive caching, and algorithm-specific tuning are opt-in through
strategy, representation, or runtime overrides.

### 5. Python Is A Facade, Not A Template Mirror

Python exposes the engine vocabulary:

- `Space`
- `Metric`
- `RecordId`
- result objects
- metrics
- representations
- runtime policies
- strategies

Python docs should not expose `_impl`, generated template names, C++ container
types, or Blaze matrix objects unless an expert explicitly asks for low-level
interop.

## Public Package Shape

```text
metric/
  __init__.py          # Space, Metric, RecordId, result and error base classes
  metrics/             # user-visible metric callables
  strategies/          # algorithm and execution strategy objects
  representations/     # matrix, tree, graph, and lazy providers
  runtime/             # cache, parallelism, validation, and planning policies
  results/             # typed result objects
  errors.py            # public exception hierarchy
  compat/              # old import paths and tuple-return wrappers
```

Top-level `metric` should export only engine-level names:

```python
from metric import Space, Metric, RecordId
```

Top-level `metric` must not export algorithm strategy names. This should fail:

```python
from metric import KMedoids
```

This is the intended expert import:

```python
from metric.strategies import KMedoids
```

## Space Constructor

Primary constructor:

```python
space = Space(
    records,
    metric=metric,
    ids=None,
    name=None,
    metadata=None,
    validate="sample",
    copy=False,
    cache="auto",
)
```

Parameters:

- `records`: finite record collection. Accept Python sequences, NumPy arrays,
  pandas DataFrames, and objects implementing the sequence protocol.
- `metric`: required callable for v1. It returns a non-negative numeric distance
  for two records. Examples should always pass this explicitly.
- `ids`: optional stable record IDs. Defaults to generated IDs preserving input
  order. IDs are not physical positions.
- `name`: optional human-readable space name used in reprs and diagnostics.
- `metadata`: optional user metadata, preserved in result objects.
- `validate`: one of `"none"`, `"sample"`, or `"strict"`.
- `copy`: whether to copy records on construction. Defaults to `False`.
- `cache`: one of `"auto"`, `"none"`, or a `metric.runtime.CachePolicy`.

Convenience constructors may exist, but they must not replace the main mental
model:

```python
space = Space.from_dataframe(df, metric=metric, id_column="sample_id")
space = Space.vectors(array, metric=Euclidean())
```

`Space.vectors(array)` may default to Euclidean only as a vector-specific
convenience. `Space(records)` without a metric should raise `MissingMetricError`.

## Metric Protocol

Any metric accepted by `Space` must satisfy this Python protocol:

```python
class Metric(Protocol):
    def __call__(self, lhs: object, rhs: object) -> float:
        ...
```

Metric objects may expose optional metadata:

```python
metric.name
metric.traits.law              # "metric", "pseudometric", "semimetric", etc.
metric.traits.symmetric
metric.traits.thread_safe
metric.traits.requires_equal_shape
metric.traits.record_kind
```

Plain callables remain valid:

```python
def padded_hamming(lhs, rhs):
    size = max(len(lhs), len(rhs))
    return sum(
        (lhs[i] if i < len(lhs) else None) != (rhs[i] if i < len(rhs) else None)
        for i in range(size)
    )

space = Space(["red", "reed", "road", "blue"], metric=padded_hamming)
```

## Space Surface

Minimum public attributes and methods:

```python
len(space)
space.ids
space.records
space.metric
space.name
space.metadata
space.version

space.record(record_id)
space.distance(lhs, rhs)
space.pairwise(ids=None)

space.to_matrix()
space.to_tree()
space.to_graph(count=8)

space.neighbors(query=None, count=5, radius=None, **options)
space.groups(count="auto", radius=None, **options)
space.embed(dimensions=2, **options)
space.map(target=None, transform=None, metric=None, **options)
space.outliers(count=None, fraction=0.05, **options)
space.denoise(strength="auto", **options)
space.compare(other, **options)
space.describe(**options)
```

The repr should show the engine model:

```text
Space(size=4, metric=EditDistance(), records=str, name=None)
```

It should not show generated pybind class names.

## Intent Methods

### `space.neighbors`

Purpose: find similar records.

```python
result = space.neighbors("metricks", count=3)
```

Signature:

```python
space.neighbors(
    query=None,
    *,
    count=5,
    radius=None,
    include_self=False,
    strategy=None,
    representation=None,
    runtime=None,
) -> NeighborResult
```

Behavior:

- If `query` is provided, return neighbors for that query record.
- If `query` is omitted, return nearest-neighbor rows for every record in the
  space.
- `count` and `radius` are mutually compatible: if both are supplied, return
  records satisfying the radius, capped by count.
- Default is exact search.
- If a matrix or tree representation already exists and is fresh, it may be
  reused. The result must report which representation was used.

Expert override:

```python
from metric.strategies import CoverTree

neighbors = space.neighbors(
    "metricks",
    count=10,
    strategy=CoverTree(exact=True),
)
```

### `space.groups`

Purpose: find groups, clusters, and representative records.

```python
groups = space.groups(count=3)
```

Signature:

```python
space.groups(
    *,
    count="auto",
    radius=None,
    min_size=1,
    strategy=None,
    representation=None,
    runtime=None,
) -> GroupResult
```

Behavior:

- `count` means requested number of groups.
- `radius` means group by neighborhood scale.
- If both are omitted, `count="auto"` uses a conservative default strategy and
  records the chosen assumptions in `result.diagnostics`.
- Noise and unassigned records are represented explicitly, not hidden.

Expert override:

```python
from metric.strategies import KMedoids

groups = space.groups(strategy=KMedoids(groups=3, max_iterations=100))
```

### `space.embed`

Purpose: create coordinates for visualization, diagnostics, or downstream vector
tools while preserving links to the original metric space.

```python
embedding = space.embed(dimensions=2)
```

Signature:

```python
space.embed(
    *,
    dimensions=2,
    metric=None,
    strategy=None,
    representation=None,
    runtime=None,
) -> EmbeddingResult
```

Behavior:

- Default output metric is Euclidean over the generated coordinates.
- Result coordinates are aligned with `space.ids`.
- The original records remain available through `result.source_space`.
- The method does not imply that METRIC is vector-first. It is a derived view of
  a metric space.

Expert override:

```python
from metric.strategies import PCFA

embedding = space.embed(strategy=PCFA(dimensions=3))
```

### `space.map`

Purpose: transform one metric space into another record space, or learn a mapping
between aligned spaces.

```python
mapped = source_space.map(target=target_space)
```

Signature:

```python
space.map(
    *,
    target=None,
    transform=None,
    metric=None,
    strategy=None,
    runtime=None,
) -> MappingResult
```

Behavior:

- `target` learns or fits a mapping between aligned source and target spaces.
- `transform` applies a deterministic Python callable to each source record.
- `metric` is required when `transform` creates records whose metric cannot be
  inferred.
- Calling `space.map()` with neither `target` nor `transform` is allowed only
  when `strategy` declares that it can derive its own target space from the
  source geometry, for example a structure-preserving or PHATE-AE-style mapping.
  Otherwise it raises `AmbiguousIntentError`. Users who only want coordinates
  should call `space.embed`.
- Mapping never mutates the source space.

Examples:

```python
feature_space = images.map(
    transform=extract_histogram,
    metric=EarthMoverDistance(),
)

relation = sensor_space.map(target=quality_space)
```

Expert override:

```python
from metric.strategies import DSPCC

mapping = sensor_space.map(target=quality_space, strategy=DSPCC(features=8))
```

Self-derived mapping:

```python
from metric.strategies import PhateAutoencoder

model = space.map(strategy=PhateAutoencoder(dimensions=2))
```

### `space.outliers`

Purpose: score or select unusual records.

```python
outliers = space.outliers(count=10)
```

Signature:

```python
space.outliers(
    *,
    count=None,
    fraction=0.05,
    threshold=None,
    strategy=None,
    representation=None,
    runtime=None,
) -> OutlierResult
```

Behavior:

- Default returns the top 5 percent by outlier score.
- `count` overrides `fraction`.
- `threshold` returns all records with score above threshold.
- Scores are always returned for all records when possible.
- Result explains whether scores are density-based, graph-based, manifold-based,
  or strategy-specific.

Expert override:

```python
from metric.strategies import LocalDensityOutliers

outliers = space.outliers(strategy=LocalDensityOutliers(k=12))
```

### `space.denoise`

Purpose: produce a cleaned derived space or cleaned records without mutating the
original records.

```python
clean = space.denoise()
```

Signature:

```python
space.denoise(
    *,
    strength="auto",
    preserve_ids=True,
    strategy=None,
    representation=None,
    runtime=None,
) -> DenoiseResult
```

Behavior:

- Returns a `DenoiseResult` containing the source space, derived space, residual
  scores, and diagnostics.
- If the selected method cannot reconstruct records for the record type, raise
  `UnsupportedOperationError` with a suggested strategy or `space.embed` path.
- Denoising must never silently change IDs.

Expert override:

```python
from metric.strategies import GraphLaplacianDenoise

clean = space.denoise(strategy=GraphLaplacianDenoise(k=12, strength=0.2))
```

### `space.compare`

Purpose: compare two metric spaces or measure dependency between aligned spaces.

```python
comparison = image_space.compare(label_space)
```

Signature:

```python
space.compare(
    other,
    *,
    align="ids",
    permutations=0,
    strategy=None,
    runtime=None,
) -> CompareResult
```

Behavior:

- `other` must be a `Space` or convertible to one with an explicit metric.
- `align="ids"` matches shared IDs.
- `align="position"` matches input order and requires equal lengths.
- Default returns a deterministic statistic only.
- P-values are computed only when `permutations > 0`.
- The result records matched IDs, dropped IDs, and alignment decisions.

Expert override:

```python
from metric.strategies import MGC

comparison = image_space.compare(
    label_space,
    strategy=MGC(),
    permutations=999,
)
```

## Result Objects

All result objects should be lightweight dataclasses or dataclass-like immutable
objects. They should have stable attributes, readable reprs, and conversion
helpers.

Common fields:

- `source_space`
- `ids`
- `strategy`
- `representation`
- `runtime`
- `exact`
- `diagnostics`
- `warnings`

Common methods:

- `to_dict()`
- `to_pandas()`
- `to_numpy()` where numeric data exists
- `plot()` only when optional plotting dependencies are installed

### `Neighbor`

```python
Neighbor(
    id=RecordId(...),
    record=...,
    distance=0.0,
    rank=0,
)
```

### `NeighborResult`

Fields:

- `query`
- `query_id`
- `neighbors`
- `rows`
- `distances`
- `exact`
- `strategy`
- `representation`
- `diagnostics`

`neighbors` is used for a single query. `rows` is used for all-record neighbor
queries and maps each ID to a list of `Neighbor` objects.

### `GroupResult`

Fields:

- `labels`
- `groups`
- `representatives`
- `counts`
- `noise_ids`
- `scores`
- `strategy`
- `diagnostics`

Labels are keyed by `RecordId`, not by physical row position.

### `EmbeddingResult`

Fields:

- `coordinates`
- `embedded_space`
- `source_space`
- `model`
- `dimensions`
- `stress`
- `trustworthiness`
- `strategy`
- `diagnostics`

`coordinates` should be a NumPy array when NumPy is installed. `embedded_space`
is a `Space` over coordinate records with an explicit metric.

### `MappingResult`

Fields:

- `model`
- `source_space`
- `target_space`
- `output_space`
- `matched_ids`
- `diagnostics`

The model must expose:

```python
model.transform(space_or_records)
model.inverse_transform(space_or_records)  # only when supported
```

If inverse transform is unsupported, raise `UnsupportedOperationError`.

### `OutlierResult`

Fields:

- `scores`
- `ranked_ids`
- `selected_ids`
- `threshold`
- `labels`
- `evidence`
- `strategy`
- `diagnostics`

`scores` is keyed by `RecordId`.

### `DenoiseResult`

Fields:

- `source_space`
- `denoised_space`
- `residuals`
- `changed_ids`
- `model`
- `strategy`
- `diagnostics`

`residuals` is keyed by `RecordId`.

### `CompareResult`

Fields:

- `value`
- `p_value`
- `statistic_name`
- `matched_ids`
- `dropped_left_ids`
- `dropped_right_ids`
- `local_scores`
- `strategy`
- `diagnostics`

`p_value` is `None` unless requested.

## Defaults

Defaults should favor clarity over cleverness.

| API | Default | Notes |
|---|---|---|
| `Space(records, metric=...)` | explicit metric required | no implicit metric for arbitrary records |
| `validate` | `"sample"` | catches obvious metric failures without quadratic cost |
| `cache` | `"auto"` | may reuse fresh representations but should not hide semantics |
| `space.neighbors` | exact, `count=5` | uses direct metric calls unless a fresh exact representation exists |
| `space.groups` | `count="auto"` | documents chosen assumptions in diagnostics |
| `space.embed` | `dimensions=2` | for visualization and simple demos |
| `space.map` | no no-arg default | requires `target` or `transform` |
| `space.outliers` | `fraction=0.05` | `count` or `threshold` can override |
| `space.denoise` | `strength="auto"` | refuses unsupported reconstruction clearly |
| `space.compare` | `align="ids"`, `permutations=0` | no expensive p-value unless requested |

Approximate results must always set `result.exact = False`.

## Expert Overrides

Expert controls should be explicit keyword-only arguments:

```python
space.neighbors(query, strategy=..., representation=..., runtime=...)
space.groups(strategy=..., representation=..., runtime=...)
space.compare(other, strategy=..., runtime=...)
```

Strategies:

```python
from metric.strategies import (
    BruteForce,
    MatrixCache,
    CoverTree,
    KNNGraph,
    KMedoids,
    DBSCAN,
    AffinityPropagation,
    MGC,
    PCFA,
    SOM,
    KOC,
    DSPCC,
)
```

Representations:

```python
from metric.representations import Matrix, Tree, Graph

matrix = space.to_matrix()
neighbors = space.neighbors("query", representation=matrix)
```

Runtime policies:

```python
from metric.runtime import RuntimePolicy

policy = RuntimePolicy(exact=True, parallel=True, cache="materialized")
result = space.neighbors("query", count=20, runtime=policy)
```

Rules:

- Algorithm names appear only in `metric.strategies`.
- Representation names appear only in `metric.representations` or `space.to_*`.
- Runtime policy names appear only in `metric.runtime`.
- Normal examples use no explicit strategy until the expert section.

## Error Model

Public methods must raise METRIC exceptions with actionable messages. They should
not leak raw pybind11 overload errors, C++ type names, or template instantiation
details.

Exception hierarchy:

```text
MetricError
  MissingMetricError
  MetricInputError
  MetricContractError
  MetricComputationError
  AmbiguousIntentError
  IncompatibleSpaceError
  StrategyError
    StrategyUnavailableError
    StrategyParameterError
  RepresentationError
    StaleRepresentationError
  OptionalDependencyError
  NotFittedError
  UnsupportedOperationError
```

Required error behavior:

- Missing metric: tell the user to pass `metric=...`.
- Bad metric return type: show the offending pair IDs and returned type.
- Negative distance during validation: raise `MetricContractError`.
- Unequal shapes: include the metric name and record IDs.
- Stale representation: include source space version and representation version.
- Missing compiled extension: raise lazily with install/build guidance.
- Missing optional dependency: name the extra, for example `metric[plot]`.
- Incompatible spaces in `compare`: explain whether ID or length alignment failed.
- Unsupported denoise reconstruction: suggest `space.embed` or a compatible
  strategy.

Error messages should include short examples when possible:

```text
MissingMetricError: Space requires an explicit metric for arbitrary records.
Use Space(records, metric=EditDistance()) or Space.vectors(array, metric=Euclidean()).
```

## API Parity With C++

Python and C++ should use the same concepts and result semantics. Python uses
methods for readability; C++ uses free functions and templates for generic code.

| Python | C++ | Notes |
|---|---|---|
| `metric.Space` | `metric::MetricSpace` | Python shortens the public name |
| `Space(records, metric=m)` | `metric::make_space(records, m)` | same finite-space model |
| `space.distance(a, b)` | `space.distance(id_a, id_b)` | Python also accepts records where unambiguous |
| `space.to_matrix()` | `metric::representations::matrix(space)` | representation view/cache |
| `space.to_tree()` | `metric::representations::cover_tree(space)` | representation view/index |
| `space.to_graph(count=k)` | `metric::representations::knn_graph(space, metric::k{k})` | sparse local structure |
| `space.neighbors(...)` | `metric::find_neighbors(space, ...)` | returns named neighbor result |
| `space.groups(...)` | `metric::find_groups(space, ...)` | returns named group result |
| `space.embed(...)` | `metric::embed(space, ...)` | C++ should expose this intent name |
| `space.map(...)` | `metric::map(space, ...)` or `metric::mappings::*` | returns mapping model/result |
| `space.outliers(...)` | `metric::find_outliers(space, ...)` | returns named score result |
| `space.denoise(...)` | `metric::denoise(space, ...)` | returns derived space/result |
| `space.compare(other)` | `metric::compare(space_a, space_b)` | returns named compare result |
| `metric.strategies.KMedoids` | `metric::strategies::k_medoids` | algorithm name isolated as strategy |

If C++ keeps lower-level names such as `reduce`, Python may expose them only as
expert aliases. The first public Python concept should be `embed` because that is
the user intent for creating coordinates.

## Docs And Examples

Documentation should be reorganized around the user path:

1. Build a space from records and a metric.
2. Ask an intent question.
3. Inspect a named result.
4. Optionally choose a strategy.
5. Optionally materialize a representation.

Required docs pages:

- `docs/api/python.md`: public facade and import policy.
- `docs/python/space.md`: `Space` constructor and record handling.
- `docs/python/results.md`: result object field reference.
- `docs/python/intents.md`: neighbors, groups, embed, map, outliers, denoise,
  compare.
- `docs/python/strategies.md`: expert strategy overrides.
- `docs/python/errors.md`: exception hierarchy and troubleshooting.
- `docs/examples/python-real-data.md`: pandas and NumPy records.
- `docs/examples/python-custom-metric.md`: callable metric over strings or rows.
- `docs/examples/python-compare-spaces.md`: aligned cross-space comparison.

Example rules:

- The first Python example must use non-vector records.
- Every example starts with `from metric import Space`.
- No example imports `_impl`.
- No normal example imports an algorithm strategy before the expert section.
- Examples use result fields, not tuple unpacking.
- Examples include at least one real-data shape: pandas rows, NumPy time series,
  or image descriptors.

### Minimal String Example

```python
from metric import Space
from metric.metrics import EditDistance

space = Space(["cat", "cot", "coat", "dog"], metric=EditDistance())

neighbors = space.neighbors("cut", count=2)
for item in neighbors.neighbors:
    print(item.record, item.distance)
```

### Real Data Example

```python
from metric import Space

def row_distance(lhs, rhs):
    return abs(lhs["temperature"] - rhs["temperature"]) + abs(lhs["pressure"] - rhs["pressure"])

space = Space.from_dataframe(df, metric=row_distance, id_column="sample_id")

outliers = space.outliers(count=10)
groups = space.groups(count=4)
```

### Strategy Override Example

```python
from metric import Space
from metric.metrics import Euclidean
from metric.strategies import KMedoids

space = Space(vectors, metric=Euclidean())
groups = space.groups(strategy=KMedoids(groups=6, max_iterations=200))
```

## Notebooks

Python should be the main demo surface. Notebooks should be small, executable,
and aligned with docs examples.

Required notebooks:

```text
python/notebooks/00_strings_as_metric_space.ipynb
python/notebooks/01_dataframe_records_custom_metric.ipynb
python/notebooks/02_neighbors_groups_outliers.ipynb
python/notebooks/03_embed_map_denoise.ipynb
python/notebooks/04_compare_aligned_spaces.ipynb
python/notebooks/05_strategy_overrides.ipynb
```

Notebook requirements:

- Run from a clean install with no manual library-path edits.
- Use deterministic small datasets checked into the repo or generated inline.
- First cell imports `Space` and metrics only.
- Strategy imports appear only in the strategy notebook or expert sections.
- No hidden downloads in CI smoke tests.
- Each notebook writes no files by default.
- Each notebook has a paired `.py` or doctest-style smoke path for CI.

## Compatibility And Deprecation

Existing Python imports should continue during the transition:

```python
from metric.distance import Edit
from metric.space import Matrix
```

Compatibility policy:

- Old imports remain available through `metric.compat` or existing modules.
- Old algorithm wrappers are documented generically as legacy or expert
  compatibility, without teaching concrete algorithm names outside
  `metric.strategies`.
- New docs prefer `metric.metrics`, `metric.Space`, and intent methods.
- Tuple-return functions may remain but should have named-result replacements.
- Misspellings get aliases, for example `Manhattan` for `Manhatten`.
- Deprecation warnings must include the new intent-first replacement.

Example warning:

```text
metric.mapping.<algorithm> is compatibility API. Prefer
Space(records, metric=...).groups(strategy=metric.strategies.KMedoids(...)).
```

## Implementation Sequence

### Phase 1: Pure Python Facade

Deliver:

- `metric.Space` wrapper class.
- `Metric` protocol.
- public result objects.
- public exception hierarchy.
- constructor support for lists, NumPy arrays, and pandas DataFrames.
- no `_impl` exposure in reprs or docs.

Acceptance:

- `from metric import Space` works.
- `Space(records, metric=callable)` works.
- errors are METRIC errors, not pybind overload errors.

### Phase 2: Distance And Representation Bridge

Deliver:

- `Space.distance`.
- `Space.pairwise`.
- `Space.to_matrix`.
- `Space.to_tree`.
- `Space.to_graph`.
- fresh/stale representation version checks.

Acceptance:

- Python results match C++ distances on shared fixtures.
- stale representation errors are deterministic.

### Phase 3: First Intent Methods

Deliver:

- `space.neighbors`.
- `space.groups`.
- `space.outliers`.
- result objects for each.
- strategy overrides under `metric.strategies`.

Acceptance:

- default examples use no algorithm imports.
- expert examples import algorithms only from `metric.strategies`.

### Phase 4: Embedding, Mapping, Compare, Denoise

Deliver:

- `space.embed`.
- `space.map`.
- `space.compare`.
- `space.denoise`.
- result objects and errors for unsupported record reconstruction.

Acceptance:

- cross-space comparison works by IDs and by position.
- embedding returns a derived `Space`.
- mapping models expose `transform`.

### Phase 5: Docs, Notebooks, And CI

Deliver:

- docs pages listed above.
- required notebooks.
- notebook smoke scripts in CI.
- API parity tests with C++ examples.

Acceptance:

- no public docs use `_impl`.
- no normal docs introduce algorithm names before strategies.
- all docs examples run from a clean wheel.

## Test Matrix

Python UX tests should cover:

- `Space` with strings and edit distance.
- `Space` with NumPy vectors and Euclidean distance.
- `Space.from_dataframe` with a custom Python metric.
- stable IDs through all result objects.
- `neighbors` exact results against pairwise brute force.
- `groups` result shape and representative IDs.
- `embed` coordinate shape and embedded-space metadata.
- `map` with deterministic transform.
- `compare` with ID alignment and position alignment.
- `outliers` score ordering.
- `denoise` unsupported reconstruction error.
- strategy import isolation.
- optional dependency errors.
- compatibility aliases.
- C++ parity fixture for distances and neighbor IDs.

## Success Criteria

The Python user experience is ready when:

- A new user can install the package and run a meaningful non-vector demo in
  five lines.
- The first public object they see is `Space`.
- The first public methods they see are intent methods.
- Algorithm names are documented only under `metric.strategies`.
- Result objects are named, inspectable, and stable.
- Real-data examples cover lists, NumPy arrays, and pandas rows.
- Error messages explain how to fix the call.
- Python and C++ docs use the same concepts and result semantics.
- Notebooks run in CI and remain aligned with docs examples.
