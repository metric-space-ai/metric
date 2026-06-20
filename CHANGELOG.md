# Changelog

## [Unreleased]

### API Changes

- Add the dedicated C++ `<metric/operators/sparsify.hpp>` include path for exact graph construction, graph diagnostics, graph symmetrization, and out-degree pruning helpers.
- Add roadmap-only C++ `<metric/mappings/koc.hpp>` with a `metric::mappings::koc(...)` factory that validates parameters and fails explicitly until the KOC engine mapping contract is promoted.

### Documentation and Examples

- Add a C++ include-smoke test and docs for the engine graph sparsification operator header.
- Document Python `KOC` and `DSPCC` strategy objects consistently as roadmap-only strategies in the API stability docs.
- Document the roadmap-only C++ KOC mapping adapter surface in the engine mapping docs.
- Add a CI-tested C++ engine flagship demo for mixed structured records with a composed domain metric and runtime diagnostics.
- Add the Python user-path docs for `Space`, intent methods, result objects, expert strategy overrides, errors, and real-data examples.

## [0.3.3] - 2026-06-20

### API Changes

- Add the first C++ engine core skeleton with `metric::MetricSpace`, `RecordId`, `Neighbor`, metric traits, engine concept traits, and `make_space`.
- Add first C++ engine representation adapters for implicit distance lookup, matrix caching, exact neighbor-index scaffolds, kNN graph adjacency, and graph topology edges over `MetricSpace`.
- Add first C++ engine nearest operators with `NeighborSet`, `operators::knn`, and `operators::range` overloads for spaces, distance providers, and neighbor indexes.
- Add first C++ engine clustering operator with `ClusteringResult` and `operators::kmedoids` overloads for spaces and distance providers.
- Add deterministic C++ engine density clustering with `operators::dbscan` overloads for spaces and distance providers.
- Add first C++ engine intent helpers and strategy objects for semantic neighbor and grouping workflows.
- Add C++ engine entropy and MGC operator wrappers with named `EntropyResult` and `CorrelationResult` objects.
- Add the first C++ engine mapping model conventions with `MappingResult` and a clustered-space mapping adapter.
- Add a C++ engine PCFA mapping adapter with explicit fit, transform, and inverse-transform support.
- Add C++ engine compare/correlate intent helpers with an explicit MGC strategy.
- Add a C++ engine reduce intent backed by the PCFA strategy.
- Add a C++ engine embed intent backed by the PCFA strategy.
- Add a C++ engine map intent for deterministic transforms into derived metric spaces.
- Expose the C++ engine reduce intent and PCFA reduction strategy through the `<metric/engine.hpp>` umbrella header.
- Add initial C++ runtime policy objects for exact lazy/materialized neighbor execution and representation-cache staleness checks.
- Add a C++ engine `describe_structure` intent with exact finite-space diagnostics and intrinsic-dimension metadata.
- Add C++ `metric::count{...}` as a semantic count parameter for `find_neighbors`.
- Add a C++ engine `find_representatives` intent backed by deterministic farthest-first selection.
- Add a C++ engine `compress` intent backed by deterministic farthest-first representative compression.
- Add a C++ engine `find_outliers` intent backed by DBSCAN noise detection with named `OutlierResult` objects.
- Add a C++ engine `denoise` intent backed by DBSCAN noise filtering with mapping-result lineage.
- Add Python `Space.to_matrix()` for an explicit finite matrix representation view.
- Add Python `Space.compare()` / `Space.correlate()` with a named `CorrelationResult` and `DistanceProfileCorrelation` strategy.
- Add Python `Space.describe()` and `Space.representatives()` intent methods with named result objects and a `FarthestFirst` strategy.
- Add Python `Space.groups()` and `find_groups` intent methods with named `ClusteringResult` objects and `KMedoids` / `DBSCAN` strategies.
- Add Python `Space.outliers()` and `find_outliers` intent methods backed by DBSCAN noise detection with named `OutlierResult` objects.
- Add Python `Space.reduce()` and `reduce_space` intent methods backed by representative and medoid reduction with named `ReductionResult` objects.
- Add Python `Space.compress()` and `compress_space` intent methods backed by representative and medoid compression with named `CompressionResult` objects.
- Add Python `Space.map()` and `map_space` intent methods for deterministic transforms with named `MappingResult` objects.
- Add Python `Space.denoise()` and `denoise_space` intent methods backed by DBSCAN noise filtering with named `MappingResult` objects.
- Add Python `Space.embed()` and `embed_space` intent methods backed by deterministic classical MDS with named `EmbeddingResult` diagnostics.
- Add Python `metric.intent` and `metric.representations` facade modules over the promoted semantic intents and matrix representation path.
- Add Python `Space.neighbors(..., count=...)`, `Space.neighbors(..., radius=...)`, and `nearest_neighbors(..., count=...)` intent-compatible neighbor arguments.
- Add Python `Space.to_tree()` / `Space.to_graph(count=...)` representation views and matching `metric.representations.tree` / `graph` helpers.
- Add Python `metric.exceptions` with `UnsupportedOperationError` for unsupported inverse reconstruction on lossy result objects.
- Add Python representation version checks with deterministic `StaleRepresentationError` failures for stale matrix, tree, and graph views.
- Add Python `metric.Metric` protocol and explicit `MissingMetricError` when constructing `Space` without a metric.
- Add Python `RecordId` defaults, `Space.vectors(...)`, engine-model `repr`, and explicit `validate`, `copy`, and `cache` constructor policies.
- Add named Python `Neighbor` / `NeighborResult` fields to promoted examples instead of teaching tuple-indexed neighbor access.
- Add structured conversion helpers across promoted Python result objects, including neighbors, clustering, outliers, representatives, structure diagnostics, correlation, embedding, mapping, reduction, compression, graph construction, and graph diagnostics.
- Add structured conversion helpers for Python runtime policy and runtime diagnostics metadata.
- Add Python `Space.compare(...)` and `Space.correlate(...)` support for raw right-hand record sets with explicit `other_metric=`.

