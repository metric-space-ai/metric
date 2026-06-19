# METRIC Engine Plan

## Premise

Assumption: the revival plan has been completed. The project builds, tests run, the README uses the new framing, packaging is usable, and the project is publicly positioned as:

> METRIC is a numerical computing framework for finite metric spaces and metric-space algorithms. Vector spaces are supported as a special case, not assumed as the foundation.

The next goal is to turn METRIC from a broad collection of algorithms into a coherent engine for metrics and finite metric spaces.

The right analogy is not scikit-learn. The better analogy is TensorFlow or PyTorch, but with a different fundamental object:

```text
TensorFlow/PyTorch:
Tensor -> Operations -> Runtime/Graph/Autograd

METRIC:
RecordSet + Metric -> MetricSpace -> Representations -> Operators -> Mappings -> Runtime
```

The engine should make metric-space computation feel like a single numerical model rather than a set of unrelated algorithms that happen to use distance functions.

## Design Goal

The engine must provide a rigorous, composable core model:

- records are arbitrary objects
- metrics define the numerical relation between records
- finite metric spaces are first-class objects
- representations are interchangeable materializations of a space
- operators consume spaces and produce values, structures, or derived spaces
- mappings transform spaces into other spaces
- the runtime decides when distances are computed, cached, materialized, approximated, invalidated, or reused

Existing classes should remain usable. The engine should start as a new abstraction layer and gradually absorb legacy APIs through adapters.

## What The Codebase Already Provides

The existing project already has most of the raw material:

- metric functors under `metric/distance`
- explicit finite-space structures: `Matrix`, `Tree`, `KNNGraph`
- graph topology and sparsification utilities
- metric-space statistics: MGC, Entropy, VMixing
- clustering and quantization: k-medoids, DBSCAN, Affinity Propagation, KOC, SOM
- mappings and transforms: PCFA, DSPCC, wavelet, DCT, HOG, Redif
- predictive approximators: ESN, Autoencoder, switch detectors, ensembles
- Python bindings and package facade

The main architectural problem is not missing algorithms. The problem is that the central object is implicit. Today, most components accept raw containers, concrete `Matrix` classes, Blaze matrices, or ad hoc tuples. The engine must make the finite metric space explicit.

## Current Friction To Resolve

The engine plan is driven by four codebase observations:

1. Metrics are informal callables.
   There is no central way to describe whether a functor is a true metric, pseudo-metric, similarity, structured metric, distributional metric, stateful metric, or thread-safe metric.

2. Space representations mix responsibilities.
   `Matrix`, `Tree`, and `KNNGraph` each own records and combine storage, cache, topology, and query behavior differently.

3. Operators are tied to concrete inputs.
   Some algorithms consume `Matrix`, some raw records, some Blaze matrices, some custom containers. They should consume concepts.

4. Python mirrors templates mechanically.
   It does not yet expose a clean Python model of Metric, Space, Representation, Operator, and Mapping.

## Core Vocabulary

The engine should standardize these terms:

### Record

A user-domain object. It may be a scalar, vector, string, image, sequence, histogram, graph, database row, or custom type.

### RecordId

A stable opaque identifier assigned by the space. Algorithms should not rely on physical vector indexes as semantic IDs.

```cpp
struct RecordId {
    std::uint64_t value;
};
```

### Metric

A callable object that returns the numerical relation between two records. The engine should treat existing functors as metrics through traits, not inheritance.

```cpp
template <class M, class R>
concept MetricCallable =
    requires(const M& metric, const R& a, const R& b) {
        { metric(a, b) } -> std::convertible_to<double>;
    };
```

### MetricTraits

A traits layer describes the mathematical and runtime behavior of a metric.

```cpp
enum class metric_law {
    metric,
    pseudometric,
    semimetric,
    distance,
    similarity
};

enum class record_kind {
    scalar,
    aligned_vector,
    sequence,
    image2d,
    distribution_sample,
    graph,
    metric_space,
    custom
};

template <class Metric>
struct metric_traits {
    static constexpr metric_law law = metric_law::distance;
    static constexpr record_kind record = record_kind::custom;
    static constexpr bool symmetric = true;
    static constexpr bool requires_equal_size = false;
    static constexpr bool stateful = false;
    static constexpr bool thread_safe = true;
};
```

This should be additive. Existing metrics continue to work; new code gains metadata.

### MetricSpace

A finite set of records plus a metric and stable IDs.

