# Testing and CI Scope

This page defines which checks are release gates during the revival and which checks remain historical or extended work.

## Required Revival Gates

The required PR and release gates are intentionally small, reproducible, and cross-platform:

- `.github/workflows/core-cpp.yml`: C++ core configure, build, core tests, promoted examples, install, and downstream `find_package(metric)` consumer on Linux, macOS, and Windows.
- `.github/workflows/python-core.yml`: Python core wheel build on Linux, macOS, and Windows for supported CPython versions, followed by the promoted metric-space and engine examples and core Python API tests.
- `.github/workflows/docs-and-format.yml`: revived-file whitespace checks, revived-source formatting checks, Markdown local-link checks, and workflow YAML parsing.
- `.github/workflows/pages.yml`: static Pages artifact for `docs/site/`.
- `.github/workflows/release-artifacts.yml`: tag/manual artifact build for source archive, Python sdist, Python wheel built from that sdist, C++ core test, and downstream CMake consumer evidence.
- `.github/workflows/publish-python.yml`: manual PyPI publishing path that builds a checked sdist and cibuildwheel wheels for CPython 3.10 through 3.14 before upload.

The corresponding local commands are listed in [release-checklist.md](release-checklist.md).

## CI Grouping Model

The validation roadmap uses four CI groups. They are named explicitly so new
tests can be promoted without changing the meaning of release gates.

| Group | Current Trigger | Workflows | Blocking Status |
|---|---|---|---|
| Fast | Pull request and `master` push on relevant paths | `core-cpp.yml`, `python-core.yml`, `docs-and-format.yml` | Required revival signal for promoted C++, Python, docs, and formatting paths. |
| Extended | Manual dispatch or release-artifact rehearsal | `test-cpp-linux.yml`, `windows.yml`, `test_python.yml`, `test-coverage.yml`, `python-package.yml`, `release-artifacts.yml` | Non-blocking until each restored area has deterministic fixtures and documented expected values. |
| Nightly | Defined for future scheduled sweeps | Not active as a scheduled workflow in the revival gate | Reserved for larger randomized sweeps, long-running demos, sanitizer builds, and convergence trend checks. |
| Benchmark | Manual dispatch | `benchmark.yml` | Non-blocking performance and application-workflow evidence; ordinary correctness CI must not fail on benchmark noise. |

The native C++ `benchmark_report_smoke` gate protects the Markdown report shape
used by future benchmark artifacts. Benchmark jobs should render
`metric::benchmarks::BenchmarkReport` output as archived evidence for
run metadata, representation cost, and application-workflow comparisons, while
deterministic smoke tests continue to own pass/fail correctness.

`example_engine_benchmark_report` is the first promoted artifact generator. It
emits the checked-in [Engine Benchmark Report](examples/engine-benchmark-report.md)
shape for benchmark suite metadata, distance cost, representation choice, and
application-workflow comparisons across the promoted string, histogram, process-curve
PHATE gallery, mixed-record, distribution/image recoding, and cross-space MGC
fixtures.
The manual `benchmark.yml` workflow builds the same generator, injects
runner/build/commit metadata through `METRIC_BENCHMARK_REPORT_*`, renders
`metric-engine-benchmark-report.md`, and uploads it as a GitHub Actions artifact.

`scripts/check_metric_numeric_guard.rb` is the Metric numeric-core guard. It
fails if promoted Engine, representation, runtime, mapping, pipeline, benchmark,
strategy, CMake, workflow, or core-smoke paths add any old upstream numeric
namespace, include, package lookup, fetch, dependency, repository, or target
marker.

`scripts/check_metric_numeric_seams.rb` is the no-visible-seam guard for the
Metric numeric core. It scans source-visible files and fails on external numeric
paths, stale rewrite artifacts, relative numeric-core includes, quoted Metric
numeric includes, or old external numeric markers inside `metric/numeric`.

