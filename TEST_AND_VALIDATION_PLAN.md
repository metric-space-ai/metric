# Test And Validation Plan

## Purpose

METRIC should be validated as an engine for finite metric spaces, not as a
loose collection of algorithms. New capabilities must be testable as composed
pipelines:

```text
primitive modules -> representations -> capabilities -> Python/C++ APIs -> demos
```

This plan defines the correctness, validation, regression, performance, and CI
strategy for that engine. The goal is to make failures local, reproducible, and
meaningful: a broken metric contract should fail in primitive tests; a broken
representation should fail parity tests; a broken user workflow should fail in
API or demo tests.

User-facing APIs should use the agreed intent names: `neighbors`, `groups`,
`embed`, `map`, `transform`, `inverse_transform`, `denoise`, `outliers`, and
`compare`. Algorithm names such as cover tree, k-medoids, PHATE, PCA, or a
specific optimizer belong in the strategy layer and in test parameters.

## Scope

The validation suite covers:

- metric laws and distance contracts, as far as each declared metric type
  supports them
- distance matrix consistency across implicit and materialized spaces
- nearest-neighbor and range-query correctness
- representation parity across implicit, matrix, tree, graph, and approximate
  indexes
- clustering invariants and mapping consistency
- embedding quality, stress, continuity, and trustworthiness
- reconstruction quality for reversible mappings
- out-of-sample stability for fitted mappings and indexes
- DNN composite loss behavior for reconstruction plus geometry supervision
- Python and C++ API parity
- performance baselines and regression thresholds
- CI grouping for fast, extended, nightly, and benchmark paths

Out of scope for this document:

- choosing the final public package name
- rewriting legacy algorithms
- declaring every legacy module production-ready at once
- replacing algorithm-specific scientific validation papers with tests

## Testing Layers

### Layer 0: Primitive Module Tests

Primitive tests validate small contracts without engine orchestration.

Required coverage:

- metrics and metric traits
- `RecordId` and stable ID behavior
- pairwise distance helpers
- distance matrix construction
- graph topology primitives
- random seed handling
- serialization of fitted state
- numeric tolerance helpers

Expected properties:

- deterministic inputs produce deterministic outputs
- failures include metric name, strategy name, seed, dataset fixture, and
  tolerance
- tests do not require Python, demos, or optional heavy dependencies

### Layer 1: Representation Contract Tests

Representation tests validate that every representation exposes the same
engine-level semantics where it claims the same capability.

Representations under test:

- implicit distance provider
- full or lazy matrix cache
- cover tree index
- kNN graph index
- graph topology view
- approximate index adapters

Required contracts:

- all exact representations return the same pairwise distances as the source
  `MetricSpace`
- stale representations reject use, refresh, or report stale state according
  to their declared update policy
- graph path distance is never accepted as metric distance unless explicitly
  requested as `graph_distance`
- approximate representations report exactness and quality diagnostics

### Layer 2: Capability Tests

Capability tests validate intent-level behavior independent of the concrete
strategy.

Capabilities under test:

- `neighbors`
- range-neighbor behavior through `neighbors(radius=...)`
- `groups`
- `embed`
- `map`
- `transform`
- `inverse_transform`
- `denoise`
- `outliers`
- `compare`
- metric-space statistics such as entropy, MGC, and intrinsic dimension

Each capability test should run at least one exact strategy and one alternate
strategy when available. Strategy names belong in fixtures, parameters, and
diagnostics, not in the user-facing test API.

### Layer 3: API Parity Tests

API parity tests validate that Python and C++ expose the same conceptual model.

Required parity dimensions:

- constructor names and option names
- intent-level method names
- metric trait declarations
- stable sample or record IDs
- result schemas for neighbors, clusters, embeddings, mappings, and diagnostics
- error behavior for invalid options and invalid metric assumptions
- serialization and reload behavior where exposed

Python tests should compare against C++ golden outputs for small deterministic
fixtures. C++ tests should not depend on Python being installed.

### Layer 4: Demo And Documentation Tests

Promoted examples are executable regression tests.

Required demo families:

- custom metric over non-vector records
- finite metric space with implicit and matrix representations
- nearest-neighbor query through an intent API
- clustering through an intent API with strategy selection
- embedding plus quality diagnostics
- mapping with reconstruction
- Python and C++ versions of the same core workflow

Every promoted demo must run in CI, avoid hidden network access, and use small
fixtures with deterministic seeds.

## Fixtures

### Synthetic Metric Fixtures

Use small deterministic fixtures for exact assertions:

- one-dimensional points with absolute distance
- two-dimensional Euclidean point clouds
- Manhattan grid points
- duplicated records for pseudometric behavior
- strings with edit distance
- histograms with earth mover or Wasserstein-style distance where available
- deliberately non-metric distances for negative tests

Each fixture should declare:

- record type
- metric name
- expected metric law: metric, pseudometric, semimetric, distance, or
  similarity
- expected symmetry
- expected finite/non-negative behavior
- expected triangle behavior, if applicable
- tolerance profile

### Golden Datasets

Golden datasets should be small enough for exact exhaustive checks.

Recommended set:

- `grid_3x3`: exact neighbor and clustering expectations
- `line_with_duplicates`: pseudometric and stable-ID behavior
- `two_blobs`: clustering and embedding sanity
- `nested_circles_small`: embedding quality and local-neighborhood preservation
- `strings_small`: non-vector metric coverage
- `noisy_low_rank`: mapping reconstruction and DNN smoke coverage

Golden files should store inputs, stable IDs, expected pairwise distances when
small enough, and expected high-level outputs. For algorithms with acceptable
multiple optima, store invariant assertions instead of a single full output.

### Randomized Fixtures

Property tests should generate bounded random metric spaces.

Rules:

- every randomized test records its seed on failure
- generated sizes stay small in fast CI
- slow randomized sweeps run in extended or nightly CI
- tolerances are tied to data scale, not hard-coded blindly
- generated invalid metrics are used only in negative tests

## Metric Axiom Validation

Metric tests should validate only the laws claimed by `metric_traits`.

### True Metric

For a metric declared as a true metric:

- non-negativity: `d(a, b) >= 0`
- identity: `d(a, a) == 0`
- separation: `d(a, b) == 0` implies `a == b` for fixtures with comparable
  records
- symmetry: `d(a, b) == d(b, a)`
- triangle inequality: `d(a, c) <= d(a, b) + d(b, c)`
- finiteness: distances are finite for supported records

### Pseudometric

For a pseudometric:

- non-negativity
- identity
- symmetry if declared symmetric
- triangle inequality if declared
- no separation requirement when distinct records may have zero distance

### Semimetric Or Distance

For semimetrics or generic distances:

- validate only declared properties
- fail if user code routes them into strategies that require stronger laws
- check that diagnostics explain the mismatch

### Similarity

For similarities:

- they must not be accepted by distance-only capabilities without an explicit
  conversion policy
- converted distances must declare the conversion strategy
- tests must verify that ordering and diagonal behavior match the conversion
  contract

### Negative Tests

Negative tests should cover:

- negative distances
- NaN and infinity
- asymmetric distances routed into symmetric caches
- triangle inequality violations routed into tree strategies that require true
  metrics
- mismatched record dimensions when a metric requires equal size
- stateful metrics used in contexts that require thread safety

## Distance Matrix Consistency

Distance matrix tests validate the core finite-space materialization.

Required assertions:

- matrix dimensions equal `space.size()`
- diagonal entries match the metric identity contract
- symmetric metrics produce symmetric storage and symmetric lookup
- asymmetric metrics do not silently mirror values
- matrix lookup by physical position maps to the same records as lookup by
  stable `RecordId`
- insertion, erase, and replacement update or invalidate caches correctly
- lazy and eager materialization produce identical values
- serialization preserves distances, IDs, metric metadata, and space version

Regression cases:

- erasing a record does not shift semantic IDs
- replacing a record changes distances involving that record
- matrix cache built for version `v` is not silently used for version `v + 1`
- graph path distance is not exposed as metric distance

## Neighbor Correctness

Neighbor tests must compare every strategy against brute-force exact search on
small spaces.

