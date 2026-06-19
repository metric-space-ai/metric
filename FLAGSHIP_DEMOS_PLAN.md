# Flagship Demos Plan

## Purpose

This plan defines the flagship demos that should prove METRIC as an engine for
finite metric spaces, not as a catalog of algorithms.

The demos must make the central model visible:

```text
RecordSet + Metric -> MetricSpace -> Intent -> Strategy -> Representation -> Runtime
```

The user-facing story is:

- records may be strings, vectors, images, time series, mixed records, or learned
  latent records
- the metric defines the geometry
- finite metric spaces are first-class objects
- vectors are one supported record type, not the foundation
- algorithm names belong in strategy objects and diagnostics
- public examples lead with intent names such as `neighbors`, `groups`,
  `outliers`, `embed`, `map`, and `diagnostics`

## Demo Rules

Every flagship demo must follow the same public API shape:

1. Construct a space from records and a metric.
2. Inspect the space, metric traits, IDs, and representation state.
3. Ask intent-level questions: neighbors, groups, and outliers.
4. Embed or map the space when a coordinate view or reusable transform is useful.
5. Show diagnostics: strategy, representation, runtime policy, quality, and
   warnings.
6. Name the composed engine modules used by the demo.

Every demo must include:

- a small fixture that runs in CI smoke mode
- an optional richer fixture for notebooks, docs, or gallery runs
- stable `RecordId` lineage in all results
- at least one explicit strategy swap
- at least one runtime or representation diagnostic
- one sentence explaining why the metric is the domain model

Every demo must avoid:

- starting by embedding records into vectors unless the demo is the explicit
  vector-space special case
- naming the demo after an algorithm
- exposing direct algorithm calls as the primary path
- returning raw tuples in new engine examples
- silently materializing an O(n^2) matrix without a policy or diagnostic

## Common API Form

Python examples should use this shape:

```python
from metric import Space
from metric import runtime, strategies
from metric.metrics import EditDistance

space = Space(
    records,
    metric=EditDistance(),
    ids=record_ids,
    name="demo_space",
)

profile = space.inspect()

neighbor_result = space.neighbors(
    query=query_record,
    count=5,
    strategy=strategies.ExactSearch(),
    runtime=runtime.Policy(lazy=True, exact=True),
)

group_result = space.groups(
    count=3,
    strategy=strategies.Medoids(),
)

outlier_result = space.outliers(
    strategy=strategies.LocalDensity(k=5),
)

embedding_result = space.embed(
    dimensions=2,
    strategy=strategies.DiffusionEmbedding(),
    runtime=runtime.Policy(materialize="knn_graph"),
)

mapping_result = space.map(
    strategy=strategies.StructurePreservingMap(),
)

diagnostics = embedding_result.diagnostics()
```

C++ examples should use the same intent order:

```cpp
auto space = metric::make_space(records, metric, metric::ids(record_ids));

auto profile = metric::inspect(space);

auto neighbor_result = metric::neighbors(
    space,
    query_record,
    metric::count{5},
    metric::strategies::exact_search{},
    metric::runtime_policy{.lazy = true, .exact = true});

auto group_result = metric::groups(
    space,
    metric::count{3},
    metric::strategies::medoids{});

auto outlier_result = metric::outliers(
    space,
    metric::strategies::local_density{.k = 5});

auto embedding_result = metric::embed(
    space,
    metric::dimensions{2},
    metric::strategies::diffusion_embedding{});

auto mapping_result = metric::map(
    space,
    metric::strategies::structure_preserving_map{});
```

The exact class names may evolve. The public shape should not.

## Shared Result Contract

All flagship demos should print or assert the same result fields.

### Inspect Result

- `space_name`
- `record_count`
- `record_kind`
- `metric_name`
- `metric_law`
- `symmetric`
- `requires_equal_size`
- `distance_type`
- `space_version`
- `representations_available`
- `sample_distances`

### Capability Results

- `source_space`
- `source_space_version`
- `source_ids`
- `result_ids`
- `strategy`
- `representation_used`
- `runtime_policy`
- `exact`
- `warnings`
- `diagnostics`

### Embedding And Mapping Results

- `embedded_space` or `target_space`
- `target_metric`
- `source_to_target_lineage`
- `supports_transform`
- `supports_inverse_transform`
- `preservation_scores`
- `training_history`, when fitted
- `reconstruction_scores`, when inverse transform exists

## Demo 1: String Edit Space

### Message

Numerical computation can start from symbolic records. The edit metric is the
geometry; vectors are optional inspection output.

### Fixture