The promoted Metric numeric/graph primitive smoke group is labelled
`metric_numeric_core`. Run it locally with:

```bash
ctest --test-dir build/core -L metric_numeric_core --output-on-failure
```

Primitive numeric smokes should reuse
`tests/core_smoke/numeric_test_helpers.hpp` for floating-point tolerances and
matrix/vector closeness checks instead of duplicating ad hoc comparison
helpers.

The promoted PHATE pipeline, application-workflow-evidence, and benchmark-report gates are
labelled for focused local validation:

```bash
ctest --test-dir build/core -L metric_phate_pipeline --output-on-failure
ctest --test-dir build/core -L metric_application_evidence --output-on-failure
ctest --test-dir build/core -L metric_benchmark_report --output-on-failure
```

The visual public-gallery gate protects the project page from synthetic hero
fixtures and protects the accepted 60k GRAE10 visual reference:

```bash
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-public-gallery-evidence.mjs
```

Passing browser smoke checks alone does not make a hero public. A public hero
also needs native C++ evidence or a documented reference asset, as tracked in
[hero-gallery-backlog.md](hero-gallery-backlog.md).

The promoted native distance-matrix and stable-ID gate is labelled
`metric_distance_matrix_id`:

```bash
ctest --test-dir build/core -L metric_distance_matrix_id --output-on-failure
```

The promoted native DistanceTable snapshot/serialization-envelope gate is
labelled `metric_distance_matrix_snapshot`:

```bash
ctest --test-dir build/core -L metric_distance_matrix_snapshot --output-on-failure
```

The promoted native brute-force neighbor-oracle and exact strategy parity gate
is labelled `metric_neighbor_oracle`:

```bash
ctest --test-dir build/core -L metric_neighbor_oracle --output-on-failure
```

