# Engine Implementation Plan

## Purpose

This document turns the engine architecture into an implementation plan.

The goal is not only to add new APIs. The goal is to make users, contributors, and maintainers understand METRIC as an engine for finite metric spaces:

```text
RecordSet + Metric -> MetricSpace -> Intent -> Strategy -> Representation -> Runtime
```

This has to be visible in three places at the same time:

1. the C++ core model
2. the Python public API
3. the documentation and flagship examples

If only the code changes, the project will still look like an algorithm collection. If only the README changes, the code will not support the claim. The engine must be implemented as an architectural layer and demonstrated as the default way to use the library.

The public interface must not force users to think in algorithm names. Users should express intent with names that are understandable from the name alone. Algorithm names belong to the strategy layer.

## Implementation Principles

### 0. Building Blocks And Composed Capabilities

METRIC must support two connected levels:

1. reusable engine building blocks
2. user-friendly end-to-end capabilities for real data

The building blocks are things such as metrics, metric spaces, distance providers, matrix caches, neighbor indices, graph topologies, kernels, operators, embedding solvers, codecs, losses, neural trainers, and mapping models.

The user-facing capabilities are things such as `neighbors`, `groups`, `embed`, `map`, `reduce`, `denoise`, `outliers`, and `compare`.

An end-to-end capability is not a black-box algorithm. It is a documented composition of engine building blocks. Users should get a simple API for common workflows, while advanced users can inspect and replace the internal components.

Example:

```text
space.map(preserve = diffusion_geometry, learn_mapping = true)

MetricSpace
  -> NeighborGraph
  -> AffinityKernel
  -> DiffusionOperator
  -> DiffusionPotential
  -> MetricEmbedding
  -> NeuralMapper
  -> MappingModel
```

This is how METRIC avoids looking like a loose list of algorithms. The library exposes usable capabilities, and those capabilities are visibly assembled from metric-space primitives.

### 1. Add The Engine Beside The Existing API First

Do not rewrite existing algorithms up front. Introduce the engine as a new core layer that wraps the current implementation:

- old `metric::Matrix` remains
- old `metric::Tree` remains
- old `metric::KNNGraph` remains
- old distance functors remain
- old tuple-return functions remain

New recommended APIs should use:

- `metric::MetricSpace`
- `metric::representations::*`
- semantic intent operations such as `find_groups`, `find_neighbors`, `compare`, `reduce`, `denoise`
- `metric::strategies::*`
- `metric::mappings::*`
- `metric::runtime::*`
- composed capability recipes that bind intents to default building-block pipelines

The old API becomes compatibility surface. The new API becomes the documented engine path.

### 2. Concepts Before Refactors

The first step is to define the concepts:

- `MetricCallable`
- `MetricSpaceLike`
- `DistanceProvider`
- `NeighborSearchIndex`
- `GraphTopology`
- `Operator`
- `Mapping`

Only after concepts exist should algorithms be moved.

### 3. Intent Names Before Algorithm Names

The normal user-facing API should describe what the user wants to do:

- find neighbors
- find groups
- find representatives
- find outliers
- compare spaces
- reduce complexity
- map to a new space
- compress a space
- denoise a space
- explain structure

Concrete algorithms should be selected as strategies:

- brute force
- matrix cache
- cover tree
- kNN graph
- k-medoids
- DBSCAN
- affinity propagation
- MGC
- PCFA
- SOM/KOC
- DSPCC

This distinction is essential for the engine identity. If the primary API is `dbscan`, `kmedoids`, `mgc`, and `pcfa`, the project looks like an algorithm collection. If the primary API is `find_groups`, `find_representatives`, `compare`, and `reduce`, the project reads as an engine.

### 4. Result Objects Before More Algorithms

Do not add more tuple-return APIs. Introduce named result types:

- `Neighbor`
- `NeighborSet`
- `ClusteringResult`
- `CorrelationResult`
- `EntropyResult`
- `MappingResult`

This is essential for engine perception: named results compose; anonymous tuples look like isolated algorithms.

### 5. Representations Are Views Over A Space

The canonical object is the finite metric space. Matrix, tree, and graph are representations of the same space. They should not be presented as unrelated containers.