Required assertions for `neighbors(count=...)`:

- returned IDs are stable `RecordId` values
- distances equal source metric distances
- results are sorted by distance and deterministic tie-break policy
- self-neighbor inclusion or exclusion follows the option contract
- `k = 0`, `k = 1`, `k = n`, and `k > n` are handled explicitly
- duplicate records are handled correctly under metric and pseudometric laws
- external query records do not mutate the source space
- exact strategies match brute force exactly within tolerance

Required assertions for `neighbors(radius=...)`:

- every returned neighbor has distance `<= radius`
- every omitted record has distance `> radius`
- radius boundary behavior is documented and tested
- negative radius fails with a clear error
- zero radius returns only records allowed by the metric law and self-option

Approximate strategies:

- report recall at `k`
- report distance inflation relative to exact results
- never claim exactness unless exact
- meet configured minimum recall on deterministic fixtures

## Representation Parity

Representation parity tests compare capability outputs across representations.

Parity matrix:

```text
Capability          Implicit  Matrix  Tree  KNN graph  Graph topology
distance            exact     exact   exact approx     topology-only
neighbors(count)    brute     exact   exact approx     not required
neighbors(radius)   brute     exact   exact optional   not required
groups              exact     exact   optional approx  topology strategies
embed               exact     exact   optional approx  topology strategies
map                 exact     exact   optional approx  optional
```

Rules:

- exact parity uses strict numeric tolerances
- approximate parity uses quality thresholds and diagnostics
- unsupported capability/representation pairs must fail at construction or
  strategy selection, not halfway through execution
- parity tests must include Python and C++ entry points for promoted APIs

## Clustering Validation

Clustering tests should favor invariant checks over brittle label snapshots.

General invariants:

- every non-noise input record is assigned to exactly one cluster
- noise labels are explicit when supported
- cluster labels may be permuted without changing correctness
- cluster medoids or exemplars are valid source `RecordId` values
- objective values do not increase across accepted optimization steps when the
  algorithm promises monotonic improvement
- deterministic seeds produce deterministic assignments
- invalid `k`, radius, or minimum-count parameters fail clearly

Distance-based clustering:

- medoid assignment distance equals the minimum distance to available medoids
- duplicate records do not create invalid empty clusters unless allowed
- `k = 1` and `k = n` are tested
- results are invariant to physical storage order when stable IDs are used,
  except where documented stochastic tie-breaking applies

Density clustering:

- core, border, and noise points match hand-built fixtures
- radius boundary behavior is tested
- graph/topology implementations match matrix implementations on small spaces

Mapping view of clustering:

- `space.map(...).transform(space)` returns a derived space or label view
  with stable source IDs
- inverse or representative reconstruction returns medoids/exemplars when the
  strategy supports it
- clustering demos use `groups` or `map`, not algorithm names as the first
  user-facing concept

## Embedding Validation

Embedding tests validate both geometry and usability.

Required diagnostics:

- raw stress or normalized stress against source distances
- local-neighborhood trustworthiness
- continuity where implemented
- nearest-neighbor overlap at one or more `k` values
- distance correlation between source distances and embedded distances
- finite coordinates and bounded numeric scale

Required assertions:

- embedding output has shape `(n_records, target_dimension)`
- stable IDs are preserved
- deterministic seeds reproduce coordinates within tolerance or reproduce
  quality diagnostics when coordinates are rotation/reflection ambiguous
- identical or zero-distance records are handled without NaN coordinates
- target dimension validation is explicit
- exact small fixtures meet minimum quality thresholds

Ambiguity handling:

- do not compare raw coordinates for methods with arbitrary rotation,
  reflection, translation, or scale unless aligned first
- prefer invariant diagnostics for regression tests
- store golden diagnostics, not full coordinate matrices, for stochastic or
  non-identifiable methods

Strategy placement:

- public tests call `embed(space, options)` or `map(...).transform`
- strategy-specific tests parameterize strategies such as PHATE, PCFA, PCA, or
  autoencoder in the strategy layer

## Mapping Reconstruction

Mapping tests validate fitted transformations between metric spaces.

Required mapping contracts:

- `fit(source_space)` stores source metric metadata and source version
- `transform(source_space)` preserves source IDs in output metadata
- `transform(new_records)` works only for mappings that declare
  out-of-sample support
- `inverse_transform(mapped_space)` declares
  whether exact reconstruction, approximate reconstruction, or representative
  reconstruction is provided
- serialization and reload produce equivalent transformations

Reconstruction assertions:

- exact reversible transforms reconstruct inputs exactly within tolerance
- approximate transforms report reconstruction error
- representative transforms return valid source records or valid decoded
  records
- reconstruction error is finite and below fixture-specific thresholds
- unsupported inverse mapping fails clearly

Mapping diagnostics:

- source distance preservation
- target distance preservation
- reconstruction error
- compression ratio where applicable
- out-of-sample error where applicable

## Out-Of-Sample Stability

Out-of-sample tests validate behavior for new records after fitting.

Required assertions:

- transforming the same new record twice is deterministic under fixed seed
- batch transform and single-record transform agree
- inserting unrelated records into a source space does not silently change a
  fitted immutable mapping
- mappings that depend on a mutable representation detect stale source versions
- neighbor relationships for held-out records are stable within configured
  tolerances
- extrapolation outside the training domain reports diagnostics when available

Recommended protocol:

1. split a deterministic fixture into fit and holdout records
2. fit using only fit records
3. transform holdout records individually and as a batch
4. compare embedded or mapped nearest-neighbor overlap against exact source
   distances
5. serialize, reload, and repeat the holdout transform

## DNN Composite Loss Validation

DNN tests should validate the native composite-loss training layer separately
from full scientific benchmarks.

Required loss components:

- reconstruction loss on decoder output
- bottleneck coordinate loss on encoder output when supervised targets exist
- optional pairwise geometry loss on bottleneck distances
- regularization terms when enabled
- total loss with explicit component weights

Required assertions:

- total loss equals the weighted sum of reported component losses
- gradients have expected shapes at decoder output and bottleneck output
- zero-weight components do not affect total loss or gradients
- sample IDs align targets after mini-batch shuffling
- fixed seeds produce reproducible first-step loss and parameter updates
- a short training run decreases total loss on a tiny deterministic fixture
- invalid target IDs fail before training begins
- serialization preserves model, codec, loss weights, and diagnostics metadata

Smoke tests:

- autoencoder reconstructs `noisy_low_rank` below a small threshold after a
  bounded number of epochs
- geometry-supervised bottleneck improves neighbor overlap or stress relative
  to reconstruction-only baseline on a small fixture
- Python wrapper and C++ native API report the same loss component names and
  shapes

Slow tests:

- longer convergence checks
- optimizer comparisons
- larger batch-size sweeps
- optional backend parity if a non-native backend is introduced

## Python/C++ Parity

Parity tests should make the public model feel like one engine.

Required paired workflows:

- construct a metric space
- compute a distance matrix
- query nearest neighbors
- run clustering through intent API
- run embedding through intent API
- map, transform, inverse_transform, serialize, and reload a mapping
- inspect diagnostics

Rules:

- Python names use clear intent names
- C++ names expose the same conceptual nouns and verbs
- strategy selection is explicit and optional in both APIs
- exceptions in Python map to typed or documented C++ errors
- output schemas are structurally equivalent
- documentation snippets are executed in both languages where possible

Golden parity format:

```text
fixture name
metric law
strategy options
C++ output JSON
Python output JSON
accepted tolerances
```

The parity harness should compare normalized JSON-like outputs rather than
language-specific object identities.

## Performance Baselines

Performance tests are regression guards, not correctness tests.

Baseline categories:

- pairwise distance materialization
- matrix cache lookup
- nearest-neighbor brute force
- nearest-neighbor tree strategy
- kNN graph build and query
- clustering runtime on small and medium fixtures
- embedding runtime on small and medium fixtures
- DNN training throughput for a tiny fixed architecture
- Python binding overhead for representative calls

Measurement rules:

- benchmarks use fixed datasets and fixed seeds
- benchmark outputs include compiler, platform, CPU, build type, dependency
  versions, and commit SHA