- CI smoke: 30 to 100 short strings with known families, typos, prefixes, and
  suffixes.
- Gallery: product names, command names, DNA-like tokens, or log event symbols.

### Construct Space

```python
space = Space(
    strings,
    metric=metrics.EditDistance(insert=1, delete=1, substitute=1),
    ids=string_ids,
    name="string_edit_space",
)
```

### Inspect

Show:

- `record_kind = sequence`
- `metric_law = metric`
- distance examples such as one insertion, one substitution, and unrelated words
- no vector dimension in the source space

### Neighbors, Groups, Outliers

```python
neighbors = space.neighbors("metric", count=5)
groups = space.groups(count=4, strategy=strategies.Medoids())
outliers = space.outliers(strategy=strategies.LocalDensity(k=5))
```

Expected user story:

- neighbors find typo variants or symbolic variants
- groups expose string families through representative strings
- outliers identify strings that do not share edit structure with any group

### Embed Or Map

```python
embedding = space.embed(
    dimensions=2,
    strategy=strategies.DiffusionEmbedding(),
)

representatives = space.reduce(
    size=12,
    strategy=strategies.Representatives(),
)
```

The embedding is a coordinate view of the edit-distance space. It must preserve
source string IDs and must not replace the source geometry.

### Diagnostics

Show:

- whether distances were lazy, cached, or matrix-backed
- graph connectedness for the embedding strategy
- stress or preservation score
- representative coverage and max assignment distance

### Composed Engine Modules

```text
RecordSet<string>
  -> EditDistance metric
  -> MetricSpace<string, EditDistance>
  -> implicit DistanceProvider or MatrixCache
  -> neighbors/groups/outliers operators
  -> diffusion embedding or representative reduction strategy
  -> RuntimePolicy + diagnostics
```

## Demo 2: Vector Space As A Special Case

### Message

Vector spaces work naturally in METRIC, but the API is the same as for strings,
images, time series, and structured records.

### Fixture

- CI smoke: 2D or 4D points in three clusters plus a few outliers.
- Gallery: embedding vectors from an external model, sensor summary vectors, or
  hand-built feature vectors.

### Construct Space

```python
space = Space(
    vectors,
    metric=metrics.Euclidean(),
    ids=point_ids,
    name="vector_space_special_case",
)
```

### Inspect

Show:

- `record_kind = aligned_vector`
- `metric_law = metric`
- vector dimension as record metadata, not as an engine assumption
- available representations: implicit distances, matrix cache, tree index, kNN
  graph

### Neighbors, Groups, Outliers

```python
neighbors = space.neighbors(vectors[0], count=8)
groups = space.groups(count=3, strategy=strategies.Medoids())
outliers = space.outliers(strategy=strategies.LocalDensity(k=8))
```

Add an explicit strategy swap:

```python
exact_neighbors = space.neighbors(
    vectors[0],
    count=8,
    strategy=strategies.ExactSearch(),
)

indexed_neighbors = space.neighbors(
    vectors[0],
    count=8,
    strategy=strategies.TreeSearch(),
)
```

Expected user story:

- vector records reuse the same intent APIs
- the runtime may choose vector-aware acceleration
- the result object has the same shape as the string demo

### Embed Or Map

```python
embedding = space.embed(
    dimensions=2,
    strategy=strategies.MDS(),
)

reduced = space.reduce(
    dimensions=2,
    strategy=strategies.PCFA(),
)
```

This demo may start with vectors because it is explicitly the vector special
case. It should still explain that `embed` and `reduce` are engine capabilities,
not prerequisites for using METRIC.

### Diagnostics

Show:

- exact search vs tree search runtime policy
- matrix cache reuse if groups and embedding share distances
- MDS stress or PCFA reconstruction loss
- outlier score with nearest-neighbor context

### Composed Engine Modules

```text
RecordSet<vector>
  -> Euclidean metric
  -> MetricSpace<vector, Euclidean>
  -> TreeIndex, MatrixCache, or KnnGraphIndex
  -> neighbors/groups/outliers operators
  -> MDS embedding or PCFA reduction strategy
  -> RuntimePolicy + diagnostics
```

## Demo 3: Image Or Histogram Space

### Message

Images and histograms can be compared by structured costs. EMD and SSIM encode
domain geometry without requiring a learned vector embedding first.

### Fixture

- CI smoke: 16 to 64 small grayscale images or 2D histograms with shifts,
  occlusions, and intensity changes.
- Gallery: thumbnails, segmentation masks, color histograms, or image patches.

### Construct Space

EMD histogram variant:

```python
ground = metrics.GridGroundDistance(width=8, height=8)

space = Space(
    histograms,
    metric=metrics.EMD(ground_distance=ground, extra_mass_penalty="auto"),
    ids=image_ids,
    name="histogram_transport_space",
)
```

SSIM image variant:

```python
space = Space(
    images,
    metric=metrics.SSIMDistance(dynamic_range=255, window=7),
    ids=image_ids,
    name="image_structure_space",
)
```

The first shipped demo should choose one default path. The other should be an
explicit variant using the same API.

### Inspect

Show:

- `record_kind = image2d` or `distribution_sample`
- metric traits for EMD or SSIM-derived distance
- image shape, histogram mass policy, and metric parameters
- whether the metric is expensive and should use cache or parallel policy

### Neighbors, Groups, Outliers

```python
neighbors = space.neighbors(query_image, count=6)
groups = space.groups(count=4, strategy=strategies.Medoids())
outliers = space.outliers(strategy=strategies.LocalDensity(k=6))
```

Expected user story:

- EMD groups shifted mass patterns that Euclidean pixel distance may split
- SSIM groups structurally similar images under lighting changes
- outliers identify damaged, blank, or structurally isolated records

### Embed Or Map

```python
embedding = space.embed(
    dimensions=2,
    strategy=strategies.DiffusionEmbedding(knn=8),
    runtime=runtime.Policy(materialize="knn_graph", parallel=True),
)

prototypes = space.reduce(
    size=10,
    strategy=strategies.Representatives(),
)
```

Optional gallery extension:

```python
som_map = space.map(
    strategy=strategies.SOM(grid=(6, 6)),
)
```

### Diagnostics

Show:

- cost of distance evaluations
- cache hit rate after repeated capabilities
- selected representation: dense matrix, kNN graph, or approximate graph
- diffusion graph connectedness
- representative coverage or SOM quantization error

### Composed Engine Modules

```text
RecordSet<image or histogram>
  -> EMD or SSIM-derived metric
  -> MetricSpace<image, metric>
  -> MatrixCache or KnnGraphIndex
  -> neighbors/groups/outliers operators
  -> diffusion embedding, representatives, or SOM map strategy
  -> parallel RuntimePolicy + diagnostics
```

## Demo 4: Time Series Time-Warp Space

### Message

Time series should be compared by alignment costs when phase shifts and temporal
elasticity matter. The time-warp metric is the domain model.

### Fixture

- CI smoke: 40 short sequences with shifted peaks, stretched events, missing
  segments, and a few abnormal curves.
- Gallery: process curves, sensor windows, demand curves, or waveform snippets.

### Construct Space

```python
space = Space(
    curves,
    metric=metrics.TWED(penalty=1.0, elastic=0.5),
    ids=curve_ids,
    name="time_warp_space",
)
```

### Inspect

Show:

- `record_kind = sequence`
- `requires_equal_size = false`
- TWED parameters as metric metadata
- distance examples for shifted vs genuinely different curves

### Neighbors, Groups, Outliers

```python
neighbors = space.neighbors(query_curve, count=5)
groups = space.groups(strategy=strategies.DBSCAN(radius=2.5, min_count=4))
outliers = space.outliers(strategy=strategies.DBSCANNoise())
```

Strategy swap:

```python
density_outliers = space.outliers(
    strategy=strategies.LocalDensity(k=5),
)
```

Expected user story:

- neighbors find curves with similar events even when events are shifted
- groups separate process regimes
- outliers identify unusual process runs or misaligned events

### Embed Or Map

```python
embedding = space.embed(
    dimensions=2,
    strategy=strategies.DiffusionEmbedding(knn=10),
)

compressed = space.reduce(
    dimensions=3,
    strategy=strategies.PCFA(),
)
```

Optional map path:

```python
feature_model = space.map(
    strategy=strategies.DSPCC(features=4),
)

feature_space = feature_model.transform(space)
```

### Diagnostics

Show:

- time-warp metric cost and runtime policy
- whether distances were clipped, cached, or computed exactly
- density parameters and noise explanation
- embedding preservation by neighborhood recall
- PCFA or DSPCC reconstruction error, when used

### Composed Engine Modules

```text
RecordSet<time_series>
  -> TWED metric
  -> MetricSpace<time_series, TWED>
  -> MatrixCache or lazy DistanceProvider
  -> neighbors/groups/outliers operators
  -> diffusion embedding, PCFA reduction, or DSPCC map strategy
  -> RuntimePolicy + diagnostics
```

## Demo 5: Mixed Structured Records With Custom Metric

### Message