### 6. Runtime Is Explicit Before It Is Smart

The first runtime should not try to guess everything. Users should be able to choose:

- exact vs approximate
- lazy vs materialized
- matrix vs tree vs graph
- serial vs parallel

Automatic planning can come later.

## Target Public Shape

### C++

```cpp
#include <metric/engine.hpp>

std::vector<std::string> records = {"metric", "metrics", "matrix", "tree"};

auto space = metric::make_space(records, metric::Edit<char>{});

auto matrix = metric::representations::matrix(space);
auto tree = metric::representations::cover_tree(space);

auto neighbors = metric::find_neighbors(space, "metricks", metric::count{2});
auto groups = metric::find_groups(space, metric::strategies::k_medoids{.groups = 2});
auto structure = metric::describe_structure(space);
```

### Python

```python
from metric import Space
from metric.metrics import EditDistance
from metric.strategies import KMedoids

space = Space(["metric", "metrics", "matrix", "tree"], metric=EditDistance())

matrix = space.to_matrix()
tree = space.to_tree()

neighbors = space.neighbors("metricks", count=2)
groups = space.groups(strategy=KMedoids(groups=2))
structure = space.describe()
```

The examples must intentionally start with non-vector records. Vector examples can follow, but they should not define the mental model.

Algorithm-specific APIs may still exist for experts and compatibility, but they are not the primary teaching path.

## Intent To Strategy Map

The public API should be organized around user intent. Strategies select how that intent is executed.

| User intent | Semantic API | Strategy examples | Internal operator family |
|---|---|---|---|
| Find similar records | `neighbors`, `find_neighbors` | brute force, matrix cache, cover tree, kNN graph | nearest / range search |
| Find groups | `groups`, `find_groups` | k-medoids, DBSCAN, affinity propagation | clustering |
| Find representatives | `representatives`, `find_representatives` | medoids, graph centers, coreset strategy | representative selection |
| Find unusual records | `outliers`, `find_outliers` | density, manifold distance, KOC | anomaly / outlier scoring |
| Compare spaces | `compare`, `correlate` | MGC, distance correlation | cross-space statistics |
| Describe a space | `describe`, `describe_structure` | entropy, intrinsic dimension, connectedness | diagnostics |
| Reduce complexity | `reduce` | PCFA, SOM, DSPCC | mapping / dimensional reduction |
| Map to another space | `map` | fitted mapper, deterministic transform | mapping |
| Compress a space | `compress` | representatives, sparse graph, wavelet-assisted strategy | compression |
| Denoise a space | `denoise` | graph Laplacian, manifold denoising | denoising |

The table should appear in the documentation and guide naming in C++ and Python. Algorithm names may appear in the strategy column, but not as the first thing a normal user has to learn.

## Repository Structure To Add

```text
metric/
  engine.hpp
  core/
    concepts.hpp
    record_id.hpp
    metric_traits.hpp
    metric_space.hpp
    neighbor.hpp
    result.hpp
    version.hpp
  representations/
    implicit.hpp
    matrix_cache.hpp
    cover_tree_index.hpp
    knn_graph_index.hpp
    graph_topology.hpp
  intent/
    neighbors.hpp
    groups.hpp
    representatives.hpp
    outliers.hpp
    compare.hpp
    reduce.hpp
    describe.hpp
    denoise.hpp
  operators/
    nearest.hpp
    clustering.hpp
    correlation.hpp
    entropy.hpp
    sparsify.hpp
  strategies/
    search.hpp
    clustering.hpp
    correlation.hpp
    reduction.hpp
    representation.hpp
  mappings/
    mapping.hpp
    pcfa.hpp
    koc.hpp
  runtime/
    policy.hpp
    execution.hpp
    cache.hpp
  compat/
    aliases.hpp
    legacy_space_adapters.hpp
```

Python facade:

```text
python/pkg/metric/
  __init__.py
  core.py
  metrics/
    __init__.py
  spaces/
    __init__.py
  representations/
    __init__.py
  operators/
    __init__.py
  strategies/
    __init__.py
  mappings/
    __init__.py
  compat/
    __init__.py
```

