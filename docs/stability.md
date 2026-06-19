# API Surface

METRIC exposes four public layers: Core Revival API, Target Engine API, Compatibility API, and Extension API.

This page replaces status labels with product-level API boundaries. The question is not whether a module belongs to the project; the question is which surface a user depends on.

## Core Revival API

The Core Revival API is the currently promoted, CI-tested entry point.

- finite metric spaces
- standard metrics
- custom metric callables
- C++ metric adapter: `metric::Metric<Record, Callable>` and `metric::make_metric<Record>(callable)`
- C++ engine skeleton: `metric::MetricSpace`, `metric::RecordId`, `metric::Neighbor`, `metric::metric_traits`, `metric::MetricCallable`, `metric::MetricSpaceLike`, `metric::DistanceProvider`, `metric::NeighborSearchIndex`, `metric::GraphTopology`, and `metric::make_space`
- C++ engine representation adapters: `metric::representations::ImplicitDistanceProvider`, `MatrixCache`, `CoverTreeIndex`, `KnnGraphIndex`, and `GraphTopology`
- C++ engine runtime policy scaffolding: `metric::runtime::policy`, `exact`, `approximate`, `lazy`, `materialized`, `serial`, `parallel`, and `cache`
- C++ engine nearest operators: `metric::NeighborSet`, `metric::operators::knn`, and `metric::operators::range`
- C++ engine clustering operators: `metric::ClusteringResult`, `metric::operators::kmedoids`, and `metric::operators::dbscan`
- C++ engine diagnostic/statistical operators: `metric::StructureDescription`, `metric::EntropyResult`, `metric::CorrelationResult`, `metric::describe_structure`, `metric::operators::entropy`, and `metric::operators::mgc`
- C++ engine representative, compression, outlier, and denoise intents: `metric::RepresentativeSet`, `metric::CompressionResult`, `metric::OutlierResult`, `metric::find_representatives`, `metric::compress`, `metric::find_outliers`, `metric::denoise`, `metric::strategies::farthest_first`, and `metric::strategies::dbscan`
- C++ engine intent helpers, strategies, and runtime policies: `metric::find_neighbors`, semantic `metric::count`, `metric::find_groups`, `metric::find_representatives`, `metric::compress`, `metric::find_outliers`, `metric::denoise`, `metric::describe_structure`, `metric::compare`, `metric::correlate`, deterministic `metric::map`, LAPACK-backed `metric::embed`, LAPACK-backed `metric::reduce`, `metric::strategies::brute_force`, `matrix_cache`, `cover_tree`, `knn_graph`, `k_medoids`, `dbscan`, `farthest_first`, `mgc`, `pcfa`, `metric::runtime::exact`, and `metric::runtime::materialized`
- C++ engine mapping model conventions: `metric::MappingResult`, `metric::Mapping`, `metric::MappingModel`, deterministic `metric::map`, DBSCAN-noise-filtered `metric::denoise`, PCFA-backed `metric::embed`, clustered-space mapping helpers under `metric::mappings`, and LAPACK-backed PCFA mapping through `<metric/mappings/pcfa.hpp>`
- C++ facade: `metric::Space::from_records` returning `metric::FiniteSpace`
- C++ helpers: `metric::operators::pairwise_distance_matrix`, `nearest_neighbors`, `range_neighbors`, `GraphConnectivityDiagnostics`, `graph_connectivity_diagnostics`, `GraphDegreeDiagnostics`, `graph_degree_diagnostics`, `GraphStretchDiagnostics`, `graph_stretch_diagnostics`, `GraphConstructionResult`, `GraphConstructionMetadata`, `exact_knn_graph`, `exact_knn_graph_edges`, `exact_radius_graph`, `exact_radius_graph_edges`, `symmetrize_graph`, `prune_graph_out_degree`, `representative_indices`, `representatives`, `medoid_index`, `medoid`, `separated_representative_indices`, `separated_representatives`, `coverage_representative_indices`, `coverage_representatives`, and `intrinsic_dimension`
- explicit C++ representations: `metric::MatrixSpace`, `metric::GraphSpace`, `metric::TreeSpace`
- Python helpers: `metric.Space`, `metric.Metric`, `metric.RuntimePolicy`, `metric.CachePolicy`, `metric.metrics`, `metric.exceptions`, `metric.MissingMetricError`, `metric.StaleRepresentationError`, `metric.UnsupportedOperationError`, `metric.spaces.FiniteMetricSpace`, `metric.intent`, `metric.representations`, `metric.representations.TreeIndex`, `metric.representations.GraphIndex`, `metric.runtime`, `metric.operators`, `metric.EmbeddingResult`, `metric.MappingResult`, `metric.OutlierResult`, `metric.ReductionResult`, `metric.CompressionResult`, `metric.strategies.ClassicMDS`, `metric.strategies.KMedoids`, `metric.strategies.DBSCAN`, `metric.strategies.FarthestFirst`, and `metric.strategies.DistanceProfileCorrelation`
- Python beta bridges: `metric.mappings`, `metric.transforms`
- nearest-neighbor, range-neighbor, pairwise-distance, exact graph-result, graph-connectivity-diagnostics, graph-degree-diagnostics, graph-stretch-diagnostics, graph-symmetrization, graph-out-degree-pruning, exact graph-edge, intrinsic-dimension, grouping, embedding, outlier-detection, DBSCAN-noise filtering, representative-selection, representative-reduction, representative-compression, deterministic mapping, cross-space comparison, explicit missing-metric errors, Python `RuntimePolicy` / `CachePolicy`, and Python `Space.from_dataframe(...)` / `Space.record(...)` / `Space.pairwise(...)` / `Space.to_matrix()` / `Space.to_tree()` / `Space.to_graph(count=...)` / queryless `Space.neighbors(count=...)` / `Space.neighbors(count=..., radius=...)` / `Space.neighbors(radius=...)` / `Space.groups(count=...)` / `Space.groups(radius=...)` / `Space.groups(strategy=...)` / `Space.embed()` / `Space.outliers(count=...)` / `Space.outliers(threshold=...)` / `Space.outliers(strategy=...)` / `Space.denoise(count=...)` / `Space.denoise(strategy=...)` / `Space.map()` / `Space.compare()` / `Space.describe()` / `Space.representatives()` / `Space.reduce()` / `Space.compress()` helpers with representation staleness checks and `UnsupportedOperationError` for unsupported inverse reconstruction
- entropy and MGC core examples
- engine flagship examples for strings, process curves, histograms, and cross-space dependency