`metric_numeric_dynamic_matrix_smoke`, `engine_clustering_operators_smoke`,
`engine_intent_strategy_smoke`, and `engine_runtime_policy_smoke` protect the
first promoted `metric::numeric::DynamicMatrix` migrations for PHATE geometry and
affinity propagation.
`metric_numeric_sparse_matrix_smoke` protects the first sparse
`metric::numeric::CompressedMatrix` kernel contract for reserve/append/finalize,
non-zero accounting, element lookup, sparse matrix-vector multiplication, and
transposed sparse matrix-vector multiplication.
`metric_numeric_sparse_vector_smoke` protects the first sparse
`metric::numeric::CompressedVector` contract for reserve/append/set
construction, non-zero accounting, const zero lookup, ordered iteration,
sparse-dense dot product, sparse+dense addition, scalar scaling, row-vector dot
product, and sparse outer product behavior.
`metric_numeric_symmetric_matrix_smoke` protects the first symmetric-matrix
contract for mirrored read/write behavior, map/threshold transforms, scalar
weighting, symmetric addition, and matrix-vector multiplication used by HOG,
correlation, MGC, graph, and affinity-propagation paths.
`metric_numeric_custom_matrix_smoke` protects the first external-buffer matrix
contract for row-major and column-major aliasing, mutable row/column
write-through, conversion to `DynamicMatrix`, and matrix-vector multiplication
used by wavelet and transform paths.
`metric_numeric_diagonal_identity_smoke` protects the first diagonal and
identity matrix contract for diagonal storage, zero off-diagonal reads,
diagonal-vector multiplication, diagonal sandwich products, and identity
regularization used by entropy, EPMGP, ESN, and PCFA paths.
`metric_numeric_inverse_llh_smoke` protects the first LAPACK-adjacent dense
inverse and Cholesky contract for `inv(...)`, inverse-matrix products,
inverse-vector solve behavior, lower-triangular `llh(...)` output, and
Cholesky reconstruction used by entropy, EPMGP, ESN, Redif, and LAPACK wrapper
paths.
`metric_numeric_lifecycle_evaluate_smoke` protects native numeric lifecycle and
materialization contracts used by MGC, entropy, EPMGP, and PCFA paths:
`reset(...)`, row/column `reset(..., index)`, `clear(...)`, `shrinkToFit()`,
scalar reset, and `evaluate(...)` for dense matrix/vector expressions.
`metric_numeric_expression_adaptors_smoke` protects dense and sparse
expression-adaptor contracts used by entropy and graph/Laplacian helper paths:
dense `expand(...)` centering, column-vector expansion, dense complex
`ctrans(...)`, and sparse-matrix `ctrans(...)`.
`metric_numeric_fixed_vector_storage_smoke` protects fixed and adapted
vector-storage contracts used by image-processing, distance-example, DNN
initializer, and DNN optimizer paths: `StaticVector`, `HybridVector`, aligned
`CustomVector`, `zero(...)`, `generate(...)`, and `isDefault(...)`.
`metric_numeric_solve_eigen_smoke` protects the first direct dense solve and
eigen contract for vector and multi-RHS matrix `solve(...)` on LAPACK-sized
systems, row-vector symmetric eigenvector behavior used by PCFA, and complex
general eigenvalues used by ESN spectral-radius normalization.
`metric_numeric_serialization_smoke` protects native
`metric::numeric::Archive` round-trips for dense matrices, sparse matrices, and
parameter vectors through the primary `metric/numeric/Numeric.h` include used by
ESN and switch-detector artifact paths.
`metric_numeric_random_smoke` protects native Metric numeric random helper
contracts used by DNN example initialization and randomization helpers:
`setSeed(...)`, `getSeed()`, bounded `rand(...)` for integer, floating, and
complex values, plus bounded `randomize(...)`.
`metric_numeric_svd_rank_smoke` protects the first full dense SVD and rank
contract for singular values, `U*S*V` reconstruction, orthonormal singular
vectors, public range `svd(..., low, upp)` filtering through the portable
full-SVD fallback, and `rank(...)` on full and rank-deficient rectangular
matrices. Low-level `gesvdx` wrappers remain provider-specific and are no longer
required by the public dense SVD range overloads.
`metric_numeric_elementwise_maps_smoke` protects dense and sparse `sqrt`,
`invsqrt`, `abs`, and `pow` expression assignment for matrices and vectors,
including sparse non-zero-only map semantics used by MGC, DNN optimizer,
image-processing, and graph-helper paths.
`metric_numeric_determinant_pd_smoke` protects dense 2x2 and 3x3
determinants, inverse-determinant consistency, positive-definite true/false
and non-square behavior, non-square determinant rejection, and entropy-style
multivariate normal density arithmetic over native `inv(...)`, `det(...)`, and
Mahalanobis primitives.
`metric_numeric_diagonal_view_smoke` protects dense diagonal view read/write
behavior, vector assignment through `diagonal(...)`, const diagonal copies,
EPMGP-style `1 / diagonal(sigma)` and `mu / diagonal(sigma)` expressions, and
sparse diagonal reads.
`metric_numeric_matrix_reductions_smoke` protects dense matrix `sum`, `mean`,
`prod`, `min`, and `max` reductions across rowwise and columnwise contracts plus
rowwise `softmax(...)`, including probability normalization and deterministic
values used by MGC, PCFA, and DNN paths.
`metric_numeric_matrix_statistics_smoke` protects dense and sparse matrix
`var` and `stddev` reductions across rowwise and columnwise contracts, including
sample-variance values and invalid one-row/one-column diagnostics.
`metric_numeric_matrix_primitives_smoke` protects reusable Metric-owned matrix
helpers used by PHATE geometry construction: `row_sums(...)`,
`normalize_rows(...)`, `row_normalized(...)`, `positive_mean_or(...)`,
`column_means(...)`, column centering, max-absolute column scaling,
`columns_centered_to_unit_max_abs(...)`, `blend_with_identity(...)`,
`identity_blended(...)`, and `matrix_power(...)`, including identity exponent
behavior and invalid zero-row/non-square diagnostics.
`engine_representations_smoke` protects reusable provider/space dense
distance-matrix helpers: `provider_dense_distance_matrix(...)` for directed
provider cells, `provider_symmetric_distance_matrix(...)` for symmetric
provider consumers such as MGC, and `metric_space_dense_distance_matrix(...)`
for exact native mapping materialization.
`metric_numeric_norm_extrema_smoke` protects dense and sparse vector and matrix
`norm`, `sqrNorm`, `l1Norm`, `l2Norm`, `l3Norm`, `l4Norm`, runtime and
compile-time `lpNorm`, `linfNorm`, and global dense `min`/`max` contracts used
by Euclidean distances, solver residuals, transform bounds, image filters, and
MGC extrema paths.
`metric_numeric_functional_map_smoke` protects generic
`metric::numeric::map(...)` lambdas over dense matrices, dense vectors, sparse
matrices, and sparse vectors, plus bool-mask composition with `&&` used by HOG
and image-filter thresholding paths.
`metric_numeric_transcendental_maps_smoke` protects dense element-wise `exp`,
`log`, `tanh`, `atan`, `round`, `floor`, `ceil`, `trunc`, and `sign` contracts,
including sigmoid-style activation arithmetic, Riemannian log-spectrum norm
arithmetic, HOG angle transforms, and image-filter rounding behavior.
`metric_graph_connected_components_smoke` protects the first promoted graph
utility contract over `metric::numeric::DynamicMatrix<bool>`, including
all-components, largest-component, limited-count, and singleton-exclusion
behavior.
`metric_numeric_vector_ops_smoke` protects the first BLAS-adjacent vector
contract for row/column orientation, dot product, L2 norm, reductions,
element-wise expressions, row-vector by matrix multiplication, and transposed
matrix by column-vector multiplication.
`metric_numeric_views_smoke` protects dense view behavior used by transform,
wavelet, DNN, and solver paths: mutable `row`, `column`, `submatrix`, and
`subvector` assignments over row-major and column-major storage.
Native DNN, PHATE-AE, and runtime-policy rebuilds also act as the first
kernel-maintenance signal for warning-level `metric/numeric` fixes, with
`metric_numeric_minmax_smoke` covering the direct variadic utility contract.