Documentation:

```text
docs/engine/
  index.md
  mental-model.md
  metric-space.md
  representations.md
  intent-api.md
  strategies.md
  operators.md
  mappings.md
  runtime.md
  migration.md
```

Examples:

```text
examples/engine/
  strings_edit_space.cpp
  time_series_twed_space.cpp
  histogram_emd_space.cpp
  cross_space_mgc.cpp
  representation_swap.cpp

python/examples/engine/
  strings_edit_space.py
  time_series_twed_space.py
  histogram_emd_space.py
  cross_space_mgc.py
```

## Phase 1: Engine Skeleton

### Goal

Create the smallest coherent engine layer that compiles and does not break old code.

### Work

Add:

- `metric/core/record_id.hpp`
- `metric/core/neighbor.hpp`
- `metric/core/metric_traits.hpp`
- `metric/core/concepts.hpp`
- `metric/core/metric_space.hpp`
- `metric/engine.hpp`

Define:

- `RecordId`
- `Neighbor<Distance>`
- `metric_law`
- `record_kind`
- `metric_traits<T>`
- `metric_result_t<Metric, Record>`
- `MetricCallable`
- `MetricSpace<Record, Metric>`
- `make_space(records, metric)`

### Constraints

- no legacy class rewrites
- no algorithm migration yet
- no Python binding yet
- include only lightweight dependencies

### Acceptance Criteria

- existing tests still compile
- a new engine smoke test builds a `MetricSpace<std::string, Edit<char>>`
- a custom user metric works with `make_space`
- `space.distance(id_a, id_b)` works
- `space.version()` exists

### Engine Perception

This phase proves the central object exists.

## Phase 2: Representation Adapters

### Goal

Make matrix, tree, and graph feel like alternative representations of one space.

### Work

Add:

- `metric/representations/implicit.hpp`
- `metric/representations/matrix_cache.hpp`
- `metric/representations/cover_tree_index.hpp`
- `metric/representations/knn_graph_index.hpp`
- `metric/representations/graph_topology.hpp`

Introduce concepts:

- `DistanceProvider`
- `NeighborSearchIndex`
- `GraphTopology`

Representation types:

- `ImplicitDistanceProvider<Space>`
- `MatrixCache<Space>`
- `CoverTreeIndex<Space>`
- `KnnGraphIndex<Space>`

Each representation stores:

- reference or shared pointer to the space
- version at build time
- `is_stale()` check

### Acceptance Criteria

- one `MetricSpace` can create matrix and tree representations
- all representation queries return `RecordId`, not raw node pointers
- `MatrixCache::distance(id_a, id_b)` equals `space.distance(id_a, id_b)`
- `CoverTreeIndex::knn(query, k)` returns `std::vector<Neighbor<Distance>>`
- stale representation detection works after a space update

### Engine Perception

This phase proves that representations are interchangeable execution structures, not separate libraries.

## Phase 3: First Operators

### Goal

Move the first algorithms behind engine-style capabilities. These are not yet the primary user-facing names; they are the implementation layer used by the Intent API.

### Work

Add:

- `metric/operators/nearest.hpp`
- `metric/operators/clustering.hpp`
- `metric/operators/entropy.hpp`
- `metric/operators/correlation.hpp`

Migrate or wrap:

- nearest / kNN / range search
- k-medoids
- DBSCAN
- Affinity Propagation
- Entropy
- MGC

Introduce result objects:

- `NeighborSet`
- `ClusteringResult`
- `EntropyResult`
- `CorrelationResult`

### Acceptance Criteria

- `operators::knn(space, query, k)` works on implicit exact search
- `operators::knn(tree_index, query, k)` works through index concept
- `operators::kmedoids(matrix_cache, clusters)` works through `DistanceProvider`
- `operators::mgc(space_a, space_b)` works without exposing raw matrix setup in the public example
- old tuple-return functions still work

### Engine Perception

This phase proves algorithms are operations on spaces, not standalone functions with unrelated input formats. The APIs in this layer may still use algorithmic names because they are implementation-level operators.

## Phase 3b: Intent API And Strategy Layer

### Goal

