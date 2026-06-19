# Representation Layer Plan

## Purpose

METRIC should be understood as an engine for finite metric spaces:

```text
RecordSet + Metric -> MetricSpace -> Intent -> Strategy -> Representation -> Runtime
```

This document defines the representation layer in that model. A representation is a view, cache, topology, or index over a `MetricSpace`. It is not the canonical owner of records and it is not a separate user concept.

The first concrete goal is to make the existing structures:

- `metric/space/matrix.hpp`
- `metric/space/tree.hpp`
- `metric/space/knn_graph.hpp`

usable as interchangeable representations of one `MetricSpace`, while leaving the existing public classes available through compatibility adapters.

## Current State

The existing classes already provide the raw behavior needed by the engine:

- `Matrix<RecType, Metric>` stores records, stores a sparse Blaze distance matrix, supports `insert`, `insert_if`, `erase`, `set`, `nn`, `knn`, `rnn`, and distance lookup by pair of indexes.
- `Tree<RecType, Metric>` implements a cover tree, stores records and node pointers, supports nearest/range search, clustering helpers, serialization, graph-distance helpers, and matrix conversion.
- `KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>` builds an approximate kNN graph, stores nodes, stores a distance matrix, supports graph nearest-neighbor search, insert, erase, and graph distance lookup.
- `Graph<WeightType, isDense, isSymmetric>` provides topology storage through Blaze matrices.

The problem is not missing capability. The problem is ownership and vocabulary. Each structure currently owns records and combines several roles:

- record storage
- metric access
- distance cache
- topology/index materialization
- query behavior
- update policy
- diagnostics

The representation layer should separate those roles without forcing a rewrite of the existing implementation.

## Target Model

`MetricSpace` is the canonical owner of records, stable IDs, metric, and version state.

```cpp
template <class Record, class Metric>
class MetricSpace {
public:
    using record_type = Record;
    using metric_type = Metric;
    using id_type = RecordId;
    using distance_type = metric_result_t<Metric, Record>;

    std::size_t size() const;
    std::uint64_t version() const;

    id_type id_at(std::size_t position) const;
    std::size_t position_of(id_type id) const;
    const Record& record(id_type id) const;
    const Metric& metric() const;

    distance_type distance(id_type a, id_type b) const;
    distance_type distance_to_record(id_type a, const Record& query) const;

    id_type insert(Record record);
    std::vector<id_type> insert_many(std::span<const Record> records);
    bool erase(id_type id);
    void replace(id_type id, Record record);
};
```

A representation attaches to a `MetricSpace` snapshot:

```cpp
template <class Space>
class Representation {
public:
    using space_type = Space;
    using record_type = typename Space::record_type;
    using distance_type = typename Space::distance_type;

    const Space& space() const;
    std::uint64_t built_for_version() const;
    bool stale() const;
};
```

The representation may be:

- implicit, computing distances on demand
- lazy, computing and caching pieces as requested
- materialized, precomputing a full structure
- exact, preserving exact metric results
- approximate, exposing quality metadata
- dynamic, supporting incremental updates
- static, requiring rebuild after mutation

## Design Principles

1. `MetricSpace` owns records. Representations hold references, handles, IDs, or caches derived from the space.

2. Stable `RecordId` values are semantic. Physical vector positions are implementation details and may change after erase or compaction.

3. Algorithms request capabilities, not concrete classes. A k-medoids implementation should need a `DistanceProvider`; a nearest-neighbor operator should need a `NeighborSearchIndex`; a graph algorithm should need a `GraphTopology`.

4. Exactness is explicit. Approximate results must be type-visible or option-visible and must carry diagnostics when possible.

5. Materialization is explicit. The runtime may choose defaults, but users can request lazy or materialized behavior.

6. Staleness is never silent. A representation built for an older space version must either reject use, refresh lazily, or report degraded validity according to its policy.

7. Existing `Matrix`, `Tree`, and `KNNGraph` stay usable. New engine types wrap or adapt them first; internal rewrites happen later only when needed.

## Core Vocabulary

