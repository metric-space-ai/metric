# Capability Roadmap

## Purpose

This roadmap defines the product-facing capability layer for METRIC as an
engine for finite metric spaces:

```text
RecordSet + Metric -> MetricSpace -> Intent -> Strategy -> Representation -> Runtime
```

The engine layer provides reusable building blocks. The capability layer gives
users end-to-end operations with intent names:

```text
neighbors, groups, embed, map, reduce, denoise, outliers, compare
```

Algorithm names belong in the strategy layer. Public examples should teach the
intent first and reveal the selected strategy only when it matters.

## Capability Rules

- A capability is a documented recipe over engine building blocks, not a
  black-box algorithm.
- User-facing APIs use intent names. Examples: `space.neighbors(...)`,
  `space.groups(...)`, `space.compare(other, ...)`.
- Strategy objects select algorithms. Examples: `strategies::k_medoids`,
  `strategies::dbscan`, `strategies::mgc`, `strategies::pcfa`.
- Representations are replaceable execution structures: implicit distance,
  matrix cache, cover tree, kNN graph, sparse graph.
- Runtime policy is explicit before it becomes automatic: exact vs approximate,
  lazy vs materialized, serial vs parallel.
- Results are named objects with `RecordId`s, not anonymous tuples.

## Public API Shape

Python should expose capability methods on `Space`:

```python
space.neighbors(query, count=5, strategy=None)
space.groups(count=None, strategy=None)
space.embed(dimensions=2, strategy=None)
space.map(target=None, strategy=None)
space.reduce(size=None, dimensions=None, strategy=None)
space.denoise(strategy=None)
space.outliers(strategy=None)
space.compare(other, strategy=None)
```

C++ should expose equivalent intent functions:

```cpp
metric::neighbors(space, query, metric::count{5}, strategy);
metric::groups(space, strategy);
metric::embed(space, metric::dimensions{2}, strategy);
metric::map(space, target, strategy);
metric::reduce(space, options, strategy);
metric::denoise(space, strategy);
metric::outliers(space, strategy);
metric::compare(space_a, space_b, strategy);
```

Operator-level APIs may use algorithm names, but they are expert or
implementation APIs.

## Capability Matrix

| Capability | User outcome | Default pipeline | Swappable strategies | Result |
|---|---|---|---|---|
| `neighbors` | Find records similar to a query or record | `MetricSpace -> DistanceProvider/NeighborSearchIndex -> NeighborSet` | brute force, matrix cache, cover tree, kNN graph | `NeighborSet` |
| `groups` | Partition records and expose representatives | `MetricSpace -> MatrixCache or GraphTopology -> clustering operator` | k-medoids, DBSCAN, affinity propagation, graph components | `ClusteringResult` |
| `embed` | Produce inspection coordinates or a low-dimensional metric view | `MetricSpace -> NeighborGraph/MatrixCache -> embedding solver -> EmbeddedSpace` | PCFA, SOM/KOC, diffusion-style embedding when available | `EmbeddingResult` |
| `map` | Learn or apply a transformation into another space | `MetricSpace -> Mapping.fit -> MappingModel -> DerivedSpace` | PCFA mapper, KOC/SOM mapper, DSPCC, experimental neural mapper | `MappingResult` |
| `reduce` | Shrink records, dimensions, or representation cost while preserving structure | `MetricSpace -> representatives or mapper -> ReducedSpace` | medoids, graph centers, PCFA, DSPCC, SOM/KOC | `ReductionResult` |
| `denoise` | Remove local noise or redundant geometry | `MetricSpace -> NeighborGraph -> smoothing/operator -> cleaned space or scores` | graph Laplacian, manifold denoising, wavelet-assisted strategy, KOC manifold distance | `DenoisingResult` |
| `outliers` | Rank unusual records and explain their local context | `MetricSpace -> neighborhoods/graph -> anomaly scores` | kNN density, DBSCAN noise, KOC anomaly, manifold distance | `OutlierResult` |
| `compare` | Measure relation between two finite metric spaces | `MetricSpace A + MetricSpace B -> pairwise operators -> relation diagnostics` | MGC, distance correlation, matrix diagnostics | `ComparisonResult` |

## Default Recipes

### neighbors

Default behavior:

```text
small or one-off query -> implicit exact search
repeated exact queries -> MatrixCache
large repeated queries -> CoverTreeIndex if metric traits allow it
approximate query mode -> KnnGraphIndex
```

The result must always contain stable `RecordId`, distance, and optional source
representation diagnostics.

### groups

Default behavior:

```text
count provided -> k-medoids over DistanceProvider
density options provided -> DBSCAN
no count and no density options -> affinity propagation when stable
graph already materialized -> graph-aware grouping strategy
```

The user asks for `groups`; the strategy decides whether this is k-medoids,
DBSCAN, affinity propagation, or another clustering operator.

### embed

Default behavior:

```text
visualization request -> 2D embedding from a matrix or neighbor graph
metric-preserving request -> PCFA-style adapter when available
topology-preserving request -> SOM/KOC-style adapter
```