### Algorithm Changes

- Run every promoted Python metric-space example from the core Python API tests so wheel and PyPI build gates share the same example coverage.
- Add C++ and Python `exact_knn_graph_edges` and `exact_radius_graph_edges` helpers using exhaustive pairwise distances and deterministic edge ordering.
- Add C++ and Python `exact_knn_graph` and `exact_radius_graph` result objects with construction metadata for exact graph helpers.
- Add C++ and Python `graph_connectivity_diagnostics` helpers for deterministic graph connectivity diagnostics.
- Add C++ and Python `graph_degree_diagnostics` helpers for deterministic graph degree diagnostics.
- Add C++ and Python `graph_stretch_diagnostics` helpers for deterministic graph shortest-path stretch diagnostics.
- Add C++ and Python `symmetrize_graph` helpers for deterministic graph `union` and `mutual` policies with reciprocal distance weighting.
- Add C++ and Python `prune_graph_out_degree` helpers for deterministic directed graph sparsification.
- Add Python `representative_indices` and `representatives` helpers using deterministic farthest-first traversal over finite metric spaces.
- Add C++ `metric::operators::representative_indices` and `representatives` helpers using the same deterministic farthest-first traversal.
- Add C++ and Python `medoid_index` and `medoid` helpers using deterministic minimum-total-distance selection.
- Add C++ and Python `separated_representative_indices` and `separated_representatives` helpers using deterministic redundancy-threshold selection.
- Add C++ and Python `coverage_representative_indices` and `coverage_representatives` helpers using deterministic greedy radius coverage.
- Add deterministic Python representative-selection fixtures for time-series alignment and structured mixed records.

### Documentation and Examples

- Add a promotion-gated research roadmap for diagnostics, representative selection, sparse graphs, cross-space dependency discovery, denoising, vector-database adapters, and benchmarks.
- Add a promoted Python time-series metric-space example using an alignment-aware callable on the core wheel path.
- Add a promoted Python histogram metric-space example using a one-dimensional transport callable on the core wheel path.
- Add a promoted Python representative-selection example using histogram records and transport distance on the core wheel path.
- Add a promoted C++ representative-selection example using string records and edit distance on the core CTest path.
- Extend promoted representative-selection examples with medoid representatives.
- Extend promoted representative-selection examples with separated representatives.
- Add graph representation terminology for exact, approximate, directed, symmetrized, weighted, and normalized graph construction.
- Add exact graph edge fixture documentation for k-nearest-neighbor and radius edge lists.
- Add CI-tested C++ and Python engine flagship examples for strings, process curves, histograms, and cross-space dependency.
- Add engine documentation chapters for metric spaces, representations, intents, strategies, operators, mappings, runtime policies, and migration.
- Update the README quickstart to show the C++ engine path, strategy selection, representation swap, and mapping-derived spaces.
- Update API and stability docs for the promoted C++ and Python outlier intent surface.