### RecordId

Opaque stable ID assigned by `MetricSpace`.

```cpp
struct RecordId {
    std::uint64_t value;
};
```

### SpaceVersion

Monotonic version counter incremented on semantic mutations:

- insert
- erase
- replace
- metric replacement, if supported
- record reordering that affects physical storage but not IDs does not need to change semantic version unless representation positions are invalidated

```cpp
struct SpaceVersion {
    std::uint64_t value;
};
```

### Neighbor

Normalized nearest-neighbor result. This replaces current differences where `Matrix` returns `(id, distance)`, `Tree` returns `(Node*, distance)`, and `KNNGraph` returns IDs only.

```cpp
template <class Distance>
struct Neighbor {
    RecordId id;
    Distance distance;
};
```

### RepresentationKind

```cpp
enum class representation_kind {
    implicit_distance,
    matrix_cache,
    cover_tree_index,
    knn_graph_index,
    graph_topology,
    custom
};
```

### Exactness

```cpp
enum class exactness {
    exact,
    approximate
};
```

### Materialization

```cpp
enum class materialization {
    implicit,
    lazy,
    eager
};
```

### UpdateMode

```cpp
enum class update_mode {
    immutable_snapshot,
    invalidate_on_write,
    lazy_refresh,
    incremental
};
```

## Concepts

### MetricSpaceLike

The minimal space interface consumed by representations.

```cpp
template <class S>
concept MetricSpaceLike =
    requires(const S& space, RecordId id, std::size_t pos) {
        typename S::record_type;
        typename S::metric_type;
        typename S::distance_type;
        { space.size() } -> std::convertible_to<std::size_t>;
        { space.version() } -> std::convertible_to<std::uint64_t>;
        { space.id_at(pos) } -> std::same_as<RecordId>;
        { space.position_of(id) } -> std::convertible_to<std::size_t>;
        { space.record(id) } -> std::same_as<const typename S::record_type&>;
        { space.distance(id, id) } -> std::convertible_to<typename S::distance_type>;
    };
```

### DistanceProvider

The smallest concept for exact or cached pairwise distance access.

```cpp
template <class P>
concept DistanceProvider =
    requires(const P& provider, RecordId a, RecordId b) {
        typename P::distance_type;
        { provider.size() } -> std::convertible_to<std::size_t>;
        { provider.contains(a) } -> std::convertible_to<bool>;
        { provider.distance(a, b) } -> std::convertible_to<typename P::distance_type>;
        { provider.exactness() } -> std::same_as<exactness>;
        { provider.stale() } -> std::convertible_to<bool>;
    };
```

Expected implementations:

- `ImplicitDistanceProvider<Space>`
- `MatrixCache<Space>`
- `CoverTreeIndex<Space>` as an exact distance provider through `space.distance`, not through graph path distance
- `KnnGraphIndex<Space>` as an approximate neighbor provider and optional pairwise cache provider

Important rule: graph path distance is not the same concept as metric distance. If exposed, it must be named `graph_distance`, not `distance`.

### MatrixReadable

For algorithms that need matrix-shaped access without depending on Blaze.

```cpp
template <class M>
concept MatrixReadable =
    DistanceProvider<M> &&
    requires(const M& matrix, std::size_t i, std::size_t j) {
        { matrix.id_at(i) } -> std::same_as<RecordId>;
        { matrix(i, j) } -> std::convertible_to<typename M::distance_type>;
    };
```

### NeighborSearchIndex

For nearest-neighbor and range-search operators.

```cpp
template <class I>
concept NeighborSearchIndex =
    requires(const I& index,
             const typename I::record_type& query,
             std::size_t k,
             typename I::distance_type radius) {
        typename I::record_type;
        typename I::distance_type;
        { index.nearest(query) } -> std::same_as<Neighbor<typename I::distance_type>>;
        { index.knn(query, k) } -> std::same_as<std::vector<Neighbor<typename I::distance_type>>>;
        { index.range(query, radius) } -> std::same_as<std::vector<Neighbor<typename I::distance_type>>>;
        { index.exactness() } -> std::same_as<exactness>;
        { index.stale() } -> std::convertible_to<bool>;
    };
```