`embed` is for inspection and derived metric views. It must not imply that all
METRIC workflows start by embedding records into vectors.

### map

Default behavior:

```text
target space provided -> fit cross-space mapping model
no target, preserve geometry -> fit derived-space mapping
cluster result provided -> build quantized mapping
```

Mappings must keep the relation between original `RecordId`s and derived
records. `inverse_transform` is optional and explicit.

### reduce

Default behavior:

```text
size target -> representative selection
dimension target -> mapping strategy
runtime target -> sparse graph or cache-aware representation reduction
```

Reduction can return fewer records, fewer coordinates, or a cheaper
representation. The result must state which kind of reduction was performed.

### denoise

Default behavior:

```text
local metric noise -> neighbor-graph smoothing
manifold cleanup -> graph or KOC manifold distance
signal/image records -> wavelet-assisted strategy when configured
```

`denoise` should return cleaned records only when reconstruction is meaningful.
Otherwise it returns scores, graph edits, or a derived space.

### outliers

Default behavior:

```text
generic space -> kNN density score
density strategy selected -> DBSCAN noise points
manifold strategy selected -> KOC/manifold-distance score
```

Outlier results should include score, rank, nearest context, and strategy
diagnostics.

### compare

Default behavior:

```text
aligned records, two metrics -> MGC
precomputed representations -> reuse MatrixCache
large spaces -> sample or approximate only with explicit policy
```

`compare` should be the main public name. `MGC` is a strategy.

## Strategy Contracts

Every strategy must declare:

- required engine concepts: `DistanceProvider`, `NeighborSearchIndex`,
  `GraphTopology`, `Mapping`, or `RuntimePolicy`
- whether it needs a true metric, pseudo-metric, distance, or similarity
- whether it is exact, approximate, deterministic, and thread-safe
- preferred representation and fallback representation
- result type and diagnostics

The runtime may build or reuse representations, but it must not silently perform
expensive global materialization without a documented policy.

## MVP Order

### 1. Capability vocabulary and result objects

Deliver:

- `NeighborSet`, `ClusteringResult`, `EmbeddingResult`,
  `MappingResult`, `ReductionResult`, `DenoisingResult`,
  `OutlierResult`, `ComparisonResult`
- intent-to-strategy table in docs
- Python and C++ names reserved for all eight capabilities

Done when examples can use intent names even if some strategies are temporary
adapters.

### 2. neighbors and compare

Deliver:

- `neighbors` over implicit distance and `MatrixCache`
- `compare` over two spaces using MGC as the default strategy
- string/edit demo and cross-space MGC demo

These are the first public proof points: local search inside one space and
relation measurement between two spaces.

### 3. groups and outliers

Deliver:

- `groups` with k-medoids and DBSCAN strategies
- `outliers` with kNN density and DBSCAN-noise strategies
- process-curve/TWED demo

These make METRIC useful for practical dataset inspection without exposing
algorithm names as the first API.

### 4. reduce and embed

Deliver:

- representative-based `reduce`
- PCFA or SOM/KOC adapter for `embed`
- histogram/image EMD or SSIM demo

These show how spaces can be made cheaper or inspectable without treating vector
embeddings as the foundation.

### 5. map and denoise

Deliver:

- `Mapping` concept and at least one fitted mapping adapter
- graph or manifold `denoise` strategy
- industrial anomaly workflow demo

These complete the end-to-end product story: define geometry, inspect it,
transform it, clean it, and score unusual records.

### 6. Runtime diagnostics

Deliver:

- explicit `exact`, `approximate`, `lazy`, `materialized`, `serial`,
  `parallel` policies
- diagnostics showing selected representation and strategy
- cache reuse and stale-representation checks

Done when a user can explain why a capability used a matrix, tree, graph, or
implicit distance path.

## Demo Alignment

| Demo | Records | Metric | Capabilities | Message |
|---|---|---|---|---|
| String/edit space | strings or symbolic sequences | edit distance | `neighbors`, `groups`, `reduce` | Numerical work does not require vectors. |
| Process curves | time series | TWED | `neighbors`, `outliers`, `groups` | The metric encodes domain structure. |
| Histograms/images | distributions or image records | EMD or SSIM | `embed`, `reduce`, `groups` | Recoding costs define useful geometry. |
| Cross-space dependency | aligned modalities | separate domain metrics | `compare`, `map` | Relations can be computed between spaces. |
| Industrial anomaly workflow | sensor or process records | domain metric | `denoise`, `outliers`, `map` | End-to-end workflows compose engine parts. |

Each promoted demo must:

- start from `RecordSet + Metric`
- use intent names in the public code path
- show at least one explicit strategy swap
- avoid starting with Euclidean vectors unless the demo is explicitly a
  vector-space special case
- run in CI smoke mode or have a smaller CI-safe variant

## Non-Goals For The Capability Layer

- Do not expose a top-level catalog of algorithms as the main product surface.
- Do not force records into vectors before the metric-space workflow.
- Do not hide all execution choices behind automatic planning in the MVP.
- Do not promote experimental neural approximators as stable default strategies.
- Do not return raw tuples from new capability APIs.