### Packaging and Build

- Prepare Python and C++ package metadata for `0.3.3`.
- Run PyPI publish dry-run `27862071672` on current `master` with `publish=false`; the source distribution and CPython 3.10 through 3.14 wheels for Linux, macOS, and Windows were built and checked successfully.

### Known Limitations

- PyPI publishing for `metric-space` remains blocked until repository PyPI credentials are replaced or PyPI Trusted Publishing is configured for `metric-space-ai/metric`.

## [0.3.2] - 2026-06-19

### API Changes

- Add `metric::operators::intrinsic_dimension` and `metric.intrinsic_dimension` as expansion-dimension diagnostics for finite metric spaces.

### Algorithm Changes

- Add Python core metric-contract checks for non-negativity, identity, symmetry, and triangle inequality across built-in edit distance, NumPy record callables, and structured-record callables.
- Add C++ and Python regression coverage for intrinsic-dimension estimates.

### Packaging and Build

- Add a Trusted Publishing option to the manual PyPI publishing workflow while preserving the repository-secret password path.
- Run every promoted Python metric-space example in the Python release-artifact gate.
- Include both promoted Python examples in cibuildwheel tests for PyPI publish builds.
- Add a revived-source formatting check to the docs and formatting gate.

### Documentation and Examples

- Add a promoted Python structured-record metric-space example using dictionaries and a domain metric callable.
- Add a promoted C++ intrinsic-dimension example and documentation.
- Document the promoted Python examples in the structured-data examples guide.
- Update testing and release-gate docs to describe Python core metric-contract coverage and multiple promoted Python examples.

### Known Limitations

- PyPI publishing for `metric-space` remains blocked until repository PyPI credentials are replaced or PyPI Trusted Publishing is configured for `metric-space-ai/metric`.

## [0.3.1] - 2026-06-19

### Packaging and Build

- Add a manual PyPI publishing workflow that builds a checked source distribution and cibuildwheel-generated CPython 3.10 through 3.14 wheels before uploading with repository PyPI credentials.
- Use Python's `Development.Module` CMake component for extension-module builds when available, avoiding an unnecessary embedded-Python library requirement in manylinux wheel builds.
- Run cibuildwheel smoke tests from the repository `python/` subtree when the package directory is scoped to `./python`.

## [0.3.0] - 2026-06-19

### API Changes

- Reframe the public C++ core around finite metric-space numerics.
- Add `metric::Metric<Record, Callable>`, `metric::make_metric<Record>(callable)`, `metric::is_metric_callable_v`, and `metric::metric_distance_t` in `metric/concepts.hpp`.
- Add `metric::Space::from_records` returning `metric::FiniteSpace`.
- Add explicit representation aliases `metric::MatrixSpace`, `metric::GraphSpace`, and `metric::TreeSpace`.
- Add C++ operator helpers under `metric::operators`: `pairwise_distance_matrix`, `nearest_neighbors`, and `range_neighbors`.
- Add Python core facade modules `metric.metrics`, `metric.spaces`, and `metric.operators`.
- Add top-level Python convenience imports for `Edit`, `Space`, `FiniteMetricSpace`, `MatrixSpace`, `nearest_neighbors`, `pairwise_distance_matrix`, and `range_neighbors`.
- Add Python beta compatibility bridges `metric.mappings` and `metric.transforms`.

### Algorithm Changes

- Restore deterministic smoke coverage for MGC and entropy examples in the revived core path.
- Add metric-contract smoke coverage for non-negativity, identity, symmetry, and triangle inequality on promoted metrics and a custom non-vector metric.
- Add consistency coverage across matrix, graph, and tree space representations.
- No new algorithm family is promoted from beta or experimental status in this revival draft.

### Packaging and Build