Expected implementations:

- `MatrixCache<Space>`: exact brute-force over cached distances for in-space queries; exact metric calls for external query records
- `CoverTreeIndex<Space>`: exact nearest and range search under metric assumptions
- `KnnGraphIndex<Space>`: approximate graph search
- future indices: HNSW, VP tree, ball tree, LSH, product quantization, domain-specific indices

### MutableRepresentation

For representations that can follow inserts or erases without full rebuild.

```cpp
template <class R>
concept MutableRepresentation =
    requires(R& rep,
             RecordId id,
             const typename R::record_type& record,
             std::span<const RecordId> erased) {
        { rep.on_insert(id, record) } -> std::same_as<void>;
        { rep.on_erase(erased) } -> std::same_as<void>;
        { rep.refresh() } -> std::same_as<void>;
    };
```

### GraphTopology

Topology without implying metric distance.

```cpp
template <class G>
concept GraphTopology =
    requires(const G& graph, RecordId id) {
        typename G::weight_type;
        { graph.node_count() } -> std::convertible_to<std::size_t>;
        { graph.contains(id) } -> std::convertible_to<bool>;
        { graph.neighbors(id) };
        { graph.has_edge(id, id) } -> std::convertible_to<bool>;
    };
```

Recommended neighbor shape:

```cpp
template <class Weight>
struct GraphNeighbor {
    RecordId id;
    Weight weight;
};
```

## Concrete Representation APIs

### ImplicitDistanceProvider

Purpose: zero-materialization provider that delegates to `MetricSpace::distance`.

Properties:

- exact
- implicit
- no cache memory
- always current if it references the live space
- useful for small spaces, one-shot operators, and correctness baselines

Sketch:

```cpp
template <MetricSpaceLike Space>
class ImplicitDistanceProvider {
public:
    using record_type = typename Space::record_type;
    using distance_type = typename Space::distance_type;

    explicit ImplicitDistanceProvider(const Space& space);

    std::size_t size() const;
    bool contains(RecordId id) const;
    distance_type distance(RecordId a, RecordId b) const;

    exactness exactness() const { return exactness::exact; }
    materialization materialization() const { return materialization::implicit; }
    bool stale() const { return false; }
};
```

### MatrixCache

Purpose: full or lazy pairwise distance cache over a `MetricSpace`.

Properties:

- exact
- lazy or eager
- symmetric storage when metric traits allow symmetry
- supports global algorithms that repeatedly access many pairwise distances
- can adapt existing `metric::Matrix`

Policy options:

```cpp
struct matrix_cache_options {
    materialization materialization = materialization::lazy;
    update_mode updates = update_mode::invalidate_on_write;
    bool symmetric = true;
    bool diagonal_zero = true;
    bool store_upper_triangle_only = true;
    bool parallel_build = false;
};
```

Sketch:

```cpp
template <MetricSpaceLike Space>
class MatrixCache {
public:
    using record_type = typename Space::record_type;
    using distance_type = typename Space::distance_type;

    MatrixCache(const Space& space, matrix_cache_options options = {});

    std::size_t size() const;
    RecordId id_at(std::size_t position) const;
    bool contains(RecordId id) const;

    distance_type distance(RecordId a, RecordId b) const;
    distance_type operator()(std::size_t i, std::size_t j) const;

    void materialize();
    void clear();
    void refresh();

    exactness exactness() const { return exactness::exact; }
    materialization materialization() const;
    bool stale() const;

    matrix_cache_stats stats() const;
    representation_diagnostics diagnostics() const;
};
```

Legacy adapter:

```cpp
template <class Record, class Metric>
class LegacyMatrixAdapter {
public:
    MatrixCache<MetricSpace<Record, Metric>> to_matrix_cache() const;
    const metric::Matrix<Record, Metric>& legacy() const;
};
```

