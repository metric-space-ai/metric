# METRIC

[![Core C++ smoke](https://github.com/metric-space-ai/metric/actions/workflows/core-cpp.yml/badge.svg)](https://github.com/metric-space-ai/metric/actions/workflows/core-cpp.yml)
[![Python core wheel](https://github.com/metric-space-ai/metric/actions/workflows/python-core.yml/badge.svg)](https://github.com/metric-space-ai/metric/actions/workflows/python-core.yml)
[![Docs and formatting](https://github.com/metric-space-ai/metric/actions/workflows/docs-and-format.yml/badge.svg)](https://github.com/metric-space-ai/metric/actions/workflows/docs-and-format.yml)

METRIC is a native C++ numerical engine for finite metric spaces.

It computes on records whose geometry is defined by an explicit metric. The records can be strings, time series, histograms, image-like distributions, graphs, structured records, or vectors. Vector spaces are supported as one record domain, not treated as the foundation of the framework.

## Links

- Project page: <https://metric-space-ai.github.io/metric/>
- Technical documentation: <https://metric-space-ai.github.io/metric/docs.html>
- Docs source: [docs/index.md](docs/index.md)
- C++ API: [docs/api/cpp.md](docs/api/cpp.md)
- Finite metric spaces: [docs/concepts/finite-metric-space.md](docs/concepts/finite-metric-space.md)
- Stability labels: [docs/stability.md](docs/stability.md)
- Changelog: [CHANGELOG.md](CHANGELOG.md)

## Finite Metric Spaces

A finite metric space is a finite set of records plus a distance function:

```text
X = {x0, x1, ..., xn}
d(x, y) = distance between two records
(X, d) = finite metric space
```

The metric supplies the geometry. METRIC keeps that metric visible across spaces, representations, operators, mappings, diagnostics, and bindings.

This matters when a domain metric is more meaningful than a generic vector embedding:

- edit distance for symbolic strings
- transport cost for histograms and distributions
- alignment distance for process curves
- structural distance for graphs or image-like records
- composed domain metrics for mixed industrial records

## C++ Quickstart

```cpp
#include <metric/distance.hpp>
#include <metric/engine.hpp>

#include <string>
#include <vector>

int main()
{
    std::vector<std::string> records = {"metric", "metrics", "matrix", "tree"};

    auto space = metric::make_space(records, metric::Edit<char>{});
    metric::representations::MatrixCache<decltype(space)> matrix(space);

    const auto neighbors = metric::find_neighbors(
        space,
        std::string("metricks"),
        metric::count{2},
        metric::strategies::cover_tree{});

    return neighbors.size() == 2 ? 0 : 1;
}
```

The same space can be queried lazily, through a materialized distance matrix, through graph or tree representations, or through mapping and diagnostic operators.

## Public Surface

- Native C++ metrics, spaces, representations, operators, mappings, diagnostics, runtime metadata, and examples.
- Python bindings and adapters for loading user data and calling selected native surfaces.
- Explicit stability labels for promoted, compatibility, beta, experimental, and historical modules.
- Deterministic C++ examples under [examples/engine](examples/engine).

Python does not contain independent METRIC algorithms. It is a binding and adapter layer over native functionality.

## Hero Examples

The engine examples compare native metric-space workflows with vector baselines.

| Demo | Native metric result | Vector baseline |
|---|---:|---:|
| [String edit](docs/examples/string-edit-baseline.md) | 4/4 expected symbolic families | 4/4 misses |
| [Histogram transport](docs/examples/histogram-transport-baseline.md) | 4/4 shifted-mass matches | 4/4 misses |
| [Process curve gallery](docs/examples/process-curve-external-gallery.md) | 16/16 UCR anomaly-window matches | 16/16 misses |
| [Distribution/image recoding](docs/examples/distribution-image-recoding-baseline.md) | 4/4 shifted-structure matches | 4/4 misses |
| [Mixed structured records](docs/examples/mixed-structured-record-baseline.md) | 4/4 expected domain families | 4/4 misses |
| [Cross-space dependency](docs/examples/cross-space-dependency-baseline.md) | MGC detects paired-space dependency | 9/12 raw-pairing misses |
| [PHATE-AE pipeline](docs/examples/phate-ae-pipeline-hero.md) | process-curve latent nearest families correct for 6/6 held-out queries | 6/6 misses |

These examples are C++ executables with documented output under [docs/examples/engine-demo-outputs.md](docs/examples/engine-demo-outputs.md).

## Build From Source

The C++ core can be consumed directly from source:

```shell
cmake --preset core
cmake --build --preset core
ctest --preset core
```

For a package-manager based build, install:

- C++17 compiler
- CMake 3.19+
- nlohmann_json
- cereal
- BLAS/LAPACK for faster linear algebra paths

Then configure without dependency fetching:

```shell
cmake -S . -B build/core \
  -DMETRIC_BUILD_CORE_TESTS=ON \
  -DMETRIC_BUILD_CORE_EXAMPLES=ON \
  -DMETRIC_FETCH_DEPS=OFF
cmake --build build/core
ctest --test-dir build/core --output-on-failure
```

The numerical matrix/vector core is included in-tree as `metric/numeric`; it is not a separate package-manager dependency.

## CMake Integration

```cmake
find_package(panda_metric REQUIRED)

add_executable(program program.cpp)
target_link_libraries(program PRIVATE panda_metric::panda_metric)
```

## Python Binding

```shell
python -m pip install ./python
```

```python
from metric import Edit, Space

records = ["red", "reed", "road", "blue"]
space = Space(records, Edit())
print(space.neighbors("read", k=2))
```

The Python package adapts Python records to the native engine surface. It does not define a separate algorithmic implementation.

## Not A Vector Database

METRIC does not provide managed persistence, online indexing, replication, authorization, or hosted search. Use it when the metric itself is the modeling decision and the record geometry should remain explicit.

## License

See [LICENSE](LICENSE).
