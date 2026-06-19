# Algorithmic Gap Analysis

## Purpose

This document separates algorithm availability from engine readiness for METRIC
as a finite metric-space engine:

```text
RecordSet + Metric -> MetricSpace -> Intent -> Strategy -> Representation -> Runtime
```

Algorithm names belong in the Strategy layer. End users should ask for
capabilities such as nearest-neighbor search, clustering, embedding,
correlation, entropy, denoising, or reconstruction. The engine should then
select or validate a strategy and a representation.

## Evidence Base

This analysis is based on the current repository layout and public surfaces:

- `README.md`, `docs/stability.md`, `docs/api/*.md`, and `docs/concepts/*.md`
- `metric/space`, `metric/mapping`, `metric/transform`, `metric/correlation`
- `metric/utils/dnn`
- `metric/space.hpp`, `metric/mapping.hpp`, `metric/transform.hpp`,
  `metric/correlation.hpp`
- `python/README.md`, `python/src/CMakeLists.txt`, `python/src/*`, and
  `python/pkg/metric/*`
- `tests/core_smoke`, `tests/*_tests`, `python/tests`, and promoted examples

The repository already contains planning documents that mention `MetricSpace`,
`RecordId`, `DistanceProvider`, representations, operators, and result objects.
Those names are not yet implemented as core source primitives in the inspected
engine modules.

## Classification

- Already implemented algorithm: usable C++ algorithm or representation exists.
- Missing engine integration: implementation exists but is not reachable through
  the target engine chain.
- Missing primitive: a foundational type, concept, or contract is absent.
- Missing Python exposure: C++ capability is not available through the default
  revived Python package or has no stable Python facade.
- Missing diagnostics/tests: behavior is not covered by promoted CI, smoke
  tests, representative examples, or structured diagnostics.

## Current Inventory

| Area | Already implemented algorithm evidence | Main gap |
| --- | --- | --- |
| Space representations | `metric::Matrix`, `metric::Tree`, and `metric::KNNGraph` exist under `metric/space`; `metric/space.hpp` includes all three. | No owning `MetricSpace` facade, no shared `RecordId`, no common neighbor/result type. |
| Matrix space | `metric/space/matrix.hpp` stores records and distances, supports `insert`, `erase`, `set`, `operator()`, `nn`, `knn`, `rnn`, and `check_matrix`. | IDs are raw `size_t` storage positions; `erase` compacts storage. |
| Tree space | `metric/space/tree.hpp` implements a cover-tree style search structure with `insert`, `erase`, `nn`, `knn`, `rnn`, clustering helpers, serialization hooks, and covering checks. | Public search returns `Node*`-based results, not representation-neutral record IDs. |
| KNN graph | `metric/space/knn_graph.hpp` implements approximate kNN graph construction and graph search. | Uses raw indexes and rebuilds internal structure on mutation; no Python convenience facade. |
| Quantized mapping | `kmedoids`, `dbscan`, and `AffProp` consume `metric::Matrix`; `kmeans` consumes vector data directly. | Algorithms are tied to concrete inputs and tuple outputs, not `DistanceProvider` and typed result objects. |
| Continuous mapping and embedding | `SOM`, `PCFA`, `ESN`, `KOC`, `DSPCC`, `Redif`, and `Autoencoder` exist under `metric/mapping`. | They are direct algorithm classes, not Strategy-layer operators returning derived spaces or typed mapping results. |
| Transform | Wavelet/DWT and DCT are included by `metric/transform.hpp`; `EnergyEncoder`, HOG, and hysteresis fitting also exist under `metric/transform`. | Transform primitives are not unified as representation-producing or space-producing operators. |
| Correlation and diagnostics | `MGC`, `MGC_direct`, `Entropy`, `EntropySimple`, `VMixing`, and `epmgp` exist. Core smoke tests cover MGC and entropy regression cases. | `metric/correlation.hpp` includes MGC only; scalar returns drop diagnostic metadata such as scale, sampling, and local-correlation evidence. |
| DNN utilities | `metric/utils/dnn` has network, layers, callbacks, optimizers, and output losses. `Network::constructFromJsonString` restores `RegressionMSE`. | No composite loss primitive; DNN tests are not part of the current full test target because `tests/CMakeLists.txt` comments out `dnn_tests`. |
| Python | Default Python CMake builds `distance`, `transform`, and `space`; full `utils`, `mapping`, and `correlation` are behind `METRIC_PYTHON_BUILD_FULL`. | Python package imports and docs still reflect broader historical modules; default facade does not yet mirror the engine concepts. |

## Prioritized Gaps

### P0: MetricSpace Facade

