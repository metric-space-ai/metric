# Building METRIC

METRIC is currently being revived around a small, reproducible C++ core build. The default developer path builds the header-only core target, minimal core examples, and a smoke test.

## Quick Developer Build

Use this path from a clean checkout when you do not already have all C++ dependencies installed:

```shell
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

The `dev` preset enables `METRIC_FETCH_DEPS=ON` and fetches missing header dependencies with CMake `FetchContent`.

## Install and Consume

The core target installs a CMake package named `panda_metric`. When configured with `METRIC_FETCH_DEPS=ON`, the fetched header-only dependencies are installed with the package so a downstream CMake consumer can use the installed prefix directly:

```shell
cmake --install build/core --prefix build/install-core
cmake -S tests/downstream_consumer -B build/downstream-consumer \
  -DCMAKE_PREFIX_PATH="$PWD/build/install-core"
cmake --build build/downstream-consumer
ctest --test-dir build/downstream-consumer --output-on-failure
```

Downstream projects link the exported target:

```cmake
find_package(panda_metric REQUIRED)

add_executable(program program.cpp)
target_link_libraries(program PRIVATE panda_metric::panda_metric)
```

## Core Dependencies

The C++ core requires:

- C++17 compiler
- CMake 3.19+
- Blaze 3.8
- nlohmann_json
- cereal
- BLAS/LAPACK for faster linear algebra paths

`METRIC_FETCH_DEPS=ON` can fetch Blaze, nlohmann_json, and cereal for developer builds. Package-manager mode should provide those dependencies before configuring.

## Package-Manager Build

Use this path when dependencies are already installed:

```shell
cmake -S . -B build/core \
  -DMETRIC_BUILD_CORE_TESTS=ON \
  -DMETRIC_BUILD_CORE_EXAMPLES=ON \
  -DMETRIC_BUILD_TESTS=OFF \
  -DMETRIC_BUILD_EXAMPLES=OFF \
  -DMETRIC_BUILD_BENCHMARKS=OFF \
  -DMETRIC_FETCH_DEPS=OFF
cmake --build build/core
ctest --test-dir build/core --output-on-failure
```

## Presets

- `core`: narrow core library and smoke test.
- `dev`: default developer build; currently equivalent to `core` in its tested surface.
- `full-tests`: broad C++ test suite. This path is still being revived.
- `release`: release-style core build.
- `python-cmake`: top-level CMake build for the revived Python core bindings.

## Build Options

- `METRIC_BUILD_CORE_TESTS`: build the minimal core smoke test. Default: `ON`.
- `METRIC_BUILD_CORE_EXAMPLES`: build the minimal core examples. Default: `ON`.
- `METRIC_BUILD_TESTS`: build the full C++ test suite. Default: `OFF`.
- `METRIC_BUILD_EXAMPLES`: build examples. Default: `OFF`.
- `METRIC_BUILD_BENCHMARKS`: build benchmarks. Default: `OFF`.
- `METRIC_BUILD_PYTHON`: build the Python bindings from the top-level CMake project. Default: `OFF`.
- `METRIC_FETCH_DEPS`: fetch missing C++ dependencies with `FetchContent`. Default: `OFF`.

The recommended Python package path remains:

```shell
METRIC_PYTHON_USE_BLAS=OFF python -m pip wheel ./python --no-deps -w wheelhouse
```

The release artifact path additionally proves that the Python source distribution is self-contained enough to build the wheel:

```shell
python -m pip install --upgrade build
python -m build ./python --sdist --outdir wheelhouse
METRIC_PYTHON_USE_BLAS=OFF python -m pip wheel wheelhouse/*.tar.gz --no-deps -w wheelhouse
```

Use the top-level CMake path when validating the CMake integration directly:

```shell
cmake --preset python-cmake
cmake --build --preset python-cmake --parallel 2
```

The legacy `BUILD_TESTS` option is accepted as a deprecated alias when set to `ON`; use `METRIC_BUILD_TESTS` for new builds.

## CI Coverage

The revived core C++ workflow configures, builds, tests, installs, and consumes the installed package on Linux, macOS, and Windows. The Python core workflow builds wheel artifacts and runs the promoted Python smoke example and core Python tests across Linux, macOS, Windows, and supported CPython versions.

Release gates are tracked in [docs/release-checklist.md](docs/release-checklist.md).