```cpp
template <class Record, class Metric>
class MetricSpace {
public:
    using record_type = Record;
    using metric_type = Metric;
    using id_type = RecordId;
    using distance_type = metric_result_t<Metric, Record>;

    std::size_t size() const;
    id_type id_at(std::size_t position) const;
    const Record& record(id_type id) const;
    distance_type distance(id_type a, id_type b) const;
    const Metric& metric() const;
    std::uint64_t version() const;
};
```

The first version can be an owning container. Later versions can support views, database-backed records, and streaming updates.

### Representation

A materialized or lazy view of a metric space:

- implicit distance provider
- full matrix cache
- sparse graph topology
- cover tree index
- kNN graph index
- approximate index

Representations should not be the canonical owner of records. They should attach to a `MetricSpace` and track the version they were built from.

### Operator

A computation over one or more metric spaces. Operators may return scalars, vectors, partitions, graph views, relation objects, or derived spaces.

Examples:

- nearest neighbors
- range search
- entropy
- intrinsic dimension
- MGC / cross-space correlation
- sparsification
- connected components
- representative selection
- clustering

### Mapping

A fitted or deterministic transformation from one metric space to another.

```text
fit(space) -> model
transform(space) -> MetricSpace<NewRecord, NewMetric>
inverse_transform(mapped_space) -> original-like records, when possible
```

Examples:

- PCFA
- SOM/KOC
- DSPCC
- quantization/clustering
- transforms that produce new records and metrics

### Runtime

The runtime is the policy layer that decides how to execute metric-space work:

- compute lazily or materialize
- cache distances or recompute
- use matrix, tree, graph, or approximate index
- validate metric assumptions
- release GIL in Python bindings
- invalidate stale representations after updates
- parallelize pairwise distance computation
- dispatch exact vs approximate algorithms

The runtime can be simple at first. It should become more powerful without changing the user-facing model.

## Layered Architecture

```text
metric/core
  RecordId
  MetricSpace
  Metric concepts
  metric_traits
  neighbor/result types
  versioning and invalidation primitives

metric/representations
  ImplicitDistanceProvider
  MatrixCache
  CoverTreeIndex
  KnnGraphIndex
  GraphTopology
  adapters for legacy Matrix, Tree, KNNGraph

metric/operators
  nearest
  range_search
  entropy
  mgc
  intrinsic_dimension
  kmedoids
  dbscan
  affprop
  sparsify
  representatives

metric/mappings
  fit/transform/inverse_transform interface
  PCFA
  SOM/KOC
  DSPCC
  quantization
  selected experimental adapters

metric/transforms
  deterministic record transforms
  wavelet
  DCT
  HOG
  energy transforms

metric/runtime
  execution policies
  cache/materialization planner
  parallel pairwise evaluation
  validation and diagnostics

metric/compat
  current Matrix/Tree/KNNGraph facades
  old namespace aliases
  old tuple-return wrappers
```

This structure makes it clear that algorithms are not peers of the data model. They operate on the model.

## Core Concepts To Implement First

### 1. Stable IDs And Neighbor Result

Every query API should return the same basic shape.

```cpp
template <class Distance>
struct Neighbor {
    RecordId id;
    Distance distance;
};
```

This normalizes current inconsistencies:

- `Matrix::knn` returns `(id, distance)`
- `Tree::knn` returns `(Node*, distance)`
- `KNNGraph::knn` returns ids only

### 2. DistanceProvider

The smallest useful concept for many algorithms.

```cpp
template <class Provider>
concept DistanceProvider =
    requires(const Provider& p, RecordId a, RecordId b) {
        { p.size() } -> std::convertible_to<std::size_t>;
        { p.distance(a, b) };
    };
```

First target algorithms:

- k-medoids
- DBSCAN
- Affinity Propagation
- MGC_direct
- Entropy where possible

### 3. NeighborSearchIndex

```cpp
template <class Index>
concept NeighborSearchIndex =
    requires(const Index& idx, typename Index::record_type q, std::size_t k) {
        { idx.nearest(q) };
        { idx.knn(q, k) };
    };
```

Legacy adapters:

- `Matrix` -> brute-force exact search
- `Tree` -> cover-tree search
- `KNNGraph` -> graph-based approximate search

### 4. GraphTopology

```cpp
template <class G>
concept GraphTopology =
    requires(const G& g, RecordId id) {
        { g.neighbors(id) };
    };
```

This separates graph topology from metric distance.

### 5. Result Objects Instead Of Tuples

Tuple returns make composition hard. Add result types:

```cpp
struct ClusteringResult {
    std::vector<RecordId> assignment;
    std::vector<RecordId> representatives;
    std::vector<std::size_t> counts;
};

struct CorrelationResult {
    double value;
    // optional diagnostics later
};
```

Compatibility wrappers can keep tuple APIs alive.

## Execution Model

The engine should support an explicit materialization model.

```cpp
auto space = metric::make_space(records, metric::Edit<char>{});

auto implicit = space.representation<metric::Implicit>();
auto matrix = space.materialize<metric::MatrixCache>();
auto tree = space.index<metric::CoverTreeIndex>();
auto graph = space.materialize<metric::KnnGraphIndex>(metric::k{8});
```

Operators should request capabilities, not concrete classes.

```cpp
auto e = metric::operators::entropy(space);
auto nn = metric::operators::nearest(space, query, metric::exact);
auto nn_fast = metric::operators::nearest(space, query, metric::approximate);
auto clusters = metric::operators::kmedoids(matrix, 5);
```

The runtime can decide:

- direct metric calls for small spaces
- full distance matrix for repeated global operators
- tree index for repeated point queries
- sparse graph for graph operators
- approximate index where allowed

## C++ API Target

The C++ API should be explicit and template-friendly.

```cpp
using Record = std::string;
auto metric = metric::Edit<char>{};

auto space = metric::make_space<Record>(records, metric);

auto matrix = metric::representations::matrix(space);
auto graph = metric::representations::knn_graph(space, metric::k{8});

auto neighbors = metric::operators::knn(space, query, metric::k{3});
auto entropy = metric::operators::entropy(space, metric::entropy_options{.k = 7});
auto clusters = metric::operators::kmedoids(matrix, metric::clusters{5});
```

Mapping API:

```cpp
auto model = metric::mappings::pcfa(metric::dimensions{2}).fit(space);
auto feature_space = model.transform(space);
auto reconstructed = model.inverse_transform(feature_space);
```

Cross-space API:

```cpp
auto image_space = metric::make_space(images, metric::SSIM<double, Pixel>{});
auto curve_space = metric::make_space(curves, metric::TWED<double>{});

auto relation = metric::operators::mgc(image_space, curve_space);
```

## Python API Target

Python should be a facade over the engine, not a dump of pybind template names.

```python
from metric import Space
from metric.metrics import EditDistance, TWED, EMD, Euclidean
from metric.operators import entropy, mgc, knn, kmedoids
from metric.representations import MatrixCache, CoverTreeIndex, KnnGraphIndex

space = Space(records, metric=EditDistance())

matrix = space.to_matrix()
tree = space.to_tree()
graph = space.to_knn_graph(k=8)

neighbors = knn(space, query, k=3)
score = entropy(space, k=7)
clusters = kmedoids(matrix, k=5)
```

Mapping API:

```python
from metric.mappings import PCFA

model = PCFA(dimensions=2).fit(space)
feature_space = model.transform(space)
reconstructed = model.inverse_transform(feature_space)
```

Principles:

- `_impl` remains private
- no star imports in public modules
- no Blaze objects in public Python API unless explicitly requested
- `Metric` is a `Protocol`
- heavy C++ calls release the GIL
- compatibility imports remain available

## Migration Strategy

### Rule 1: Do Not Rewrite Everything

The first engine layer should wrap existing classes:

- `MatrixCache` wraps or delegates to `metric::Matrix`
- `CoverTreeIndex` wraps or delegates to `metric::Tree`
- `KnnGraphIndex` wraps or delegates to `metric::KNNGraph`
- operators initially call existing implementations internally

### Rule 2: New Concepts First, Algorithms Second

Implement the concepts and adapters before moving algorithms.

If the engine starts by rewriting clustering, mapping, and correlation, it will stall. If it starts with `MetricSpace`, `DistanceProvider`, `Neighbor`, and `Representation`, existing algorithms can move gradually.

### Rule 3: Compatibility Facades Stay

Existing code should keep compiling:

```cpp
metric::Matrix<Record, Metric> matrix(records);
```

But recommended docs should use:

```cpp
auto space = metric::make_space(records, metric);
auto matrix = metric::representations::matrix(space);
```

### Rule 4: Tuple APIs Become Legacy

New operators return named result objects. Old tuple-return functions remain as wrappers.

### Rule 5: Runtime Policies Are Opt-In First

The first runtime should be explicit, not magical. Let users choose materialization and exact/approximate strategies. Add automatic planning after the concepts stabilize.

## Milestones