Real records are often mixed objects. METRIC should let users define the domain
metric directly instead of flattening everything into one vector prematurely.

### Fixture

- CI smoke: 50 records with numeric summaries, categorical status, short text,
  histogram bins, and a time-series window.
- Gallery: industrial inspections, tickets, process batches, or product quality
  observations.

### Example Record

```python
record = {
    "temperature_summary": [72.1, 73.4, 75.0],
    "status": "warmup",
    "message": "valve drift warning",
    "spectrum": [0.0, 0.2, 0.5, 0.3],
    "curve": [1.0, 1.2, 1.5, 1.3, 1.1],
}
```

### Construct Space

```python
metric = metrics.Composite(
    fields={
        "temperature_summary": metrics.Euclidean(),
        "status": metrics.Discrete(mismatch=2.0),
        "message": metrics.EditDistance(),
        "spectrum": metrics.EMD(ground_distance=metrics.LineGroundDistance(4)),
        "curve": metrics.TWED(penalty=1.0, elastic=0.5),
    },
    weights={
        "temperature_summary": 0.20,
        "status": 0.15,
        "message": 0.15,
        "spectrum": 0.25,
        "curve": 0.25,
    },
    normalize=True,
)

space = Space(
    records,
    metric=metric,
    ids=record_ids,
    name="mixed_record_space",
)
```

### Inspect

Show:

- `record_kind = custom`
- per-field metric traits
- weights and normalization policy
- distance explanation with per-field contributions for selected pairs

### Neighbors, Groups, Outliers

```python
neighbors = space.neighbors(query_record, count=5)
groups = space.groups(count=5, strategy=strategies.Medoids())
outliers = space.outliers(strategy=strategies.LocalDensity(k=6))
```

Expected user story:

- neighbors are close because several field metrics agree, not because the
  record was flattened
- groups expose representative structured records
- outlier explanations identify the fields that made a record unusual

### Embed Or Map

```python
embedding = space.embed(
    dimensions=2,
    strategy=strategies.DiffusionEmbedding(knn=8),
)

case_map = space.map(
    target=label_space,
    strategy=strategies.StructurePreservingMap(),
)
```

Optional reduction:

```python
summary_space = space.reduce(
    size=12,
    strategy=strategies.Representatives(explain=True),
)
```

### Diagnostics

Show:

- per-field distance contribution for neighbors and outliers
- metric trait composition and warnings if triangle inequality cannot be proven
- representation choice and cache reuse
- group representative coverage
- map preservation scores against the target space, if target is provided

### Composed Engine Modules

```text
RecordSet<mixed_record>
  -> Composite custom metric
  -> MetricSpace<mixed_record, Composite>
  -> explainable DistanceProvider
  -> neighbors/groups/outliers operators
  -> diffusion embedding, representative reduction, or target map strategy
  -> RuntimePolicy + diagnostics
```

## Demo 6: Learnable Structure Map

### Message

Learnable mappings are fitted engine strategies. A PHATE-AE-style strategy
belongs behind `map`; the user intent is to learn a reusable
structure-preserving map.

### Fixture

- CI smoke: a small non-vector source space with synthetic records and a tiny
  training budget, or a vector fixture where the source metric is still explicit.
- Gallery: strings, images, or time series with enough records to show
  out-of-sample transform and approximate inverse transform.

### Construct Space

The source space should intentionally be non-vector when practical:

```python
space = Space(
    curves,
    metric=metrics.TWED(penalty=1.0, elastic=0.5),
    ids=curve_ids,
    name="learnable_time_warp_space",
)
```

If the implementation first supports vectors only, the demo title and text must
still say that this is an implementation limitation of the initial strategy, not
an engine assumption.

### Inspect

Show:

- source metric traits
- whether the strategy can consume the source metric directly or needs a
  precomputed representation
- training fixture size and runtime policy
- out-of-sample support level

### Neighbors, Groups, Outliers

Before training, run ordinary engine capabilities:

```python
neighbors = space.neighbors(query_curve, count=5)
groups = space.groups(count=4, strategy=strategies.Medoids())
outliers = space.outliers(strategy=strategies.LocalDensity(k=5))
```

These establish the source geometry that the learned map must preserve.

### Embed Or Map

```python
model = space.map(
    strategy=strategies.PhateAE(
        dimensions=2,
        diffusion_knn=8,
        epochs=20,
        seed=7,
    ),
    runtime=runtime.Policy(materialize="knn_graph", deterministic=True),
)

mapped_space = model.transform(space)
new_mapped_records = model.transform(new_records)
reconstructed = model.inverse_transform(mapped_space)
```

