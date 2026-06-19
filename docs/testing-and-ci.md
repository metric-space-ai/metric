# Testing and CI Scope

This page defines which checks are release gates during the revival and which checks remain historical or extended work.

## Required Revival Gates

The required PR and release gates are intentionally small, reproducible, and cross-platform:

- `.github/workflows/core-cpp.yml`: C++ core configure, build, core tests, promoted examples, install, and downstream `find_package(panda_metric)` consumer on Linux, macOS, and Windows.
- `.github/workflows/python-core.yml`: Python core wheel build on Linux, macOS, and Windows for supported CPython versions, followed by the promoted metric-space examples and core Python API tests.
- `.github/workflows/docs-and-format.yml`: revived-file whitespace checks, revived-source formatting checks, Markdown local-link checks, and workflow YAML parsing.
- `.github/workflows/pages.yml`: static Pages artifact for `docs/site/`.
- `.github/workflows/release-artifacts.yml`: tag/manual artifact build for source archive, Python sdist, Python wheel built from that sdist, C++ core test, and downstream CMake consumer evidence.
- `.github/workflows/publish-python.yml`: manual PyPI publishing path that builds a checked sdist and cibuildwheel wheels for CPython 3.10 through 3.14 before upload.

The corresponding local commands are listed in [release-checklist.md](release-checklist.md).

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
- representative-selection and radius-coverage regression values
- promoted examples for strings, custom metrics, explicit representations, TWED, EMD, MGC, intrinsic dimension, representative selection, and entropy when LAPACK is available

The Python core wheel path covers:

- importability of the revived Python facade modules
- `Space`, `FiniteMetricSpace`, and operator helper behavior
- metric contract checks for built-in edit distance, NumPy record callables, structured record callables, time-series alignment callables, and histogram transport callables
- intrinsic-dimension regression values
- deterministic representative-selection and radius-coverage regression values for histogram transport, time-series alignment, and structured mixed-record callables
- promoted Python examples for strings, structured records, time-series alignment, histogram transport, and representative selection
- subprocess execution of every `python/examples/metric_space/*.py` example from the core Python API tests, so PyPI cibuildwheel tests cover the same promoted examples even when the workflow invokes only the core test suite directly

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

## Legacy Workflows

The old broad workflows are retained as manual-only workflows:

- `.github/workflows/test-cpp-linux.yml`
- `.github/workflows/windows.yml`
- `.github/workflows/test_python.yml`
- `.github/workflows/test-coverage.yml`
- `.github/workflows/python-package.yml`
- `.github/workflows/benchmark.yml`

They are not PR gates because they encode pre-revival assumptions such as older Python versions, legacy Actions versions, full-suite builds, TestPyPI publishing, or benchmark publishing. Keeping them manual preserves history without making unstable or outdated checks block the revived core.