Expose understandable user-facing operations and move algorithm names into explicit strategy objects.

### Work

Add:

- `metric/intent/neighbors.hpp`
- `metric/intent/groups.hpp`
- `metric/intent/representatives.hpp`
- `metric/intent/outliers.hpp`
- `metric/intent/compare.hpp`
- `metric/intent/reduce.hpp`
- `metric/intent/describe.hpp`
- `metric/strategies/search.hpp`
- `metric/strategies/clustering.hpp`
- `metric/strategies/correlation.hpp`
- `metric/strategies/reduction.hpp`

Define semantic operations:

- `find_neighbors(space, query, count)`
- `find_groups(space, strategy)`
- `find_representatives(space, count, strategy)`
- `find_outliers(space, strategy)`
- `compare(space_a, space_b, strategy)`
- `reduce(space, dimensions, strategy)`
- `describe_structure(space)`

Define strategies:

- `strategies::brute_force`
- `strategies::matrix_cache`
- `strategies::cover_tree`
- `strategies::knn_graph`
- `strategies::k_medoids`
- `strategies::dbscan`
- `strategies::affinity_propagation`
- `strategies::mgc`
- `strategies::pcfa`
- `strategies::som`
- `strategies::koc`
- `strategies::dspcc`

### Acceptance Criteria

- public examples use intent names, not algorithm names
- algorithms are selected through strategy objects
- every strategy maps to an operator or representation internally
- docs contain an intent-to-strategy table
- old direct algorithm APIs remain available as expert/compatibility APIs

### Engine Perception

This phase proves METRIC models user intent and executes algorithms as strategies. It is the main guardrail against looking like a scikit-learn-style algorithm catalog.

## Phase 4: Mapping Model

### Goal

Make mappings composable and space-producing.

### Work

Add:

- `metric/mappings/mapping.hpp`
- `metric/mappings/pcfa.hpp`
- `metric/mappings/koc.hpp`
- `metric/mappings/clustered_space.hpp`

Define conventions:

```text
fit(space) -> model
model.transform(space) -> derived MetricSpace
model.inverse_transform(derived_space) -> reconstructed records, if supported
```

Initial mapping adapters:

- PCFA
- KOC/SOM
- clustering result to quantized space

### Acceptance Criteria

- one mapping produces a new finite metric space
- mapping result stores relation to original `RecordId`s
- inverse/reconstruction is explicit and optional
- old mapping APIs remain available

### Engine Perception

This phase proves METRIC can compute new metric spaces from existing metric spaces.

## Phase 5: Python Facade

### Goal

Expose the engine, not pybind implementation details.

### Work

Add public Python modules:

- `metric.Space`
- `metric.Metric` protocol
- `metric.metrics`
- `metric.representations`
- semantic methods on `Space`, such as `neighbors`, `groups`, `compare`, `reduce`, `describe`
- `metric.intent`
- `metric.strategies`
- `metric.operators` as expert/implementation layer
- `metric.mappings`

Hide:

- `_impl`
- generated template class names
- Blaze matrix types unless explicitly requested

Compatibility aliases:

- `Manhattan = Manhatten`
- `Minkowski = P_norm`
- `ThresholdedEuclidean = Euclidean_thresholded`

Fix or deprecate:

- `Tree.empty` returning size
- `Tree.insert` overloads for insert-if behavior
- `random_see`
- top-level debug exports

### Acceptance Criteria

- `from metric import Space` works
- `Space(records, metric=EditDistance()).to_matrix()` works
- Python examples use semantic intent methods first
- algorithm names appear as strategy objects, not as the primary API
- old imports emit no immediate breakage
- missing optional compiled modules fail lazily with clear errors

### Engine Perception

This phase proves the engine is the public Python model, not an implementation detail hidden behind C++ templates.

## Phase 6: Runtime Policies

### Goal

Turn representation choice into explicit execution policy.

### Work

Add:

- `metric/runtime/policy.hpp`
- `metric/runtime/execution.hpp`
- `metric/runtime/cache.hpp`

Policies:

- `exact`
- `approximate`
- `lazy`
- `materialized`
- `parallel`
- `serial`