- Raise the top-level CMake minimum to 3.19.
- Normalize CMake options around `METRIC_BUILD_TESTS`, `METRIC_BUILD_EXAMPLES`, `METRIC_BUILD_BENCHMARKS`, `METRIC_BUILD_PYTHON`, and core-specific smoke/example options.
- Keep `BUILD_TESTS` as a deprecated compatibility alias.
- Add CMake presets for `core`, `dev`, `full-tests`, and `release`.
- Add a `python-cmake` preset for top-level CMake validation of the Python core bindings.
- Add optional `METRIC_FETCH_DEPS` FetchContent fallback for core dependencies.
- Add install/export support verified by a downstream `find_package(panda_metric)` consumer.
- Add `python/pyproject.toml` with PEP 621 project metadata, modern Python version metadata, and core-wheel CI for CPython 3.10 through 3.14.
- Rename the public Python distribution to `metric-space`; the import package remains `metric`.
- Add release artifact workflow for source archive, Python sdist, Python wheel built from that sdist, C++ core checks, and downstream CMake consumer evidence.

### Documentation and Examples

- Rewrite the README around finite metric-space numerics rather than generic machine learning.
- Add concept docs for metric spaces, finite metric spaces, vector spaces as a special case, explicit representations, and metrics as recoding costs.
- Add API docs for C++ and Python.
- Add example docs for custom metrics, structured data, TWED, EMD, entropy diagnostics, MGC correlation, and industrial anomaly workflow framing.
- Add promoted C++ examples under `examples/core/` and a promoted Python string/edit-space example.
- Add static GitHub Pages content under `docs/site/`.
- Add revival status, testing/CI scope, module stability, and release checklist docs.

### Compatibility Notes

- Preserve historical C++ representation names such as `metric::Matrix`, `metric::Tree`, and `metric::KNNGraph`.
- Add correctly spelled `metric::Manhattan` and `metric::Manhattan_standardized` aliases while keeping `metric::Manhatten` compatibility.
- Keep broad historical examples, mappings, transforms, native DNN code, image-processing code, and old test suites in-tree as beta, experimental, or historical surfaces.
- Convert old broad workflows to manual-only workflows while new revival workflows cover core C++, Python wheels, docs/formatting, and Pages artifacts.

### Known Limitations

- The target intent facade for `groups`, `embed`, `map`, `reduce`, `denoise`, `outliers`, and `compare` remains roadmap API until backed by stable strategies, result contracts, examples, and CI.
- Mapping and transform algorithms remain beta or compatibility surfaces; `metric.mappings` and `metric.transforms` only expose installed legacy names.
- The default Python core wheel intentionally exposes a narrow revived surface; broader legacy bindings require separate restoration.
- GitHub release `v0.3.0` is published with release artifacts; PyPI publishing for `metric-space` remains pending until package ownership or Trusted Publishing is confirmed.

## [0.2.1] - 2022-05-16

### Added
- PCFA average() and weights() getter methods and constructor that uses what they return, so as user in enabled to export and re-create the PCFA model [#324](https://github.com/panda-official/metric/pull/324)
- Minimal test for PCFA [#324](https://github.com/panda-official/metric/pull/324)
- Save and load example lines at `PCFA_example.cpp` [#324](https://github.com/panda-official/metric/pull/324)

### Changed
- Add build ability without LAPACK [#325](https://github.com/panda-official/metric/pull/325)
- Add link to cereal::cereal target [#325](https://github.com/panda-official/metric/pull/325)
- Change cereal version to 1.3.2 in CI [#325](https://github.com/panda-official/metric/pull/325)


## [0.2.0] - 2022-02-10

### Added
- Zero derivative padding with tests [#313](https://github.com/panda-official/metric/pull/313)
- Cmake `metric` target [#308](https://github.com/panda-official/metric/pull/308)
- Cmake variables for benchmarks, examples and tests [#319](https://github.com/panda-official/metric/pull/319)
- Cmake install instructions [#319](https://github.com/panda-official/metric/pull/319)
- `.clang-format`
- Changelog

### Changed
- Metric headers moved from `modules` to `metric` directory [#318](https://github.com/panda-official/metric/pull/318)
  [#319](https://github.com/panda-official/metric/pull/319)
- Move buildin dependencies (`blaze`, `json`, `cereal`) to external [#315](https://github.com/panda-official/metric/pull/315)
- Wavelet `dbwavf` refactor [#317](https://github.com/panda-official/metric/pull/317)
- Fix dozens spelling errors, layout errors, etc in `README.md` [#319](https://github.com/panda-official/metric/pull/319)
- Put in order CI workflows [#311](https://github.com/panda-official/metric/pull/311)
  [#312](https://github.com/panda-official/metric/pull/312)
- Reformat code according to `.clang-format`

### Removed
- `crossfilter`, `n-api`[#315](https://github.com/panda-official/metric/pull/315)