Fast CI is the only group expected to run on ordinary pull requests. Extended,
nightly, and benchmark checks must remain opt-in until their fixtures are small,
deterministic, and documented enough to avoid flaky release blockers.

When a capability moves from roadmap to promoted status, it should first gain a
local deterministic fixture, then enter the extended group, then graduate into
fast CI only after runtime and platform cost are understood.

## Fast C++ Test Path

The fast C++ path is controlled by these CMake options and presets:

- `METRIC_BUILD_CORE_TESTS=ON`
- `METRIC_BUILD_CORE_EXAMPLES=ON`
- `METRIC_BUILD_TESTS=OFF`
- `METRIC_BUILD_EXAMPLES=OFF`
- `cmake --preset core`
- `cmake --preset dev`

The core tests cover:

- basic metric and space construction
- metric contract checks
- matrix, graph, and tree consistency
- MGC regression values
- entropy regression values when LAPACK is available
- intrinsic-dimension regression values
- exact graph edge fixtures for k-nearest-neighbor and radius construction
- representative-selection, medoid, separated-representative, and radius-coverage regression values
- promoted examples for strings, custom metrics, explicit representations, TWED,
  EMD, distribution/image recoding, PHATE-AE vector-row and process-curve
  fitted mapping, MGC, intrinsic dimension, representative selection, and
  entropy when LAPACK is available

The Python core wheel path covers:

- importability of the revived Python facade modules
- `Space`, `FiniteMetricSpace`, and operator helper behavior
- metric contract checks for built-in edit distance, NumPy record callables, structured record callables, time-series alignment callables, and histogram transport callables
- intrinsic-dimension regression values
- exact graph edge fixtures for k-nearest-neighbor and radius construction
- deterministic representative-selection, medoid, separated-representative, and radius-coverage regression values for histogram transport, time-series alignment, and structured mixed-record callables
- promoted Python examples for strings, structured records, time-series alignment, histogram transport, representative selection, and engine flagship workflows
- subprocess execution of every `python/examples/metric_space/*.py` and `python/examples/engine/*.py` example from the core Python API tests, so PyPI cibuildwheel tests cover the same promoted examples even when the workflow invokes only the core test suite directly
- promoted Python notebooks under `python/notebooks/`, executed by `python/notebooks/smoke_notebooks.py` from the core Python API tests without requiring Jupyter in CI

Local release rehearsals can also run:

```bash
python scripts/check_python_wheel_smoke.py
```

This builds an `mtrc` wheel from `python/`, installs it into a fresh virtual
environment, verifies that `metric` imports from `site-packages`, exercises a
promoted Python facade slice across neighbors, groups, outliers, denoise,
representatives, reduce, compress, embed, map, compare, graph diagnostics, and
runtime diagnostics, runs the promoted `python/examples/metric_space/*.py` and
`python/examples/engine/*.py` examples from that installed wheel, and checks
that vector-row PHATE-AE delegates to the native C++ binding while custom
Python metric spaces are rejected. On local Python distributions where `venv`
cannot seed `pip`, the script falls back to `uv venv --seed` when `uv` is
available.

`python-core.yml` runs this installed-wheel smoke once on Ubuntu/Python 3.12.
The full Python matrix still builds, installs, runs examples, and executes the
core Python API tests for every supported CPython version.

## Extended and Historical Tests

The full historical suite is still in-tree and can be explored with `METRIC_BUILD_TESTS=ON`, but it is not a revival release gate yet. It contains broad mapping, transform, DNN, image-processing, wrapper, and legacy utility coverage that requires additional restoration work before it can be treated as a supported signal.

The support status for those areas is maintained in the module status matrix in [stability.md](stability.md). Code moves from extended or historical coverage into the release gate only after it has deterministic fixtures, public documentation, and CI-backed examples.

The top-level [examples README](../examples/README.md) and [tests README](../tests/README.md) mark which folders are part of the revived core gate and which folders are historical or extended restoration candidates.

Known historical exclusions in the current tree:

- `tests/dnn_tests` is not added by `tests/CMakeLists.txt`.
- `tests/correlation_tests/correlation_tests.cpp` is not added by `tests/correlation_tests/CMakeLists.txt`.
- `Connected_Components_Test.cpp` is listed but disabled in `tests/correlation_tests/CMakeLists.txt`.
- `tests/transform_test/hog_tests.cpp` is listed but disabled in `tests/transform_test/CMakeLists.txt`.
- `examples/dnn_examples` is not added by `examples/CMakeLists.txt`.
- `examples/correlation_examples` is not added by `examples/CMakeLists.txt`.
- `examples/distance_examples/earth_mover_distance_2_example.cpp` is skipped by its local `CMakeLists.txt`.

Those exclusions are intentional for the revival gate. They should be restored one area at a time with deterministic fixtures and explicit expected values before they become release blockers.

## Manual Extended Workflows

The old broad workflows are retained as manual-only workflows:

- `.github/workflows/test-cpp-linux.yml`
- `.github/workflows/windows.yml`
- `.github/workflows/test_python.yml`
- `.github/workflows/test-coverage.yml`
- `.github/workflows/python-package.yml`

They are not PR gates because they encode pre-revival assumptions such as older Python versions, legacy Actions versions, full-suite builds, TestPyPI publishing, or benchmark publishing. Keeping them manual preserves history without making unstable or outdated checks block the revived core.

`.github/workflows/benchmark.yml` is also manual-only, but it is no longer only
a legacy publishing path: it now archives the native engine benchmark report
artifact while keeping timing and trend publishing non-blocking.