Status: missing primitive and missing engine integration.

Evidence:

- Stable docs and promoted examples construct `metric::Matrix` directly.
- `metric/space.hpp` is an include aggregator for `Matrix`, `Tree`, and
  `KNNGraph`, not an owning facade.
- Source search finds `MetricSpace` only in planning and documentation files,
  not as a core engine type.

What is implemented:

- Explicit representations exist and are useful: matrix, tree, and kNN graph.
- Core smoke tests already prove that matrix, tree, and graph can be built from
  the same records and metric.

What is missing:

- `MetricSpace<Record, Metric>` or equivalent owning facade.
- Construction from `RecordSet + Metric`.
- Versioning or invalidation when records change.
- Representation creation through the facade, for example `space.matrix()`,
  `space.tree()`, `space.knn_graph(k)`, or lazy distance access.
- A user-facing capability API that starts from intent rather than a concrete
  representation.

Impact:

- Users must learn representation classes before they can express the metric
  space itself.
- Algorithms cannot reliably share caches, IDs, or diagnostics across
  representations.
- Engine examples cannot show the intended chain end to end.

Recommended first step:

- Add a minimal `MetricSpace<Record, Metric>` that owns records and a metric,
  exposes `size`, `record(id)`, `distance(a, b)`, and can build existing
  representations without changing those representations first.

### P0: RecordId

Status: missing primitive.

Evidence:

- `Matrix::insert` returns `std::size_t`; `Matrix::erase` removes a storage
  position and compacts records.
- `Tree::Node` has an internal `ID` and `Tree` has an `index_map`, but search
  returns `Node*` in `nn`, `knn`, and `rnn`.
- `KNNGraph` exposes raw indexes and erases by compacting `_nodes`.
- Existing neighbor APIs have incompatible result shapes:
  `std::size_t`, `std::pair<std::size_t, distance>`, `Node*`, and
  `std::vector<std::size_t>`.

What is implemented:

- Each representation has some local notion of identity or index.
- Tree already contains a partial ID-to-storage map internally.

What is missing:

- A shared opaque `RecordId`.
- Stable ID semantics across insertion, deletion, rebuilding, and
  representation conversion.
- A common `Neighbor<RecordId, Distance>` result.
- A way to preserve source-to-derived-space provenance.

Impact:

- Results from one representation cannot safely feed another representation or
  operator without ad hoc index assumptions.
- Mapping outputs cannot preserve lineage to original records in a stable way.

Recommended first step:

- Introduce `RecordId` and `Neighbor` in a small core header. Keep conversion
  adapters around existing raw-index APIs, then migrate operators gradually.

### P0: DistanceProvider

Status: missing primitive and missing engine integration.

Evidence:

- `kmedoids`, `dbscan`, and `AffProp` require `metric::Matrix`.
- `kmeans` works directly on `std::vector<std::vector<T>>` and a string-named
  distance measure.
- `MGC` computes its own distance matrices from input containers and metrics.
- `KNNGraph` computes and stores its own `_distance_matrix`.
- There is no source-level `DistanceProvider` concept or adapter in the engine
  modules.

What is implemented:

- Distance calculation and caching exist inside multiple algorithms and
  representations.
- Matrix can serve as a full distance cache today.

What is missing:

- A common provider interface for `distance(RecordId, RecordId)`.
- Adapters for implicit metric calls, matrix cache, tree-backed access, graph
  access, and externally supplied distance matrices.
- Capability declarations: exact lookup, approximate lookup, symmetric,
  sparse, mutable, cached, and metric-assumption metadata.

Impact:

- Algorithms are over-coupled to the representation they were originally
  written for.
- Reuse of distance caches is accidental instead of engine-managed.
- Strategy selection cannot reason about whether an algorithm needs full
  pairwise distances, neighbor access, or local graph structure.

Recommended first step:

- Define `DistanceProvider` and implement adapters for `MetricSpace` on-demand
  distances and `Matrix`. Move `kmedoids`, `dbscan`, `AffProp`, entropy, and
  MGC behind that interface before touching more historical algorithms.

### P1: Representation Unification

Status: missing engine integration and missing primitive.

Evidence:

- Matrix, Tree, and KNNGraph expose similar operations but with different
  return types and mutation behavior.
- `metric/transform.hpp` exposes only DCT and wavelet while other transform
  primitives live outside the umbrella include.
- `metric/correlation.hpp` exposes MGC but not entropy.
- Python `space` bindings include Matrix and Tree, while the package-level
  convenience facade exposes a Matrix factory.

What is implemented:

- The core representation algorithms exist.
- `tests/core_smoke/space_consistency_smoke.cpp` already compares matrix,
  tree, and graph over the same finite records.

