# METRIC

[![Core C++ smoke](https://github.com/metric-space-ai/metric/actions/workflows/core-cpp.yml/badge.svg)](https://github.com/metric-space-ai/metric/actions/workflows/core-cpp.yml)
[![Python core wheel](https://github.com/metric-space-ai/metric/actions/workflows/python-core.yml/badge.svg)](https://github.com/metric-space-ai/metric/actions/workflows/python-core.yml)
[![Docs and formatting](https://github.com/metric-space-ai/metric/actions/workflows/docs-and-format.yml/badge.svg)](https://github.com/metric-space-ai/metric/actions/workflows/docs-and-format.yml)

Native C++ framework for finite metric spaces.

METRIC is built for data that is meaningful before it is forced into vectors:
strings, process curves, histograms, image distributions, graphs, event
sequences, and mixed industrial records. Choose a true metric for two records,
and METRIC makes the whole finite collection computable.

```text
records + metric -> finite metric space
finite metric space -> search, entropy, correlation, sampling, transformation
```

## TL;DR

METRIC starts from a deliberately small model.

1. There are observations.
2. Observations are different and distinguishable.
3. In software we do not have the observations themselves; we have finite
   records of them.
4. Records and observations are not identical.
5. The records themselves do not have to be numbers, vectors, or sortable
   objects.
6. What can be ordered is a relation between two records.
7. A metric is the rule that assigns the smallest admissible relation value to
   each record pair.

That metric value can be interpreted as distance, minimal recoding effort, path
length through an imagined intermediate space, or another domain-specific view.
The interpretation is downstream. The computable object is the finite metric
space: a finite record set plus one authoritative metric.

The C++ namespace is `mtrc`. The project and package name remain METRIC.

## Why METRIC

Most data analysis stacks assume the data is already numeric, tabular, or
embedded into vectors. That is often the wrong starting point.

A machine state can include a time series, a categorical state, maintenance
text, a distribution, and a sensor vector. A process window can be more
naturally compared by alignment than by coordinate overlap. A histogram or
image-like distribution can be compared by movement of mass. A string or event
sequence can be compared by edits.

METRIC keeps those native record forms and lets a metric define the numerical
structure. The user usually does not need to invent a metric from scratch:
METRIC provides admitted metrics for common record domains and keeps unproven
or non-metric pairwise functions out of the normal metric surface.

Once two records can be compared, the finite record set becomes a finite metric
space. METRIC can then compute neighborhoods, representatives, entropy,
correlation between paired spaces, outliers, modified spaces, and derived maps
without requiring one global embedding model first.

## Core Idea

Software never has observations directly. It has records: finite recordings of
observations. A record is not identical to the observation it records, and it
does not have to be sortable or numeric.

METRIC starts with a finite set of records and a metric. The metric assigns one
numeric value to each record pair. That value can later be interpreted as
distance, recoding cost, transformation cost, path length through an imagined
intermediate space, diffusion structure, or mapping target, depending on the
domain and operation.

The metric comes first. The interpretation comes after.

Vector spaces are included as a special case: records are coordinate records and
the metric is induced by a coordinate metric such as Euclidean, Manhattan, or
Chebyshev. Embeddings are useful derived spaces, not a required input format.

## What You Can Compute

- nearest-neighbor search and range queries
- representative selection and finite-space reduction
- clustering and outlier detection
- entropy and intrinsic-structure diagnostics
- correlation and dependence between paired metric spaces
- sampling and metric-space walks over finite geometry
- graph, table, tree, and indexed representations of the same source space
- modified and derived metric spaces, including coordinate spaces when useful

## Documentation Hierarchy

METRIC documentation is organized by the object being computed, not by a list of
algorithms.

| Level | What belongs here | Examples |
| --- | --- | --- |
| Level 1: finite metric-space model | Records, metrics, spaces, entropy, correlation, sampling, and transformation as questions about a finite set `(X, d)`. | "What is the record?", "What is the metric?", "What property or derived space is being computed?" |
| Level 2: framework components | C++ namespaces and components that own those questions. | `mtrc::record`, `mtrc::metric`, `mtrc::space`, `mtrc::stats::properties`, `mtrc::stats::correlate`, `mtrc::stats::sample`, `mtrc::modify::map`, `mtrc::modify::compose`, `mtrc::solve`. |
| Level 3: concrete implementations | Algorithms, estimators, papers, and compatibility adapters inside a Level-2 component. | TWED, Wasserstein/EMD, MGC, PHATE, autoencoders, native DNN solvers, PCFA, SOM/KOC, DBSCAN, MDS. |

PHATE, AE, DNN, MGC, and entropy are described through this hierarchy:
entropy is a property of an existing finite metric space; MGC is a dependence
test between aligned metric spaces; PHATE and autoencoder workflows construct
derived coordinate spaces; the native DNN code is a solver used by fitted
metric-space mappings.

## Metrics

METRIC separates true metrics from similarities, scores, divergences, and other
pairwise functions. Operators that depend on metric geometry require metrics
with documented metric-law behavior on their domain.

Core metric families include:

- Euclidean, Manhattan, Chebyshev, and Minkowski/P-norm metrics
- edit distance for strings and symbolic sequences
- transport metrics for histograms and distributions
- time-series and sequence alignment metrics such as TWED/ERP-style distances
- Ruzicka / weighted Jaccard-style metrics for nonnegative records
- composed metrics for structured records

See [Metric Discovery](docs/metrics/index.md) and
[True Metric Catalog](docs/metrics/true-metric-catalog.md).

## Example Applications

### Condition Monitoring

Process windows can remain time-series records. A curve metric can compare
windows by alignment. Entropy, representatives, outliers, and cross-space
dependence can then operate on the process geometry directly.

Docs:
[process curves](docs/examples/process-curve-external-gallery.md),
[entropy diagnostics](docs/examples/entropy-diagnostics.md),
[industrial anomaly workflow](docs/examples/industrial-anomaly-workflow.md).

### Distribution And Image Recoding

Histograms and image-like distributions can be compared by movement of mass.
This captures shifted structure that coordinate-wise overlap can miss.

Docs:
[histogram transport](docs/examples/histogram-transport-baseline.md),
[distribution/image recoding](docs/examples/distribution-image-recoding-baseline.md).

### Mixed Records

A record can combine numeric fields, categories, text, histograms, process
curves, and other record types. METRIC can compose field metrics so the combined
record remains directly computable.

Docs:
[mixed structured records](docs/examples/mixed-structured-record-baseline.md),
[cross-space dependency](docs/examples/cross-space-dependency-baseline.md).

### Metric-Aware Maps

When coordinates are useful, METRIC treats them as derived spaces. PHATE-style
geometry and autoencoder mappings are used as ways to produce coordinate views
from an existing metric space, while the source metric remains explicit.

Docs:
[PHATE-AE pipeline](docs/examples/phate-ae-pipeline-workflow.md).

## C++ Quickstart

```cpp
#include <metric/workflow.hpp>

#include <string>
#include <vector>

int main()
{
    std::vector<std::string> input = {"metric", "metrics", "matrix", "tree"};

    auto records = mtrc::record::import_records(input);
    auto space = mtrc::space::build_checked(records, mtrc::Edit<char>{});

    auto profile = mtrc::stats::profile(space);
    auto diagnosis = mtrc::stats::diagnose_space(space);
    auto neighbors = mtrc::space::query::k_nearest(space, std::string("metricks"), 2);
    auto representatives = mtrc::modify::represent::represent(space, 2);

    return profile.record_count == 4 && diagnosis.discoverable_metric &&
           neighbors.size() == 2 && representatives.size() == 2 ? 0 : 1;
}
```

For practical applications the recommended C++ entry point is
`<metric/workflow.hpp>`. It aggregates the stable Level-1 workflow surfaces:
record import/validation, metric catalog/admission, space construction and
pair-value access, read-only stats, and derived-space modify components. The
underlying ownership stays explicit: `record` handles records, `metric` handles
metrics, `space` handles finite spaces, `stats` investigates an existing space,
and `modify` creates derived spaces.

Mixed records can be imported from parallel typed field columns with
`mtrc::record::import_mixed_records`; the returned report keeps field-count and
row-count validation explicit before a composed metric is selected.

For homogeneous numeric records in files, `mtrc::record::read_csv<double>` and
`read_tsv<double>` load vector records directly in C++. They validate row shape,
parse failures, and non-finite values before a metric is selected. The matching
`write_csv` and `write_tsv` helpers export records without pulling a separate
CSV dependency into the core.

Metric Discovery is available at runtime through
`mtrc::metric::discover_metrics(mtrc::record_kind::sequence)`. The returned
entries expose the record family, metric-law/admission status, domain gate, and
metric alternative for quarantined or rejected compatibility distances.

`mtrc::stats::diagnose_space` gives one compact read-only report for an existing
space: profile, metric-law/admission status, a neighbor check, and k-NN outlier
scores.

Copyable C++ application templates live under
[`examples/templates`](examples/templates). They cover the practical first
steps after the TL;DR model:

- time-series condition monitoring with TWED
- histogram and image-like distribution comparison with strict Wasserstein
- mixed records assembled from typed columns
- correlation/dependence between paired finite metric spaces
- representative record-set reduction
- derived coordinate spaces when a coordinate view is useful

Finite spaces can also be exported as native C++ artifacts:
`mtrc::space::persistence::save` stores records, stable `RecordId`s, the space
version, metric identity, and optional materialized pair values; `load` restores
the space with an explicitly supplied metric. Subspace and merged-space
artifacts preserve their existing lineage rows.

## Build From Source

```shell
cmake --preset core
cmake --build --preset core
ctest --preset core
```

For a package-manager based build, install:

- C++17 compiler
- CMake 3.19+
- BLAS/LAPACK for the stats/entropy layer used by `<metric/workflow.hpp>` (the quickstart
  above calls `mtrc::stats::profile`/`diagnose_space`, which pull the LAPACK-backed entropy
  path). The exported `metric::metric` target links it automatically when found. It is only
  optional if you restrict yourself to records, metrics, and space/search without the
  stats/entropy layer.

Then configure without dependency fetching:

```shell
cmake -S . -B build/core \
  -DMETRIC_BUILD_CORE_TESTS=ON \
  -DMETRIC_BUILD_CORE_EXAMPLES=ON
cmake --build build/core
ctest --test-dir build/core --output-on-failure
```

The numerical matrix/vector core is included in-tree as `metric/numeric`; it is
not a separate package-manager dependency.

## CMake Integration

```cmake
find_package(metric REQUIRED)

add_executable(program program.cpp)
target_link_libraries(program PRIVATE metric::metric)
```

The `metric::metric` target carries its LAPACK dependency automatically (re-resolved by the
package config via `find_dependency(LAPACK)`), so the stats/entropy and
`<metric/workflow.hpp>` paths link without extra flags. For a raw header-only compile without
CMake, link LAPACK yourself, e.g. `c++ -std=c++17 -I. app.cpp -framework Accelerate` on macOS
or `c++ -std=c++17 -I. app.cpp -llapack` on Linux.

## Python Binding

Python is an adapter layer over the native implementation. The current core wheel
runs space construction and inspection (`distance`, `pairwise`, and the
`to_matrix`/`to_tree`/`to_graph` representation views), exact neighbor search
(`neighbors`, `nearest`, `within_radius`), representative selection, reduction,
compression, structural description, intrinsic-dimension diagnostics, and the
metric constructors. Higher intent methods such as `embed` and
`compare`/`correlate` still raise `StrategyUnavailableError` until their native
bindings are promoted in the default wheel. Use the C++ surface for those
analyses today.

```shell
python -m pip install ./python
```

```python
from metric import Edit, Space

records = ["red", "reed", "road", "blue"]
space = Space(records, Edit())

print(space.distance(0, 1))   # 1
print(space.pairwise())       # cached pairwise edit distances
print([neighbor.record for neighbor in space.neighbors("read", count=2).neighbors])
print(space.describe().to_dict()["intrinsic_dimension"])
```

## Documentation

- Project page: <https://metric-space-ai.github.io/metric/>
- Technical documentation: <https://metric-space-ai.github.io/metric/docs.html>
- Finite metric spaces: [docs/concepts/finite-metric-space.md](docs/concepts/finite-metric-space.md)
- METRIC vocabulary: [docs/concepts/metric-vocabulary.md](docs/concepts/metric-vocabulary.md)
- C++ API: [docs/api/cpp.md](docs/api/cpp.md)
- Python API: [docs/api/python.md](docs/api/python.md)
- Stability labels: [docs/stability.md](docs/stability.md)
- Changelog: [CHANGELOG.md](CHANGELOG.md)

## Scope

METRIC is not a hosted vector database. It does not provide managed
persistence, replication, authorization, or online service operations. Use it
when records and their metric are the central object of computation.

## License

See [LICENSE](LICENSE).