### Milestone 1: Engine Specification And Skeleton

Deliverables:

- `metric/core/record_id.hpp`
- `metric/core/metric_traits.hpp`
- `metric/core/metric_space.hpp`
- `metric/core/neighbor.hpp`
- `metric/core/concepts.hpp`
- docs page: "METRIC Engine Model"
- compile-only tests for concepts

Acceptance criteria:

- existing metric functors can be used with `make_space`
- no old public API breaks
- custom user metric works in a minimal example

### Milestone 2: Representation Adapters

Deliverables:

- `ImplicitDistanceProvider`
- `MatrixCache` adapter
- `CoverTreeIndex` adapter
- `KnnGraphIndex` adapter
- common `Neighbor` return type
- version tracking for stale representations

Acceptance criteria:

- same `MetricSpace` can produce matrix, tree, and graph representations
- all representations share stable `RecordId`
- `knn` result shape is identical across exact matrix/tree/graph backends

### Milestone 3: First Engine Operators

Prioritize algorithms already close to metric-space native:

- nearest / knn / range search
- k-medoids
- DBSCAN
- Affinity Propagation
- MGC
- Entropy

Deliverables:

- `metric/operators/nearest.hpp`
- `metric/operators/clustering.hpp`
- `metric/operators/correlation.hpp`
- `metric/operators/entropy.hpp`
- named result objects
- compatibility wrappers

Acceptance criteria:

- operators accept concepts, not concrete legacy classes
- current tests pass
- new tests show the same operator working on at least two representations

### Milestone 4: Mapping Interface

Deliverables:

- `Mapping` concept
- `fit/transform/inverse_transform` convention
- PCFA adapter
- KOC/SOM adapter
- cluster-result-as-space adapter

Acceptance criteria:

- mapping consumes `MetricSpace`
- mapping returns a derived `MetricSpace` or typed result
- at least one mapping supports inverse/reconstruction

### Milestone 5: Python Facade

Deliverables:

- `metric.Space`
- `metric.metrics` aliases and protocols
- `metric.representations`
- `metric.operators`
- `metric.mappings`
- lazy compatibility layer for current modules

Acceptance criteria:

- Python examples do not expose `_impl` or Blaze by default
- `Space(...).to_matrix().distance(i,j)` works
- `entropy(space)`, `mgc(space_a, space_b)`, and `kmedoids(space, k)` work
- old imports continue with deprecation warnings where appropriate

### Milestone 6: Runtime Planner

Deliverables:

- execution policies:
  - `exact`
  - `approximate`
  - `lazy`
  - `materialized`
  - `parallel`
- materialization heuristics
- distance cache policy
- invalidation policy
- concurrency audit for stateful metrics

Acceptance criteria:

- repeated global operators reuse matrix caches
- repeated query operators use tree/graph indexes
- unsafe stateful metrics are either serialized or documented
- Python heavy calls release the GIL

### Milestone 7: Flagship Demonstrations

Deliverables:

- String/edit finite metric space
- time-series/TWED process-curve anomaly workflow
- histogram/image EMD or SSIM workflow
- cross-space MGC workflow
- representation swap demo: implicit vs matrix vs tree vs graph
- benchmark comparing domain metric vs naive vector embedding baseline

Acceptance criteria:

- demos show why vector-space-only thinking is too narrow
- demos use the unified engine API
- every flagship demo is CI-smoke-tested

## Demonstration Strategy

The demos should prove the frame, not just show features.

### Demo 1: Non-Vector Space

Records: strings or symbolic sequences.

Metric: edit distance.

Show:

- finite metric space construction
- full matrix materialization
- nearest neighbor
- clustering

Message:

No vector embedding is needed to do numerical work.

### Demo 2: Structured Time-Series Space

Records: process curves.

Metric: TWED.

Show:

- anomaly detection
- kNN graph
- representative selection
- comparison with Euclidean on aligned vectors

Message:

The right metric captures structure that vector-space assumptions miss.

### Demo 3: Distribution/Image Space

Records: histograms or flattened images.

Metric: EMD or SSIM.

Show:

- space construction
- graph representation
- sparsification
- clustering/representatives

Message:

Structure and recoding costs matter.

### Demo 4: Cross-Space Dependency

Records: two aligned modalities, e.g. image-derived records and sensor curves.

Metrics: SSIM/EMD for one space, TWED or distribution metric for the other.

Operator: MGC.

Show:

- correlation between metric spaces without forcing both into one vector space

Message:

Relations between data sources can be computed at the metric-space level.