First runtime behavior:

- exact small-space kNN uses implicit brute force
- repeated global operators can reuse `MatrixCache`
- repeated query workloads can use `CoverTreeIndex`
- graph operators request `GraphTopology`
- stateful metrics can be marked non-thread-safe

### Acceptance Criteria

- user can explicitly select exact/materialized/approximate behavior
- runtime diagnostics can explain which representation was used
- cache invalidation respects `space.version()`
- no hidden O(n^2) materialization without an explicit or documented policy

### Engine Perception

This phase proves METRIC is an execution engine, not only an API naming layer.

## Phase 7: Flagship Demonstrations

### Goal

Demonstrate why metric-space computation is broader than vector-space or embedding-only workflows.

### Demo A: Strings As A Finite Metric Space

Records:

- strings or symbolic sequences

Metric:

- edit distance

Show:

- `MetricSpace`
- `MatrixCache`
- `neighbors`
- `groups`
- strategy selection with `k_medoids`

Message:

Numerical computation does not require a vector space.

### Demo B: Process Curves

Records:

- time series / process curves

Metric:

- TWED

Show:

- graph representation
- nearest anomalous curves
- representative selection
- comparison against naive Euclidean vectors

Message:

The metric encodes domain structure.

### Demo C: Histograms Or Images

Records:

- histograms or image-derived distributions

Metric:

- EMD or SSIM

Show:

- matrix vs graph materialization
- clustering
- sparsification

Message:

Geometry of recoding costs matters.

### Demo D: Cross-Space Dependency

Records:

- two aligned modalities

Metrics:

- separate domain metrics

Intent:

- compare spaces

Show:

- relation between two metric spaces without forcing them into one feature table
- strategy selection with MGC

Message:

METRIC computes relations between spaces, not only distances inside one vector space.

### Acceptance Criteria

- demos are small enough to run in CI smoke mode
- every demo uses the engine API
- demos avoid starting with Euclidean vectors
- docs explicitly call out why vector-space-only thinking would be weaker

## Phase 8: Documentation And Engine Identity

### Goal

Make the engine framing unavoidable.

### Work

Create docs:

- "The METRIC Engine Model"
- "Finite Metric Spaces"
- "Vector Spaces As A Special Case"
- "Intent API: What Users Ask For"
- "Strategies: How METRIC Executes It"
- "Representations: Matrix, Tree, Graph, Lazy"
- "Operators"
- "Mappings"
- "Runtime Policies"
- "Migrating From Legacy API"

README must show:

1. one-line engine statement
2. the computation pipeline
3. non-vector quickstart
4. representation swap example
5. intent API example
6. strategy selection example
7. mapping creates a new space

Avoid:

- leading with machine learning
- leading with vector examples
- presenting a table of algorithms as the main value
- exposing `DBSCAN`, `KMedoids`, `MGC`, or `PCFA` as the first-level user vocabulary
- leaking experimental modules as headline features

### Acceptance Criteria

- a reader can describe METRIC without saying "scikit-learn for metrics"
- docs consistently use Metric, MetricSpace, Representation, Operator, Mapping, Runtime
- docs distinguish user intent from algorithm strategy
- every promoted code sample is tested

## Phase 9: Legacy Consolidation

### Goal

Gradually make the old code serve the engine instead of competing with it.

### Work

Move old components into one of these categories:

Stable engine-backed:

- metrics
- metric spaces
- nearest/range
- entropy/MGC
- k-medoids/DBSCAN/AffProp

Engine mapping:

- PCFA
- KOC/SOM
- selected transforms that produce spaces

Experimental:

- ESN
- Autoencoder
- Redif
- switch detectors
- ensembles/DT
- EPMG
- hysteresis fitting

Compatibility:

- old `Matrix`, `Tree`, `KNNGraph`
- old tuple-return clustering functions
- old Python import paths

### Acceptance Criteria

- stable docs show only engine-backed modules
- experimental docs are clearly marked
- compatibility APIs remain tested but are no longer the primary teaching path

## Testing Plan

### Core Tests

- `MetricSpace` construction
- custom metric callable
- stable IDs
- version increment
- distance result type detection

