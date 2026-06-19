# METRIC

[![Core C++ smoke](https://github.com/metric-space-ai/metric/actions/workflows/core-cpp.yml/badge.svg)](https://github.com/metric-space-ai/metric/actions/workflows/core-cpp.yml)
[![Python core wheel](https://github.com/metric-space-ai/metric/actions/workflows/python-core.yml/badge.svg)](https://github.com/metric-space-ai/metric/actions/workflows/python-core.yml)
[![Docs and formatting](https://github.com/metric-space-ai/metric/actions/workflows/docs-and-format.yml/badge.svg)](https://github.com/metric-space-ai/metric/actions/workflows/docs-and-format.yml)

METRIC is a numerical computing engine for finite metric spaces.

It is built for datasets whose geometry is defined by distances between records. The records may be strings, time series, graphs, histograms, images, structured objects, or vectors. Vector spaces are supported as a special case, not assumed as the foundation.

METRIC is not another vector database and not a loose algorithm catalog. It provides one coherent engine layer around finite metric spaces: metrics, spaces, representations, operators, mappings, diagnostics, and runtimes.

## Links

- Project page: <https://metric-space-ai.github.io/metric/>
- Technical documentation: <https://metric-space-ai.github.io/metric/docs.html>
- Docs source: [docs/index.md](docs/index.md)
- Engine overview: [docs/engine/index.md](docs/engine/index.md)
- API surface: [docs/stability.md](docs/stability.md)
- Research roadmap: [docs/research-roadmap.md](docs/research-roadmap.md)
- Revival status: [docs/revival-status.md](docs/revival-status.md)
- Changelog: [CHANGELOG.md](CHANGELOG.md)

## Quick Introduction

Most numerical and AI tooling starts with vectors:

- tensor libraries operate on arrays
- vector databases index embedding vectors
- many machine-learning APIs assume feature matrices
- many algorithm packages expose separate estimators

METRIC starts one level more generally:

```text
I have records.
I can compare two records with a meaningful metric.
Therefore I have a finite metric space.
Now I want to compute with that space.
```

That difference matters when the native distance is more meaningful than a vector embedding: edit distance for strings, transport cost for distributions, time-warp distance for process curves, graph distance for structures, perceptual distance for images, or custom industrial penalties for mixed records.

## Engine Model

```text
RecordSet + Metric -> MetricSpace -> Intent -> Strategy -> Representation -> Runtime -> Result
```

The revived engine surface exposes metric constructors, finite metric spaces, representation adapters, semantic intents, explicit strategies, mapping results, and runtime policies. The default vocabulary is user intent:

- `neighbors`
- `groups`
- `embed`
- `map`
- `reduce`
- `denoise`
- `outliers`
- `compare`

Concrete algorithms are strategies. A user asks for `groups` or `outliers`; k-medoids, DBSCAN, KOC, and future density methods are execution choices. A user asks to `map`; clustered-space mappings, PCFA, deterministic transforms, neural mappers, and autoencoder-based strategies are interchangeable implementations behind the same intent. The promoted examples include the stable core API and the first CI-tested engine demos for strings, process curves, histograms, and cross-space dependency.

## Engine Quickstart Shape

The first engine path starts from non-vector records and keeps representation, strategy, and result metadata explicit:

```cpp
#include <metric/distance.hpp>
#include <metric/engine.hpp>

#include <string>
#include <vector>

std::vector<std::string> records = {"metric", "metrics", "matrix", "tree"};
auto space = metric::make_space(records, metric::Edit<char>{});

metric::representations::MatrixCache<decltype(space)> matrix(space);
auto neighbors = metric::find_neighbors(space, std::string("metricks"), 2, metric::strategies::cover_tree{});
auto groups = metric::find_groups(space, metric::strategies::k_medoids(2));
```

The same `MetricSpace` can be queried lazily, through a materialized matrix, or through an index strategy. Algorithms are named at the strategy layer; the user-facing calls remain `find_neighbors`, `find_groups`, `find_representatives`, `find_outliers`, `compare`, `reduce`, and `describe_structure`.

Mappings produce explicit derived spaces:

```cpp
auto mapping = metric::mappings::make_clustered_space_mapping(groups);
auto model = metric::mappings::fit(mapping, space);
auto clustered = metric::mappings::transform(model, space);
```

`clustered.space` is a new finite metric space. The `MappingResult` keeps source `RecordId` lineage, representative records, strategy metadata, and whether inverse reconstruction is supported.

## Python Example

```python
from metric import Edit, Space

records = ["cat", "cot", "coat", "dog"]
space = Space(records, Edit())

print("distance(cat, cot) =", space(0, 1))

for record_id, distance in space.neighbors("cut", 2):
    print(f"{records[record_id]}: {distance}")
```

The records are strings. They are not embedded into vectors first. Edit distance defines the geometry, and the `Space` object can drive search and operator workflows.

## C++ Example

```cpp
#include <iostream>
#include <string>
#include <vector>

#include <metric/distance.hpp>
#include <metric/engine.hpp>

int main()
{
    std::vector<std::string> records = {"metric", "metrics", "matrix", "tree"};

    auto space = metric::make_space(records, metric::Edit<char>{});
    metric::representations::MatrixCache<decltype(space)> matrix(space);

    std::cout << "distance(metric, metrics) = "
              << matrix.distance(space.id(0), space.id(1)) << "\n";

    const auto neighbors = metric::find_neighbors(
        space,
        std::string("metricks"),
        2,
        metric::strategies::cover_tree{});
    for (const auto &neighbor : neighbors) {
        std::cout << records[neighbor.id.index()] << ": " << neighbor.distance << "\n";
    }

    const auto groups = metric::find_groups(space, metric::strategies::k_medoids(2));
    std::cout << "string groups = " << groups.cluster_count << "\n";

    return neighbors.size() == 2 ? 0 : 1;
}
```

The lower-level C++ representations remain available for expert control and compatibility:

- `metric::Space`
- `metric::Metric`
- `metric::FiniteSpace`
- `metric::operators::pairwise_distance_matrix`
- `metric::operators::nearest_neighbors`
- `metric::operators::range_neighbors`
- `metric::operators::GraphConnectivityDiagnostics`
- `metric::operators::GraphDegreeDiagnostics`
- `metric::operators::GraphStretchDiagnostics`
- `metric::operators::GraphConstructionResult`
- `metric::operators::GraphConstructionMetadata`
- `metric::operators::graph_connectivity_diagnostics`
- `metric::operators::graph_degree_diagnostics`
- `metric::operators::graph_stretch_diagnostics`
- `metric::operators::exact_knn_graph`
- `metric::operators::exact_knn_graph_edges`
- `metric::operators::exact_radius_graph`
- `metric::operators::exact_radius_graph_edges`
- `metric::operators::symmetrize_graph`
- `metric::operators::prune_graph_out_degree`
- `metric::operators::representative_indices`
- `metric::operators::representatives`
- `metric::operators::medoid_index`
- `metric::operators::medoid`
- `metric::operators::separated_representative_indices`
- `metric::operators::separated_representatives`
- `metric::operators::coverage_representative_indices`
- `metric::operators::coverage_representatives`
- `metric::operators::intrinsic_dimension`
- `metric::MatrixSpace`
- `metric::GraphSpace`
- `metric::TreeSpace`
- `metric::Matrix`
- `metric::KNNGraph`
- `metric::Tree`

## What METRIC Gives Users

METRIC gives users a stable way to compute with non-vector data without discarding the metric that makes the data meaningful.

- Domain metrics stay explicit.
- Vector spaces work as a normal special case.
- Representations such as matrices, graphs, and trees are runtime choices.
- Intent names stay understandable.
- Algorithm names remain available as strategies.
- End-to-end workflows are composed from reusable engine parts.
- Result objects preserve diagnostics, lineage, assumptions, and runtime metadata.

## When Should I Use METRIC?

Use METRIC when the distance between records is the core modeling decision.

- Your records are strings, time series, histograms, graphs, images, structured objects, or mixed domain records.
- You have a domain metric that is more meaningful than a generic embedding distance.
- You want explicit metric-space representations such as matrices, nearest-neighbor graphs, or search trees.
- You need operators such as nearest-neighbor search, entropy diagnostics, MGC correlation, sparsification, or cross-space analysis.
- You want vector methods available as a special case without making vectors the foundation of the workflow.

## When Should I Not Use METRIC?

Do not use METRIC as the first tool when the problem is already solved by a specialized vector, tensor, or database system.

- You only need dense tensor training, automatic differentiation, or GPU neural-network layers.
- You only need large-scale approximate vector search over precomputed embeddings.
- You need a managed vector database with persistence, replication, authorization, and online indexing.
- You do not have a meaningful metric or distance model for the records.
- You need a fully polished high-level Python estimator API today; that facade is still being restored.

## Documentation

Start with:

- [METRIC Docs](docs/index.md)
- [Engine Overview](docs/engine/index.md)
- [C++ API](docs/api/cpp.md)
- [Python API](docs/api/python.md)
- [API Surface](docs/stability.md)

Core concepts:

- [Metric Spaces](docs/concepts/metric-space.md)
- [Finite Metric Spaces](docs/concepts/finite-metric-space.md)
- [Metrics as Recoding Costs](docs/concepts/metrics-as-recoding-costs.md)
- [Vector Space as a Special Case](docs/concepts/vector-space-as-special-case.md)
- [Explicit Representations](docs/concepts/explicit-representations.md)

Examples:

- [Custom Metric Example](docs/examples/custom-metric.md)
- [Structured Data](docs/examples/structured-data.md)
- [Time-Series Space With TWED](docs/examples/time-series-twed.md)
- [Histogram Space With EMD](docs/examples/histogram-emd.md)
- [Representative Selection](docs/examples/representative-selection.md)
- [Entropy Diagnostics](docs/examples/entropy-diagnostics.md)
- [Intrinsic Dimension Diagnostic](docs/examples/intrinsic-dimension.md)
- [Correlation Between Metric Spaces](docs/examples/correlation-between-spaces.md)
- [Industrial Anomaly Workflow](docs/examples/industrial-anomaly-workflow.md)

## Build From Source

The C++ core is available as a package target and can also be consumed directly from source:

```shell
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

For the narrow C++ core profile:

```shell
cmake --preset core
cmake --build --preset core
ctest --preset core
```

For a package-manager based build, install:

- C++17 compiler
- CMake 3.19+
- Blaze
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

## Integration

### CMake

```cmake
find_package(panda_metric REQUIRED)

add_executable(program program.cpp)
target_link_libraries(program PRIVATE panda_metric::panda_metric)
```

### Python

```shell
python -m pip install ./python
```

```python
from metric import Edit, Space

records = ["red", "reed", "road", "blue"]
space = Space(records, Edit())
print(space.neighbors("read", k=2))
```

## Compatibility

Historical names remain supported where they are part of existing user code. New examples use the engine vocabulary, but compatibility APIs are documented and tested as adapter surfaces.

## Stable Core and Research Breadth

The revived stable core is intentionally small: distance metrics, typed C++ metric callables, minimal C++ and Python `Space` facades, explicit spaces, C++ and Python operator helpers, nearest-neighbor access, entropy and MGC examples, reproducible builds, and CI-backed examples.

METRIC also preserves broader research work in mappings, transforms, neural approximators, industrial demos, and domain-specific experiments. Python exposes `metric.mappings` and `metric.transforms` as beta compatibility bridges so users can inspect installed legacy bindings without treating them as stable core APIs. Those modules remain valuable, but they are not advertised as stable release gates until they have deterministic tests, documented assumptions, and promoted examples. The current module-by-module support status is maintained in [API Surface](docs/stability.md).

## Background

METRIC grew out of the PANDA project and earlier industrial AI experiments. That history explains the breadth of algorithms still present in the repository: mappings, feature extractors, transforms, DNN components, and anomaly workflows. The revived public frame is narrower and clearer: compute with finite metric spaces directly, then use mapping or learning layers only when they serve that metric-space workflow.

## License

See [LICENSE](LICENSE).