Migration rule: new code should use `MatrixCache`; existing `metric::Matrix` remains a compatibility type until it can be internally backed by `MetricSpace + MatrixCache`.

### CoverTreeIndex

Purpose: exact metric-space nearest-neighbor and range-search index over a `MetricSpace`.

Properties:

- exact under required metric assumptions
- materialized index
- dynamic inserts are possible
- erases may be incremental if safe; otherwise mark stale and rebuild
- adapts existing `metric::Tree`

Policy options:

```cpp
struct cover_tree_options {
    int truncate = -1;
    update_mode updates = update_mode::incremental;
    bool validate_covering = false;
    bool rebuild_on_erase = true;
};
```

Sketch:

```cpp
template <MetricSpaceLike Space>
class CoverTreeIndex {
public:
    using record_type = typename Space::record_type;
    using distance_type = typename Space::distance_type;

    CoverTreeIndex(const Space& space, cover_tree_options options = {});

    Neighbor<distance_type> nearest(const record_type& query) const;
    std::vector<Neighbor<distance_type>> knn(const record_type& query, std::size_t k) const;
    std::vector<Neighbor<distance_type>> range(const record_type& query, distance_type radius) const;

    distance_type distance(RecordId a, RecordId b) const;
    distance_type graph_distance(RecordId a, RecordId b) const;

    void refresh();
    void on_insert(RecordId id, const record_type& record);
    void on_erase(std::span<const RecordId> ids);

    exactness exactness() const { return exactness::exact; }
    materialization materialization() const { return materialization::eager; }
    bool stale() const;

    cover_tree_stats stats() const;
    representation_diagnostics diagnostics() const;
};
```

Legacy adapter rule:

- `Tree::nn`, `Tree::knn`, and `Tree::rnn` return node pointers today.
- `CoverTreeIndex` must translate node IDs to `RecordId` and normalized `Neighbor<Distance>` results.
- `Tree::distance_by_id` and `Tree::distance` are graph-distance concepts and must not be used as metric distance providers unless explicitly named.

### KnnGraphIndex

Purpose: approximate neighbor index and graph representation over a `MetricSpace`.

Properties:

- approximate by default
- materialized graph
- exposes both neighbor search and topology
- can optionally maintain a local distance cache for edges
- adapts existing `metric::KNNGraph`

Policy options:

```cpp
struct knn_graph_options {
    std::size_t neighbors = 10;
    std::size_t max_bruteforce_size = 32;
    int max_iterations = 100;
    double update_range = 0.02;
    update_mode updates = update_mode::lazy_refresh;
    bool symmetric = true;
    bool keep_edge_distances = true;
};
```

Sketch:

```cpp
template <MetricSpaceLike Space>
class KnnGraphIndex {
public:
    using record_type = typename Space::record_type;
    using distance_type = typename Space::distance_type;
    using weight_type = distance_type;

    KnnGraphIndex(const Space& space, knn_graph_options options);

    Neighbor<distance_type> nearest(const record_type& query) const;
    std::vector<Neighbor<distance_type>> knn(const record_type& query, std::size_t k) const;
    std::vector<Neighbor<distance_type>> range(const record_type& query, distance_type radius) const;

    std::size_t node_count() const;
    bool contains(RecordId id) const;
    std::vector<GraphNeighbor<weight_type>> neighbors(RecordId id) const;
    bool has_edge(RecordId a, RecordId b) const;

    distance_type distance(RecordId a, RecordId b) const;
    std::optional<distance_type> cached_edge_distance(RecordId a, RecordId b) const;

    void refresh();
    void on_insert(RecordId id, const record_type& record);
    void on_erase(std::span<const RecordId> ids);

    exactness exactness() const { return exactness::approximate; }
    materialization materialization() const { return materialization::eager; }
    bool stale() const;

    knn_graph_stats stats() const;
    approximation_diagnostics diagnostics() const;
};
```

Important API distinction:

- `knn(query, k)` returns approximate nearest neighbors.
- `neighbors(id)` returns graph adjacency.
- `distance(a, b)` returns exact metric distance through the space unless documented as cached approximate distance.
- `cached_edge_distance(a, b)` returns only known edge distance and may be empty.

