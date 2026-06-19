# Changelog

## [Unreleased]

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