## Stability Tiers

Stable revival surface:

- distance metrics used by promoted examples
- typed C++ metric callable adapter under `metric::Metric`
- initial C++ engine skeleton under `metric/engine.hpp`, `metric/core/`, `metric/representations/`, and `metric/runtime/`
- initial C++ engine nearest operators under `metric/operators/nearest.hpp`
- initial C++ engine clustering operators under `metric/operators/clustering.hpp`
- initial C++ engine entropy and correlation operators under `metric/operators/entropy.hpp` and `metric/operators/correlation.hpp`
- initial C++ engine intent helpers under `metric/intent/` and strategy objects under `metric/strategies/`, including representative compression, DBSCAN-noise outlier detection, denoising, and PCFA-backed embedding
- initial C++ engine mapping conventions, clustered-space adapter, and PCFA mapping adapter under `metric/mappings/`
- minimal C++ `Space` facade for `from_records`, `neighbors`, `nearest`, and `within_radius`
- C++ operator helpers under `metric::operators`
- explicit finite-space representations: matrix, graph, and tree
- nearest-neighbor, range-neighbor, pairwise-distance, exact graph-result, graph-connectivity-diagnostics, graph-degree-diagnostics, graph-stretch-diagnostics, graph-symmetrization, graph-out-degree-pruning, exact graph-edge, intrinsic-dimension, representative-selection, and C++ representative-compression helpers
- minimal Python `Space` facade for `from_dataframe`, `record`, `pairwise`, `to_matrix`, `to_tree`, `to_graph`, `neighbors` with count/radius arguments, `nearest`, `within_radius`, `groups` with count/radius or explicit strategy, `embed`, `outliers` with count/fraction/threshold or explicit strategy, `denoise` with count/fraction/threshold or explicit strategy, `representatives`, `reduce`, `compress`, `map`, `describe`, and `compare`
- entropy and MGC regression examples
- Python core helpers under `metric.metrics`, `metric.exceptions`, `metric.spaces`, `metric.intent`, `metric.representations`, `metric.runtime`, `metric.strategies`, and `metric.operators`
- Python beta compatibility bridges under `metric.mappings` and `metric.transforms`
- CMake install/export and downstream consumer support

Beta surface:

- structured metrics with documented examples, such as TWED and EMD
- mapping modules that still need broader deterministic fixtures
- transforms used by documented workflows
- graph sparsification utilities outside the promoted `prune_graph_out_degree` operator

Experimental and historical surface:

- neural-network approximators and native DNN components
- autoencoder-related mapping code
- old industrial demos and benchmark workflows
- incomplete or disabled historical tests
- broad full-suite examples not executed by the revival CI gate

Experimental code can remain in-tree, but it must not be treated as a release blocker or first-page capability until it has clear docs, deterministic tests, and CI coverage.

## Module Status Matrix

This matrix labels the current tree by support status. It is deliberately path-based so users can tell which code is part of the revived public surface and which code is preserved for research or compatibility.

| Area | Representative paths | Status | Release meaning |
|---|---|---|---|
| Core C++ facade | `metric/concepts.hpp`, `metric/space.hpp`, `metric/operators.hpp`, `metric/metric.hpp` | Core Revival | Promoted API, covered by core CI, safe for new examples. |
| C++ engine skeleton | `metric/engine.hpp`, `metric/core/`, `metric/representations/`, `metric/runtime/`, `metric/operators/nearest.hpp`, `metric/operators/clustering.hpp`, `metric/operators/entropy.hpp`, `metric/operators/correlation.hpp`, `metric/intent/`, `metric/strategies/`, `metric/mappings/` | Core Revival | Initial engine object model for `MetricSpace`, stable record IDs, metric traits, concept traits, semantic `metric::count`, representation adapters, runtime policy scaffolding, nearest, clustering, entropy, and MGC operators, semantic `find_neighbors` / `find_groups` / `find_representatives` / `compress` / `find_outliers` / `denoise` / `describe_structure` / `compare` / `map` intent helpers with strategy objects, LAPACK-backed PCFA embed/reduce intents, and mapping result conventions with deterministic transform, DBSCAN-noise filtering, clustered-space, and PCFA derivation, covered by core CI. |
| Core C++ spaces | `metric/space/matrix.hpp`, `metric/space/knn_graph.hpp`, `metric/space/tree.hpp` | Core Revival / Expert | Stable as explicit representations; lower-level names remain compatibility APIs. |
| Promoted C++ metrics | `metric/distance/k-related/Standards.hpp`, `metric/distance/k-structured/Edit.hpp`, `metric/distance/k-structured/TWED.hpp`, `metric/distance/k-structured/EMD.hpp` | Core Revival | Covered by promoted examples or smoke tests. Broader distance modules need their own fixtures before promotion. |
| Core diagnostics | `metric/operators.hpp`, `metric/correlation/entropy.hpp`, `metric/correlation/mgc.hpp` | Core Revival | Covered by deterministic smoke tests and examples. |
| Core C++ examples and tests | `examples/core/`, `examples/engine/`, `tests/core_smoke/`, `tests/downstream_consumer/` | Core Revival | Required release gates. Every promoted example here is executed by CTest. |
| Python core facade | `python/pkg/metric/metrics.py`, `python/pkg/metric/exceptions.py`, `python/pkg/metric/spaces.py`, `python/pkg/metric/intent.py`, `python/pkg/metric/representations.py`, `python/pkg/metric/runtime.py`, `python/pkg/metric/strategies.py`, `python/pkg/metric/operators.py`, `python/pkg/metric/__init__.py`, `python/examples/engine/` | Core Revival | Promoted Python API and examples, including the `Metric` protocol, explicit missing-metric errors, deterministic graph-result, graph-connectivity-diagnostics, graph-degree-diagnostics, graph-stretch-diagnostics, graph-symmetrization, graph-out-degree-pruning, graph-edge, grouping, classical-MDS embedding, outlier-detection, DBSCAN-noise filtering, representative-selection, representative-reduction, representative-compression, deterministic mapping, unsupported inverse-reconstruction errors, deterministic representation-staleness errors, medoid, separated-representative, radius-coverage helpers, semantic `metric.intent` aliases, explicit `metric.representations` matrix/tree/graph materialization, `RuntimePolicy`, `CachePolicy`, `runtime=` on promoted Space intent methods, `Space.from_dataframe(...)`, stable `Space.ids`, `Space.record(...)`, `Space.pairwise(...)`, `Space.to_matrix()`, `Space.to_tree()`, `Space.to_graph(count=...)`, queryless `Space.neighbors(count=...)`, `Space.neighbors(count=..., radius=...)`, `Space.neighbors(radius=...)`, `Space.groups(count=...)`, `Space.groups(radius=...)`, `Space.embed()`, `Space.outliers(count=...)`, `Space.outliers(threshold=...)`, `Space.denoise(count=...)`, `Space.denoise(strategy=...)`, `Space.reduce()`, `Space.compress()`, `Space.map()`, and `Space.compare()`, covered by wheel CI and core Python tests. |
| Python beta bridges | `python/pkg/metric/mappings.py`, `python/pkg/metric/transforms.py` | Beta / Compatibility | Stable import locations that expose installed legacy names without promoting those algorithms into the core-wheel contract. |
| Python compatibility bindings | `python/pkg/metric/distance/`, `python/pkg/metric/correlation/`, `python/pkg/metric/mapping/`, `python/pkg/metric/space/`, `python/pkg/metric/transform/`, `python/pkg/metric/utils/` | Compatibility | Import-compatible surface for existing users; new examples should prefer the revived facade. |
| Mapping algorithms | `metric/mapping/`, `examples/mapping_examples/`, `tests/mapping_tests/` | Beta | Useful research code, not a default release gate until deterministic fixtures and public result contracts are added. |
| Transform and image-processing code | `metric/transform/`, `metric/utils/image_processing/`, `examples/transform_examples/`, `tests/transform_test/` | Beta / Experimental | Kept in-tree; promote only with documented assumptions and CI-backed examples. |
| Graph utilities and sparsification | `metric/utils/graph/`, `tests/sparsification_tests/` | Beta / Expert | Relevant to the engine model, but not yet part of the first-page API. |
| Native neural and autoencoder code | `metric/utils/dnn/`, autoencoder mapping files, `examples/dnn_examples/`, `tests/dnn_tests/` | Experimental | Preserved research surface; not a release blocker for the revived core. |
| Historical broad examples and tests | legacy `examples/*_examples/` outside `examples/core/`, legacy tests outside `tests/core_smoke/` | Historical / Extended | Manual restoration candidates. They should not block core install, docs, or wheel builds. |
| Bundled or third-party support code | `metric/3rdparty/`, bundled dependency install output | Support | Not a METRIC public API. |

