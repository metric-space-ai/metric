# METRIC Python Package

Python bindings for METRIC metric-space numerics.

The public Python distribution name is `metric-space`. The import package remains
`metric`.

The revived core package exposes the project concepts explicitly:

- `metric.metrics`: metric constructors such as `Edit`
- `metric.Space`: minimal intent-first finite metric space facade
- `metric.spaces`: finite metric-space helpers such as `FiniteMetricSpace`
- `metric.operators`: pairwise-distance, nearest-neighbor, range-neighbor, exact graph-result, graph-degree-diagnostics, graph-symmetrization, graph-out-degree-pruning, exact graph-edge, representative-selection, medoid, separated-representative, and radius-coverage helpers
- `metric.mappings`: beta compatibility bridge for installed mapping bindings
- `metric.transforms`: beta compatibility bridge for installed transform bindings

The same objects are also available as convenience imports from `metric` for short examples.

# Installation

The Python package is in revival. The current supported CPython targets for the core wheel are 3.10, 3.11, 3.12, 3.13, and 3.14.

After the package is published to PyPI, install the core package with:

```shell
python -m pip install metric-space
```

Until PyPI publishing is completed, build from source with the instructions below.

Packaging now uses `pyproject.toml` with isolated PEP 517 builds and PEP 621 project metadata. `setup.py` provides the CMake extension build hook and the source-distribution hook that carries the required C++ headers into the Python sdist. The build prefers the `pybind11` package from the build environment (`pybind11>=3.0.0`) and falls back to a current FetchContent copy when no CMake package is available.

CI builds core wheel artifacts for Linux, macOS, and Windows across the supported CPython versions.

# Build from the source

```
git clone --recurse-submodules https://github.com/metric-space-ai/metric
```

## Install Prerequisites

### Ubuntu

```
sudo apt-get install cmake
sudo apt-get install libboost-all-dev
```

OpenBLAS or another BLAS implementation is optional for the revived core wheel. It can improve broader linear-algebra paths, but the CI core wheel builds with `METRIC_PYTHON_USE_BLAS=OFF`.

### Windows

Install [Miniconda](https://docs.conda.io/en/latest/miniconda.html).
In Conda CLI initialize your virtual environment with desired Python version:

```bash
conda create --name my_env -y python=3.12
conda activate my_env
```

OpenBLAS is optional for the revived core wheel. For legacy full bindings or broader linear-algebra paths, install it from conda-forge:

```bash
conda config --add channels conda-forge
conda update -n base conda -y
conda install -c conda-forge libopenblas openblas -y
```

## Build package

At least 2GB of RAM is required

```
python -m pip wheel . --no-deps -w dist
```

By default this builds the revived core bindings. At this stage the default wheel exposes the `Edit` distance binding, space bindings, transform bindings, and the pure Python package surface. The legacy standard-distance, utils, mapping, and correlation bindings still require restoration and can be attempted with:

```shell
CMAKE_COMMON_VARIABLES="-DMETRIC_PYTHON_BUILD_FULL=ON" python -m pip wheel . --no-deps -w dist
```

To force the portable core-wheel mode used by CI:

```shell
CMAKE_COMMON_VARIABLES="-DMETRIC_PYTHON_USE_BLAS=OFF" python -m pip wheel . --no-deps -w dist
```

### Install module

```
python -m pip install dist/*
```

## Examples

```python
from metric import Edit, Space

records = ["cat", "cot", "coat", "dog"]
space = Space(records, Edit())

print("distance(cat, cot) =", space(0, 1))
print("nearest:", space.neighbors("cut", 2))
```

The core-wheel example is also available at `examples/metric_space/string_edit_space.py`. Full correlation, mapping, and standard-distance examples remain part of the broader restoration path until their bindings are promoted into the default wheel.

`metric.mappings` and `metric.transforms` are importable beta namespaces. They expose `available()` and `legacy_module()` so callers can inspect whether a wheel includes the broader legacy bindings without making those bindings part of the core-wheel contract.

## NumPy records

The revived Python facade treats NumPy rows as records when the metric callable accepts them:

```python
import numpy as np
from metric import Space

records = np.array([[0.0, 0.0], [3.0, 4.0], [6.0, 8.0]])
space = Space(records, lambda lhs, rhs: float(np.linalg.norm(lhs - rhs)))
print(space.distance(0, 1))
```

Pairwise distance helpers return Python lists of lists; call `np.asarray(...)` if an ndarray result is required.