What is missing:

- A representation vocabulary: matrix cache, tree index, neighbor graph, lazy
  provider, transform output, embedding output.
- A common query/result contract for `nn`, `knn`, `rnn`, and pairwise distance.
- Representation metadata: exact vs approximate, build parameters, metric
  version, record version, and validity.

Impact:

- Algorithms cannot request the representation properties they need.
- Users see representation classes as separate products rather than engine
  views over the same metric space.

Recommended first step:

- Add adapters that normalize existing representations to common engine
  concepts. Keep the historical classes as compatibility surfaces.

### P1: Diffusion And Embedding Operators

Status: implemented algorithms with missing engine integration.

Evidence:

- `metric/mapping/Redif.hpp` implements diffusion-based `encode` and `decode`.
- `metric/mapping/PCFA.hpp`, `SOM.hpp`, `autoencoder.hpp`, `DSPCC.hpp`, and
  `ESN.hpp` implement embedding, reconstruction, mapping, or prediction
  behavior.
- `metric/transform/energy_encoder.hpp`, wavelet, and DCT implement
  deterministic feature transforms.
- Examples use these algorithms directly rather than through a metric-space
  operator layer.

What is implemented:

- Multiple embedding, denoising, compression, and reconstruction algorithms.
- Python bindings exist for some historical mapping algorithms when the full
  Python build is enabled.

What is missing:

- Operator contracts for "embed this space", "denoise this space",
  "transform records", and "return a derived metric space".
- Strategy metadata that says which algorithm was used without making the
  algorithm name the user-facing capability.
- Output provenance from derived records back to original `RecordId`s.
- Unified result diagnostics: reconstruction error, fit status, dimensions,
  graph/laplacian parameters, and convergence metadata.

Impact:

- The repository has valuable algorithms, but they do not yet read as an
  engine for finite metric spaces.
- Capability-level demos cannot compose space construction, strategy choice,
  representation building, mapping, and runtime diagnostics.

Recommended first step:

- Wrap one narrow path first: `MetricSpace -> embed(strategy=PCFA) ->
  MappingResult -> derived MetricSpace`. Then repeat for Redif as a diffusion
  strategy.

### P1: Result Objects

Status: missing primitive and missing engine integration.

Evidence:

- `kmedoids`, `kmeans`, and `dbscan` return tuples of assignments,
  representatives or means, and counts.
- `AffProp` returns a tuple.
- MGC and entropy return scalar `double` values.
- Neighbor search returns representation-specific raw IDs, node pointers, or
  pairs.

What is implemented:

- Algorithm outputs are available and tested in direct form.

What is missing:

- `ClusteringResult`, `CorrelationResult`, `EntropyResult`, `MappingResult`,
  and `NeighborResult` objects.
- Fields for source IDs, strategy name, representation metadata, parameters,
  diagnostics, warnings, and timing.
- Stable Python objects mirroring the C++ result types.

Impact:

- Users must unpack tuples and know algorithm-specific ordering.
- Diagnostics are hard to preserve, compare, or display.
- Python exposure has to freeze awkward tuple contracts if results are not
  fixed first.

Recommended first step:

- Start with result objects for nearest neighbors and clustering because those
  touch `RecordId`, `DistanceProvider`, and representation unification.

### P1: DNN Composite Loss

Status: missing primitive, partial algorithm support, missing diagnostics/tests.

Evidence:

- `metric/utils/dnn/Output.h` models a single output loss object.
- `Network::constructFromJsonString` recognizes `RegressionMSE`.
- `metric/utils/dnn/dnn-includes.h` includes `RegressionMSE` and
  `MultiClassEntropy`; `BinaryClassEntropy` is commented out.
- `Autoencoder::train` calls `fit(trainData, trainData, ...)`, so current
  autoencoder training is reconstruction-loss centric.
- `tests/CMakeLists.txt` comments out `add_subdirectory(dnn_tests)`.

What is implemented:

- A lightweight DNN stack with layers, callbacks, optimizers, JSON
  construction, serialization, and at least MSE-style output loss.
- Autoencoder encode/decode/predict wrappers.

What is missing:

- A composite loss object that combines reconstruction loss, metric-preserving
  distance loss, regularization, and optional task loss.
- JSON schema for weighted loss terms.
- Diagnostics for individual loss components.
- CI coverage for DNN loss behavior.

Impact:

- Autoencoder and embedding strategies cannot be presented as metric-space
  preserving operators yet.
- DNN code remains historical support code instead of a strategy component in
  the engine.

Recommended first step:

- Add a non-invasive `CompositeOutput` or `CompositeLoss` abstraction in
  `metric/utils/dnn`, but only promote it after DNN tests are restored.

### P1: Demos And Tests

Status: missing diagnostics/tests and missing end-to-end capability demos.

Evidence:

- Core smoke tests cover Matrix, custom metrics, representation consistency,
  MGC, and entropy.
- Full C++ tests exist but are opt-in through `METRIC_BUILD_TESTS`.
- DNN tests are currently disabled in `tests/CMakeLists.txt`.
- Python docs state the package is in revival status and the default wheel
  exposes only the revived core surface; full mapping/correlation bindings are
  behind `METRIC_PYTHON_BUILD_FULL`.
- Examples are broad and historical, but they mostly demonstrate concrete
  algorithms rather than the target engine chain.

What is implemented:

- Useful smoke and regression tests for the stable revival core.
- Many historical examples across distance, space, mapping, transform,
  correlation, and DNN.

What is missing:

- One promoted C++ engine demo using the exact target chain.
- One promoted Python engine demo using the same concepts.
- Strategy-selection tests.
- Representation equivalence tests returning common `RecordId`/`Neighbor`
  objects.
- Result-object serialization and diagnostics tests.
- Import tests that distinguish default Python wheel behavior from full legacy
  binding behavior.

Impact:

- The intended public identity cannot be verified by CI yet.
- Regressions in facade, exposure, and result semantics would be easy to miss.

Recommended first step:

- Add doc-only and smoke-level tests for the first facade path before moving
  broad historical examples into the promoted surface.

## Algorithm Family Gap Matrix

| Family | Algorithm implemented | Engine integration | Missing primitive | Python exposure | Diagnostics/tests |
| --- | --- | --- | --- | --- | --- |
| Nearest neighbors | Matrix, Tree, KNNGraph | Partial, concrete classes only | `MetricSpace`, `RecordId`, `Neighbor` | Matrix/Tree binding exists; convenience facade is Matrix-first; KNNGraph absent from Python facade | Core consistency smoke exists; common-result tests missing |
| Clustering | kmedoids, kmeans, dbscan, AffProp, hierarchical clustering, KOC/SOM workflows | Partial, direct function/class calls | `DistanceProvider`, `ClusteringResult` | Historical full build exposes some modules; default wheel does not | Mapping tests exist, but engine-level result tests missing |
| Correlation | MGC, MGC_direct | Partial, direct operator class and internal distance matrices | `DistanceProvider`, `CorrelationResult` | Behind full Python build | C++ smoke and Python historical tests exist; diagnostic result tests missing |
| Entropy/dimensionality | Entropy, EntropySimple, VMixing | Partial, direct header use | `DistanceProvider`, `EntropyResult` | Behind full Python build | C++ smoke exists; facade and result tests missing |
| Transform | Wavelet/DWT, DCT, EnergyEncoder, HOG, hysteresis fitting | Partial, direct transforms | transform operator contract, derived-space provenance | Wavelet default binding exists; broader transform facade missing | Transform tests exist; engine composition tests missing |
| Embedding/diffusion | Redif, PCFA, SOM, DSPCC, Autoencoder, ESN | Missing as engine operators | `MappingResult`, derived `MetricSpace`, provenance | Some full-build bindings exist | Historical tests/examples exist; promoted engine demos missing |
| DNN | Network, layers, optimizers, MSE output, autoencoder | Missing as metric-space strategy component | composite loss, loss diagnostics | Autoencoder binding behind full mapping build | DNN tests disabled in current CMake test graph |

## Recommended Sequencing

1. Add `RecordId`, `Neighbor`, and a minimal `MetricSpace` facade.
2. Add `DistanceProvider` adapters for on-demand metric distances and
   `metric::Matrix`.
3. Normalize Matrix, Tree, and KNNGraph neighbor results through adapters.
4. Add `ClusteringResult` and move one matrix-based clustering strategy through
   `DistanceProvider`.
5. Add `CorrelationResult` and `EntropyResult` without changing the existing
   scalar APIs.
6. Add one embedding operator path, preferably PCFA first, because its inputs
   and outputs are easier to validate than the DNN stack.
7. Restore DNN tests before promoting composite DNN loss.
8. Promote one C++ and one Python end-to-end demo that use the engine chain.

## Non-Goals For The First Pass

- Do not rewrite historical algorithms before the facade and adapter contracts
  exist.
- Do not make algorithm names the primary user API.
- Do not promote full Python mapping/correlation imports until the default
  build behavior is explicit and tested.
- Do not require DNN revival before the finite metric-space engine path is
  demonstrable with Matrix, DistanceProvider, and one mapping/correlation
  operator.