### GraphTopology

Purpose: reusable graph view over `RecordId` nodes, independent of how edges were produced.

Graph sources:

- kNN graph
- radius graph
- epsilon graph
- tree-derived parent/child topology
- grid graph
- sparsification output
- user-provided graph

Sketch:

```cpp
template <class Weight = bool>
class GraphTopologyView {
public:
    using weight_type = Weight;

    std::size_t node_count() const;
    bool contains(RecordId id) const;

    std::vector<GraphNeighbor<Weight>> neighbors(RecordId id) const;
    bool has_edge(RecordId a, RecordId b) const;
    std::optional<Weight> edge_weight(RecordId a, RecordId b) const;

    bool symmetric() const;
    bool weighted() const;
    topology_diagnostics diagnostics() const;
};
```

The existing `metric::Graph` can remain the Blaze-backed storage. The engine layer should map between `RecordId` and physical matrix row positions.

## Policies

### Materialization Policy

```cpp
struct representation_policy {
    materialization materialization = materialization::lazy;
    exactness exactness = exactness::exact;
    update_mode updates = update_mode::invalidate_on_write;
};
```

Rules:

- `implicit` means no derived storage.
- `lazy` means storage is filled on demand and must track hit/miss/invalidation stats.
- `eager` means the representation builds its required structure up front.

### Exact And Approximate Runtime

Runtime options:

```cpp
enum class runtime_mode {
    exact_only,
    prefer_exact,
    allow_approximate,
    approximate_only
};

struct query_policy {
    runtime_mode mode = runtime_mode::prefer_exact;
    std::optional<double> min_recall;
    std::optional<std::chrono::milliseconds> budget;
};
```

Dispatch examples:

- `exact_only` may use `ImplicitDistanceProvider`, `MatrixCache`, or `CoverTreeIndex`.
- `prefer_exact` uses exact representations unless cost or user options select an approximate index.
- `allow_approximate` may use `KnnGraphIndex` for neighbor search and must mark results approximate.
- `approximate_only` rejects exact-only strategies when no approximate representation is available.

Approximate result shape:

```cpp
template <class Distance>
struct NeighborSearchResult {
    std::vector<Neighbor<Distance>> neighbors;
    exactness exactness;
    std::optional<double> estimated_recall;
    representation_kind source;
};
```

## Conversion

Conversions should preserve the `MetricSpace` identity and return representations attached to the same space.

```cpp
auto provider = metric::representations::implicit(space);
auto matrix = metric::representations::matrix_cache(space);
auto tree = metric::representations::cover_tree(space);
auto graph = metric::representations::knn_graph(space, knn_graph_options{.neighbors = 16});
```

Allowed conversions:

```text
MetricSpace -> ImplicitDistanceProvider
MetricSpace -> MatrixCache
MetricSpace -> CoverTreeIndex
MetricSpace -> KnnGraphIndex
MetricSpace -> GraphTopologyView

MatrixCache -> GraphTopologyView by threshold, k, or sparsification
CoverTreeIndex -> MatrixCache by exact pairwise materialization
CoverTreeIndex -> GraphTopologyView by parent/child topology
KnnGraphIndex -> GraphTopologyView by adjacency extraction
KnnGraphIndex -> MatrixCache only for known cached distances or by explicit exact materialization
GraphTopologyView + DistanceProvider -> weighted GraphTopologyView
```

Conversion APIs should be explicit about cost:

```cpp
auto matrix = materialize_matrix(space);
auto graph = build_knn_graph(space, knn_graph_options{.neighbors = 10});
auto weighted = weight_edges(topology, provider);
auto radius_graph = threshold_graph(matrix, radius);
```

Forbidden implicit conversions:

- approximate graph distances to exact metric distances
- graph path distance to metric distance
- physical row index to `RecordId`
- stale representation to fresh representation without validation or refresh

## Diagnostics

All representations should expose a common diagnostic envelope:

```cpp
struct representation_diagnostics {
    representation_kind kind;
    exactness exactness;
    materialization materialization;
    update_mode updates;

    std::uint64_t space_version;
    std::uint64_t built_for_version;
    bool stale;

    std::size_t records;
    std::size_t distance_evaluations;
    std::size_t cached_distances;
    std::size_t memory_bytes_estimate;

    std::vector<std::string> warnings;
};
```

Representation-specific diagnostics:

```cpp
struct matrix_cache_stats {
    std::size_t hits;
    std::size_t misses;
    double fill_ratio;
    bool symmetric_storage;
};

struct cover_tree_stats {
    int min_scale;
    int max_scale;
    int truncate_level;
    std::size_t nodes;
    bool covering_validated;
    bool covering_valid;
};

struct knn_graph_stats {
    std::size_t nodes;
    std::size_t edges;
    std::size_t neighbors_requested;
    int build_iterations;
    double update_range;
};

struct approximation_diagnostics {
    representation_diagnostics common;
    std::optional<double> estimated_recall;
    std::optional<double> sampled_recall;
    std::optional<double> sampled_error;
};
```

Validation hooks:

- `MatrixCache::diagnostics` can compare sampled cached values to direct metric calls.
- `CoverTreeIndex::diagnostics` can call cover invariant checks when enabled.
- `KnnGraphIndex::diagnostics` can sample exact kNN from a matrix or implicit provider and estimate recall.
- `GraphTopologyView::diagnostics` can report symmetry, isolated nodes, degree distribution, and invalid IDs.

## Update And Insert Semantics

### MetricSpace Mutations

`MetricSpace` should be the only normal mutation entry point.

```cpp
auto id = space.insert(record);
space.erase(id);
space.replace(id, new_record);
```

Every semantic mutation increments `space.version()`.

Representations observe changes through one of three mechanisms:

1. explicit refresh by user
2. runtime-managed refresh
3. registered observer callbacks

Initial implementation should prefer explicit refresh and version checks. Observer callbacks can come later.

### Representation Behavior On Insert

`ImplicitDistanceProvider`:

- no action needed
- remains fresh because it delegates to the live space

`MatrixCache`:

- lazy policy: add unmapped row/column state; compute distances on demand
- eager policy: compute new row/column immediately
- immutable snapshot: remain usable as a snapshot but report old version

`CoverTreeIndex`:

- incremental policy: insert the new record into the tree and bind tree node ID to `RecordId`
- lazy refresh policy: mark stale and rebuild on next query if allowed
- immutable snapshot: keep old index and report stale relative to live space

`KnnGraphIndex`:

- incremental insert may connect the new node through graph search and local refinement
- lazy refresh should mark stale and rebuild before queries that require current results
- approximate result diagnostics should state whether the graph has pending updates

### Representation Behavior On Erase

Erase is harder than insert because physical storage and graph/tree topology may need repair.

`ImplicitDistanceProvider`:

- no action needed for remaining IDs
- `contains(id)` returns false for erased IDs

`MatrixCache`:

- tombstone erased ID or rebuild compact storage
- do not reinterpret old row indexes as new IDs
- expose compaction as an explicit operation

`CoverTreeIndex`:

- if legacy erase can safely remove by record or node, map `RecordId` to record and update mappings
- otherwise mark stale and require rebuild

`KnnGraphIndex`:

- erase removes node adjacency and invalidates affected neighbor lists
- default should be mark stale unless incremental repair is implemented and tested

### Replace Semantics

Replacing a record under the same `RecordId` invalidates all distances touching that ID.

Default behavior:

- `MatrixCache`: invalidate row/column for the ID
- `CoverTreeIndex`: remove and reinsert, or rebuild if removal is not safe
- `KnnGraphIndex`: mark stale or locally repair
- `GraphTopologyView`: topology may remain valid only if it is user-provided and independent of metric values; metric-derived topology is stale

## Runtime Selection

The runtime should select representations from requested intent and policy.

```cpp
auto result = metric::neighbors(
    space,
    query,
    count{10},
    query_policy{.mode = runtime_mode::allow_approximate});
```