## Testing Strategy

The engine needs tests for contracts, not only outputs.

### Metric Contract Tests

- non-negativity
- identity or pseudo-metric caveat
- symmetry
- triangle inequality where required
- equal-size behavior where required
- deterministic behavior
- thread-safety markers for stateful metrics

### Representation Consistency Tests

For a fixed space:

- implicit distance equals matrix cache distance
- matrix cache nearest equals brute force
- tree nearest matches exact search for test fixtures
- graph approximate search respects documented approximation behavior
- updates invalidate stale representations

### Operator Consistency Tests

Same operator over different providers:

- `kmedoids(MatrixCache)` vs `kmedoids(DistanceProvider)`
- `dbscan(MatrixCache)` vs generic provider
- MGC raw-record path vs matrix path
- entropy raw-record path vs space path

### Python API Tests

- public import surface
- no `_impl` leakage in docs examples
- custom Python callable metric
- NumPy inputs
- non-vector records where possible
- compatibility imports

## Performance Strategy

The engine should expose performance as a representation decision.

Key principles:

- do not hide O(n^2) pairwise materialization
- make exact vs approximate explicit
- cache only with a space version
- parallelize expensive pairwise metrics
- allow metrics to declare stateful/thread-unsafe behavior
- support expensive metrics through surrogate approximators later

Future optimization directions:

- blocked pairwise distance computation
- SIMD/vectorized metric kernels where appropriate
- sparse graph construction without full matrix materialization
- approximate nearest-neighbor strategies for expensive metrics
- learned surrogates as acceleration, not as foundation

## Governance Of Stable vs Experimental Code

Engine Core:

- Metric traits/concepts
- MetricSpace
- representations
- nearest/range
- entropy/MGC
- k-medoids/DBSCAN/AffProp after adapter work

Engine Mappings:

- PCFA
- SOM/KOC
- DSPCC after cleanup
- clustering-as-mapping

Experimental:

- ESN
- Autoencoder
- Redif
- switch detectors
- ensembles/DT
- EPMG
- hysteresis fitting
- some transform-specific workflows

Experimental modules can remain, but they should not define the engine vocabulary.

## Naming Decisions

Recommended public names:

- `MetricSpace`, not only `Space`
- `MatrixCache`, not `Matrix`, for the representation role
- `CoverTreeIndex`, not only `Tree`
- `KnnGraphIndex`, not only `KNNGraph`
- `GraphTopology`, for topology-only graph data
- `DistanceProvider`, for pairwise distance access
- `NeighborSearchIndex`, for `nn/knn/rnn`
- `Operator`, for computations over spaces
- `Mapping`, for fitted or deterministic transformations

Compatibility aliases:

- `Manhattan = Manhatten`
- `Minkowski = P_norm`
- `CramerVonMises = CramervonMises`
- old `Matrix`, `Tree`, `KNNGraph` remain available

## First Implementation Sequence

1. Add `metric/core` with concepts, traits, IDs, neighbor result, and owning `MetricSpace`.
2. Add adapters over existing `Matrix`, `Tree`, and `KNNGraph`.
3. Add generic `operators::knn` and `operators::range_search`.
4. Move `kmedoids`, `dbscan`, and `AffProp` behind `DistanceProvider`.
5. Move MGC/Entropy to accept `MetricSpace` or `DistanceProvider`.
6. Add result objects and keep tuple wrappers.
7. Add Python `Space` facade and public `operators`.
8. Add flagship demos.
9. Add runtime policies and automatic materialization heuristics.

## Key Open Decisions

- Should the owning core type be named `MetricSpace` publicly, or `Space` with `MetricSpace` as the C++ type?
- Should representations own references/views, shared pointers, or value copies of spaces?
- How strict should metric-law validation be at runtime?
- Should approximate representations be part of core or a separate extension namespace?
- Should Python allow arbitrary object records initially, or focus first on NumPy/string/list records?
- Should graph topology be weighted by metric distances by default, or remain topology-only unless requested?
- How should expensive, stateful metrics like EMD be made thread-safe?

## Success Criteria

The engine is successful when users naturally write code in this shape:

```text
define records
define metric
construct metric space
choose representation
run operators
derive mapped spaces
compose results
```

Instead of this shape:

```text
pick unrelated algorithm
massage data into its expected container
guess which metric overload compiles
unwrap tuple result
repeat for next algorithm
```

The visible shift should be:

```text
from algorithm collection
to metric-space computation engine
```

That is the architectural move that makes METRIC distinct.