The public action is `map`. PHATE-AE appears as strategy provenance and in a
strategy options section.

### Diagnostics

Show:

- source representation used to build diffusion geometry
- diffusion time and graph connectedness
- training loss and early-stop state
- neighborhood preservation from source space to mapped space
- reconstruction error when inverse transform is supported
- out-of-sample support level: none, approximate, parametric, or incremental
- random seed and deterministic policy

### Composed Engine Modules

```text
RecordSet<source_record>
  -> source metric
  -> MetricSpace<source_record, source_metric>
  -> KnnGraphIndex or MatrixCache
  -> diffusion geometry operator
  -> PHATE-AE-style map strategy
  -> MappingModel with transform and optional inverse_transform
  -> RuntimePolicy + diagnostics
```

## Cross-Demo Sequence

The gallery should be ordered to teach the engine model:

1. String edit space: non-vector records first.
2. Time series time-warp space: metric encodes alignment.
3. Image or histogram space: structured costs and expensive metrics.
4. Mixed structured records: custom domain metric composition.
5. Vector space special case: vectors fit the same API.
6. Learnable structure map: fitted mapping as a composed engine capability.

This order prevents users from assuming that METRIC starts with vectors.

## Runtime And Representation Story

Each demo must show at least one representation decision:

| Situation | Expected representation story |
|---|---|
| Small string fixture | Lazy distance provider first, optional matrix cache reuse |
| Vector fixture | Tree or vector-aware exact search may replace brute force |
| Image or EMD fixture | Expensive metric uses parallel materialization or kNN graph |
| Time-series fixture | TWED uses cache or clipping policy when repeated |
| Mixed records | Explainable distance provider exposes per-field contributions |
| Learnable map | kNN graph or matrix cache feeds diffusion geometry and training |

Diagnostics should be written for users, not implementers. A user should be able
to answer:

- Which metric defined the space?
- Which representation was built?
- Which strategy executed the intent?
- Was execution exact, approximate, lazy, materialized, serial, or parallel?
- What quality score says the result preserved the source geometry?
- Which warnings affect interpretation?

## Documentation Deliverables

Each flagship demo should eventually produce:

- `examples/engine/<demo_name>.py`
- `examples/core/<demo_name>.cpp`, when the C++ engine API is available
- a short docs page under `docs/examples/`
- a CI smoke test using the small fixture
- one saved expected-output snippet for diagnostics
- optional notebook or gallery asset for richer visualization

The first implementation may keep demos in Python only if the engine facade is
first exposed there. The plan should still preserve the C++ intent shape.

## Acceptance Criteria

A demo is flagship-ready when:

- it starts from `RecordSet + Metric`
- it uses the common API form
- it never requires vectorization before constructing the source space
- it uses intent names in public code
- strategy names appear only as strategy arguments, diagnostics, or expert notes
- result objects preserve `RecordId` lineage
- diagnostics state representation and runtime policy
- the demo has a CI-safe fixture
- the demo has one explicit strategy or representation swap
- the demo text explains what the metric means in the domain

The full demo set is flagship-ready when:

- at least four demos start from non-vector records
- the vector demo is clearly labeled as a special case
- at least one demo uses a custom mixed metric
- at least one demo uses an expensive structured metric such as EMD, SSIM, or
  TWED
- at least one demo fits a reusable mapping model
- all demos expose consistent result fields
- docs can compare demos line-by-line and show the same engine shape

## Open Design Decisions

- Should SSIM be exposed as a distance wrapper by default, or as a similarity
  with an explicit conversion strategy?
- Which image fixture is small enough for CI while still making EMD or SSIM
  meaningful?
- Should composite metrics provide formal metric-law validation, runtime
  warnings, or both?
- Which strategies are stable enough for the first public `embed` and `map`
  examples?
- How should expensive metric cache keys encode metric parameters and source
  space version?
- What is the minimum PHATE-AE-style smoke fixture that proves the API without
  making CI slow or flaky?

## First Milestone

Build the first two demos in this order:

1. String edit space.
2. Vector space as a special case.

This gives one non-vector proof point and one familiar baseline while forcing
both to share the same API and result contracts.

## Second Milestone

Add the expensive native metric demos:

1. Time series time-warp space.
2. Image or histogram space.

These prove that METRIC can operate where the meaningful distance is not a
simple vector norm and where runtime policy matters.

## Third Milestone

Add the composed and fitted demos:

1. Mixed structured records with custom metric.
2. Learnable structure map.

These prove that METRIC can compose domain metrics and fit reusable mappings
without changing the public engine model.