Selection sketch:

```text
Intent: one-shot exact nearest query
  small N -> ImplicitDistanceProvider
  repeated queries -> CoverTreeIndex
  existing MatrixCache -> MatrixCache scan

Intent: global pairwise algorithm
  needs many pairwise distances -> MatrixCache
  memory budget too low -> lazy MatrixCache or block evaluator

Intent: fast approximate neighbor query
  existing KnnGraphIndex -> KnnGraphIndex
  no graph and approximate allowed -> build KnnGraphIndex

Intent: graph operator
  existing GraphTopologyView -> use it
  only MatrixCache exists -> derive threshold or kNN topology
  only KnnGraphIndex exists -> use adjacency
```

Runtime must return which representation was used:

```cpp
struct execution_trace {
    representation_kind selected_representation;
    exactness exactness;
    bool reused_existing;
    bool built_new;
    std::vector<std::string> notes;
};
```

## Legacy Mapping

| Existing type | Engine representation | First adapter behavior | Long-term direction |
|---|---|---|---|
| `metric::Matrix<RecType, Metric>` | `MatrixCache<Space>` | wrap as an owning legacy snapshot, expose normalized IDs | implement matrix storage behind `MetricSpace` |
| `metric::Tree<RecType, Metric>` | `CoverTreeIndex<Space>` | build from `space.record(id)` and maintain ID mapping | make tree store `RecordId` payloads |
| `metric::KNNGraph<Sample, Distance>` | `KnnGraphIndex<Space>` | build graph from space records and translate adjacency to IDs | separate graph topology, edge distances, and search policy |
| `metric::Graph<Weight>` | `GraphTopologyView<Weight>` | wrap Blaze graph with ID-position map | keep as storage backend |

Compatibility APIs should be additive:

```cpp
namespace metric::compat {
    template <class Space>
    metric::Matrix<typename Space::record_type, typename Space::metric_type>
    to_legacy_matrix(const Space& space);

    template <class Space>
    metric::Tree<typename Space::record_type, typename Space::metric_type>
    to_legacy_tree(const Space& space, cover_tree_options options = {});

    template <class Space>
    auto to_legacy_knn_graph(const Space& space, knn_graph_options options);
}
```

## File Layout

Add representation layer headers beside the future engine core:

```text
metric/
  representations/
    representation_traits.hpp
    distance_provider.hpp
    implicit_distance_provider.hpp
    matrix_cache.hpp
    cover_tree_index.hpp
    knn_graph_index.hpp
    graph_topology.hpp
    conversion.hpp
    diagnostics.hpp
    policy.hpp
    legacy_adapters.hpp
```

Top-level include:

```text
metric/representations.hpp
```

Optional integration include:

```text
metric/engine.hpp
```

## Implementation Phases

### Phase 1: Concepts And Result Types

Add:

- `RecordId`
- `Neighbor`
- `DistanceProvider`
- `NeighborSearchIndex`
- `GraphTopology`
- representation policy enums
- representation diagnostics structs

Acceptance criteria:

- concepts compile independently of legacy `Matrix`, `Tree`, and `KNNGraph`
- examples can type-check against a fake provider
- no existing API changes

### Phase 2: MetricSpace And Implicit Provider

Add:

- owning `MetricSpace<Record, Metric>`
- `ImplicitDistanceProvider<Space>`
- versioning and `RecordId` mapping

Acceptance criteria:

- string records with edit distance can be queried by `RecordId`
- direct metric calls are routed through the provider
- insert, erase, and replace update the version

### Phase 3: MatrixCache

Add:

- lazy/eager `MatrixCache`
- symmetric storage policy
- diagnostics and sampled validation
- adapter from and to legacy `metric::Matrix`

Acceptance criteria:

- exact distances match `space.distance`
- lazy cache reports hits and misses
- stale matrix behavior is deterministic after insert, erase, and replace
- at least one existing matrix-consuming algorithm can consume `DistanceProvider`

### Phase 4: CoverTreeIndex