### Metric Contract Tests

- non-negativity
- symmetry
- identity or pseudo-metric caveat
- triangle inequality where declared
- unequal-size behavior where declared

### Representation Tests

- implicit vs matrix distance equality
- tree nearest equals exact nearest on small fixtures
- kNN graph returns valid IDs
- stale version detection

### Operator Tests

- same operator over implicit and matrix provider
- k-medoids over generic provider
- DBSCAN over generic provider
- MGC over raw records and spaces
- entropy over raw records and spaces

### Python Tests

- public import surface
- `Space` with string records
- `Space` with NumPy records
- custom Python metric
- `to_matrix`, `to_tree`, `space.neighbors`
- strategy selection through `KMedoids`, `DBSCAN`, `MGC`, or equivalent objects
- compatibility aliases

## PR Sequence

### PR 1: Engine Core Skeleton

Files:

- `metric/engine.hpp`
- `metric/core/*`
- `tests/engine_core/*`
- `docs/engine/mental-model.md`

No legacy refactors.

### PR 2: Representation Adapters

Files:

- `metric/representations/*`
- adapter tests
- string/edit representation example

### PR 3: First Operators

Files:

- `metric/operators/nearest.hpp`
- `metric/operators/clustering.hpp`
- result objects
- wrappers over k-medoids/DBSCAN/AffProp

### PR 4: Intent API And Strategies

Files:

- `metric/intent/*`
- `metric/strategies/*`
- docs intent-to-strategy table
- semantic examples using `find_groups`, `find_neighbors`, `compare`

### PR 5: Entropy And MGC As Operators

Files:

- `metric/operators/entropy.hpp`
- `metric/operators/correlation.hpp`
- engine examples for cross-space correlation

### PR 6: Mapping Interface

Files:

- `metric/mappings/mapping.hpp`
- PCFA adapter
- KOC adapter or clustering-as-space adapter

### PR 7: Python Engine Facade

Files:

- `python/pkg/metric/core.py`
- `python/pkg/metric/spaces`
- `python/pkg/metric/representations`
- `python/pkg/metric/intent`
- `python/pkg/metric/strategies`
- `python/pkg/metric/operators`
- `python/pkg/metric/mappings`

### PR 8: Runtime Policies

Files:

- `metric/runtime/*`
- diagnostics
- policy tests

### PR 9: Flagship Examples And Docs

Files:

- `examples/engine/*`
- `python/examples/engine/*`
- `docs/engine/*`
- README engine section

## Open Decisions To Resolve Before PR 1

1. Public name: `MetricSpace` or `Space`?
2. Should `RecordId` be dense and index-like, or opaque and generation-stamped?
3. Should `MetricSpace` own records by value first, or support views immediately?
4. Should `metric_traits` default to `distance` or fail until specialized?
5. Should Python arbitrary object records be supported in v1 of the facade?
6. Should `MatrixCache` store all distances eagerly or allow lazy fill?
7. Should tree/graph representations be exact/approximate by type name or by policy?

## Recommended Answers For First Version

1. Use `MetricSpace` in C++; expose `Space` in Python.
2. Use opaque `RecordId` with dense internal storage initially.
3. Own records by value initially; add views later.
4. Let `metric_traits` default conservatively to `distance/custom`.
5. Support Python lists, strings, and NumPy arrays first.
6. Make `MatrixCache` eager; add lazy matrix later.
7. Encode exact/approximate in policy, but document representation behavior.

## Definition Of Done

METRIC can be considered an engine when this is true:

- the first public example creates a `MetricSpace`
- matrix/tree/graph are shown as representations of that space
- users call semantic intents such as neighbors, groups, representatives, compare, reduce, and describe
- algorithms appear as strategies, not as the main user vocabulary
- mappings produce new spaces or explicit mapping results
- Python and C++ use the same vocabulary
- docs no longer present the library as a list of algorithms
- flagship demos prove non-vector records are first-class

The final user-facing mental model must be:

```text
I define a metric space.
Then I ask understandable questions about that space.
If needed, I choose the strategy used to answer them.
```

Not:

```text
I pick one algorithm and reshape my data until it accepts it.
```