- benchmark assertions use broad thresholds in CI and tighter trend analysis in
  archived benchmark jobs
- correctness CI must not fail because of ordinary performance noise
- severe regressions fail only in dedicated benchmark or nightly jobs unless
  they indicate algorithmic non-termination

Recommended thresholds:

- fast CI: no hard timing assertions except timeouts
- extended CI: fail on runtime greater than 3x recorded baseline for stable
  microbenchmarks
- nightly benchmark: record trends and flag greater than 20 percent regression
  for investigation

## Regression Strategy

Every fixed bug should add the narrowest failing test at the lowest layer that
would have caught it.

Regression record fields:

- issue or pull request reference
- fixture name
- metric law
- representation
- capability
- strategy
- seed
- expected invariant

Regression tests should avoid large snapshots unless the output is naturally a
small schema. Prefer invariant checks for algorithms with non-unique valid
outputs.

## CI Grouping

### Fast CI

Runs on every pull request.

Contents:

- C++ configure and build
- primitive unit tests
- exact small fixture tests
- representation parity on tiny fixtures
- API smoke tests for C++
- Python smoke tests when Python build is enabled
- promoted minimal demos

Target runtime: under 10 minutes per platform.

### Extended CI

Runs on main branch, release candidates, and labeled pull requests.

Contents:

- full C++ test suite
- Python/C++ parity tests
- randomized property tests with bounded seed count
- embedding quality tests
- mapping reconstruction tests
- DNN smoke training tests
- serialization compatibility tests

Target runtime: under 45 minutes.

### Nightly CI

Runs on schedule.

Contents:

- larger randomized sweeps
- approximate-index quality sweeps
- stress/trustworthiness trend checks
- DNN convergence checks
- cross-platform Python wheel checks
- sanitizer builds where practical
- long-running demos and examples

### Benchmark CI

Runs on schedule or explicit trigger.

Contents:

- microbenchmarks
- medium fixture performance baselines
- memory-use tracking where available
- archived benchmark reports
- trend comparison against the latest accepted baseline

Benchmark CI should publish results but should not block ordinary development
unless an explicit performance gate is configured.

## Test Naming

Test names should encode intent first and strategy second.

Examples:

```text
neighbors_returns_bruteforce_neighbors_with_matrix_strategy
neighbors_reports_recall_with_knn_graph_strategy
embed_preserves_local_neighbors_with_phate_strategy
groups_assigns_each_record_once_with_kmedoids_strategy
map_inverse_transforms_with_autoencoder_strategy
```

Avoid public test names that imply the algorithm is the API:

```text
bad: cover_tree_knn_works
good: neighbors_returns_exact_results_with_cover_tree_strategy
```

## Acceptance Criteria

The validation plan is implemented when:

- metric law tests exist for representative true metrics, pseudometrics,
  semimetrics, distances, and similarities
- exact distance matrix tests cover IDs, mutation, staleness, and
  serialization
- exact neighbor strategies match brute force on golden fixtures
- approximate neighbor strategies report recall and quality diagnostics
- representation parity tests exist for promoted capabilities
- clustering tests use invariant checks instead of fragile label snapshots
- embedding tests report stress and trustworthiness or documented alternatives
- mapping tests cover transform, reconstruction, out-of-sample behavior, and
  reload
- DNN composite-loss tests cover component accounting, target ID alignment, and
  short-run loss decrease
- Python and C++ expose matched intent-level workflows
- promoted demos run in CI
- fast, extended, nightly, and benchmark CI groups are defined and documented

## Implementation Order

1. Add shared fixtures and numeric tolerance helpers.
2. Add metric law tests and negative metric routing tests.
3. Add distance matrix and stable-ID consistency tests.
4. Add brute-force neighbor oracle and exact strategy parity tests.
5. Add representation parity harness for promoted capabilities.
6. Add clustering invariant tests.
7. Add embedding diagnostic tests.
8. Add mapping reconstruction and out-of-sample tests.
9. Add DNN composite-loss tests.
10. Add Python/C++ parity harness.
11. Promote executable demos into CI.
12. Add benchmark baselines and reporting.