Promotion from Beta or Experimental to Core Revival requires all of the following:

- a documented user-facing API and examples
- deterministic tests with expected values or contract assertions
- inclusion in the relevant C++ or Python CI gate
- release-note coverage for assumptions, behavior, and limitations

## Target Engine API

The Target Engine API is the public direction for normal users once the facade is implemented and tested. Target methods use understandable names:

- `neighbors`
- `groups`
- `embed`
- `map`
- `reduce`
- `compress`
- `denoise`
- `outliers`
- `compare`

Target objects include broader C++ and Python `Space` intents, strategy selection, runtime policy, and result objects with diagnostics and lineage. Documentation should label this surface as target or roadmap until those names are backed by CI examples.

## Expert API

The Expert API exposes algorithm and runtime control without changing the user-facing model.

- strategy selection
- matrix, graph, and tree representations
- exact and approximate runtime policies
- sparse and dense execution policies
- metric validation tools
- graph construction controls
- embedding and mapping internals
- native DNN components used by mapping strategies

Users reach this layer when they need performance tuning, explicit algorithm choice, reproducibility controls, or implementation-level diagnostics.

## Compatibility API

The Compatibility API keeps older METRIC names and lower-level surfaces usable.

- `metric::Matrix`
- `metric::Tree`
- `metric::KNNGraph`
- compatibility mapping and transform classes
- `metric::Manhatten` as an alias-compatible spelling
- tuple-returning low-level algorithms where existing users depend on them

New examples use the engine vocabulary. Existing code can migrate gradually.

## Extension API

The Extension API is for adding new metric-space capabilities.

Extension points include:

- custom metrics
- distance providers
- representations
- neighbor indexes
- grouping strategies
- embedding strategies
- mapping strategies
- loss functions
- neural modules
- result diagnostics

An extension integrates cleanly when it can be expressed as:

```text
Intent + Strategy + Representation + Runtime -> Result
```

## Release Contract

Every public release documents:

- Core API changes
- Expert API changes
- Compatibility API changes
- algorithmic behavior changes
- packaging changes
- known numerical or scaling limitations

The release checklist is maintained in [release-checklist.md](release-checklist.md).