Add:

- `CoverTreeIndex` wrapper over `metric::Tree`
- mapping between tree node IDs and `RecordId`
- normalized nearest, kNN, and range result types
- optional covering diagnostics

Acceptance criteria:

- exact nearest results match `MatrixCache` on sampled spaces
- tree queries return `RecordId`, not node pointers
- graph-distance helpers are named separately from metric distance
- insert behavior is covered by tests

### Phase 5: KnnGraphIndex And GraphTopology

Add:

- `KnnGraphIndex` wrapper over `metric::KNNGraph`
- `GraphTopologyView`
- adjacency extraction through `RecordId`
- approximate diagnostics

Acceptance criteria:

- graph search results are marked approximate
- topology algorithms can consume `GraphTopology`
- sampled recall can be computed against `MatrixCache`
- erase either repairs correctly or marks stale

### Phase 6: Runtime Integration

Add:

- representation registry per `MetricSpace`
- explicit runtime selection policies
- conversion helpers
- execution traces

Acceptance criteria:

- `neighbors(space, query, policy)` can switch between matrix, tree, and graph
- exact-only mode rejects approximate-only results
- stale representations are refreshed or rejected according to policy
- diagnostics show which representation was selected

## Testing Strategy

Core tests:

- stable `RecordId` survives insert and erase
- `position_of(id)` changes do not change semantic ID
- all exact providers match direct metric calls
- stale detection fires after mutation
- lazy matrix cache computes only requested distances
- eager matrix cache materializes expected number of distances
- cover tree nearest matches brute force on sampled datasets
- kNN graph reports approximate exactness and recall diagnostics
- graph topology never exposes invalid IDs

Property-style checks:

- symmetric metrics produce symmetric matrix entries
- replacing a record invalidates all affected cached distances
- erasing an ID makes `contains(id)` false everywhere after refresh
- conversion preserves space identity and version metadata

Regression checks:

- legacy `Matrix`, `Tree`, and `KNNGraph` still compile
- compatibility adapters do not change old return types
- new engine examples do not require users to construct legacy classes directly

## Documentation Requirements

Documentation should teach:

1. A metric space is the object users reason about.
2. A representation is an implementation choice.
3. Exact and approximate behavior is explicit.
4. Matrix, tree, and graph can be swapped for the same intent.
5. Graph topology is not metric distance.

Required examples:

```text
docs/engine/representations.md
examples/engine/representation_swap.cpp
python/examples/engine/representation_swap.py
```

The flagship representation example should use non-vector records first, for example strings with edit distance, to reinforce that vector space is a special case.

## Open Decisions

1. Whether `MetricSpace` should store records by value first or support external views in phase 1.
2. Whether representation registries live inside `MetricSpace` or in a separate runtime context.
3. Whether erase should preserve tombstones by default or compact storage immediately.
4. How much approximate quality metadata `KnnGraphIndex` can provide without expensive validation.
5. Whether legacy `Tree` should be changed to store `RecordId` directly or only wrapped.

## Non-Goals

- Do not remove legacy `Matrix`, `Tree`, or `KNNGraph`.
- Do not rewrite all operators before concepts and adapters exist.
- Do not make automatic runtime planning mandatory in the first implementation.
- Do not hide approximate behavior behind exact-looking APIs.
- Do not treat graph path distance as metric distance.

## Success Criteria

The representation layer is successful when the same user intent can run against different representations without changing the user model:

```cpp
auto space = metric::make_space(records, metric);

auto exact_matrix = metric::representations::matrix_cache(space);
auto exact_tree = metric::representations::cover_tree(space);
auto approx_graph = metric::representations::knn_graph(space, {.neighbors = 16});

auto a = metric::neighbors(space, query, count{5}, exact_matrix);
auto b = metric::neighbors(space, query, count{5}, exact_tree);
auto c = metric::neighbors(space, query, count{5}, approx_graph);
```

The user sees one finite metric space and three interchangeable runtime representations. The library sees precise capabilities, explicit policies, versioned caches, and diagnostics.
