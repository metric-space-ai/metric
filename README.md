# METRIC

[![Core C++ smoke](https://github.com/metric-space-ai/metric/actions/workflows/core-cpp.yml/badge.svg)](https://github.com/metric-space-ai/metric/actions/workflows/core-cpp.yml)
[![Python core wheel](https://github.com/metric-space-ai/metric/actions/workflows/python-core.yml/badge.svg)](https://github.com/metric-space-ai/metric/actions/workflows/python-core.yml)
[![Docs and formatting](https://github.com/metric-space-ai/metric/actions/workflows/docs-and-format.yml/badge.svg)](https://github.com/metric-space-ai/metric/actions/workflows/docs-and-format.yml)

Native C++ framework for finite metric spaces.

METRIC is the framework for working with finite metric spaces: finite record
collections whose structure is defined by one authoritative metric. It is built
for experimentation, data analysis, research workflows, and production systems
where records have structure before they are encoded as vectors.

Strings, process curves, histograms, image distributions, graphs, event
sequences, vectors, and mixed industrial records can all be source records.
Choose a true metric for two records, and METRIC makes the whole finite
collection computable.

```text
records + metric -> finite metric space
finite metric space -> search, entropy, correlation, sampling, transformation
```

The project deliberately uses metric-space language as its public vocabulary:
records, metrics, finite spaces, relations, measures, neighborhoods, lineage,
derived spaces, dynamics, and evidence. Classical machine-learning terms are
used only when they name an implementation detail, comparison baseline, or
external reference.

## Data Analysis With METRIC

Use METRIC when analysis depends on how records compare, not only on the
columns they expose. A record can be a customer history, machine cycle, text
event stream, histogram, image distribution, graph, vector row, or mixed
industrial object. The metric is the analysis decision: it defines what it
means for two records to be near, separated, redundant, representative, or
structurally related.

Common analysis jobs:

- Find similar cases under a domain metric: process windows, documents,
  customers, image distributions, graphs, event sequences, or mixed records.
- Build representative sets from real records for review panels, monitoring
  anchors, prototypes, baselines, and reduced datasets.
- Review anomalies and boundary cases by inspecting sparse neighborhoods,
  nearest-neighbor gaps, local density, and extreme distance profiles.
- Compare cohorts, sites, time periods, experiments, or system states through
  their distances, neighborhoods, representatives, and density structure.
- Connect modalities such as event logs and sensor curves, text and behavior,
  histograms and structured records, without requiring both sides to share one
  coordinate table first.
- Create coordinate views, maps, and visual evidence when they clarify the
  finite space, while keeping source records, metric identity, and lineage
  explicit.

The workflow is: define the records, choose or admit the metric, build the
finite metric space, then run search, diagnostics, comparison, modification,
mapping, and evidence export over that object.

## TL;DR

METRIC starts from a deliberately small premise.

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
Finite metric-space computing is the brand language for the project.

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
correlation between paired spaces, singular-record scores, modified spaces, and
derived maps without requiring one global coordinate representation first.

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
Chebyshev. Coordinate views are useful derived spaces, not a required input
format.

## Relationship To Classical ML

Coordinate-first workflows remain useful: they are fast and effective when the
relevant structure is already captured by numeric features. METRIC treats that
as an important special case, not as the foundation.

METRIC starts with a finite record set and the metric that defines its geometry.
From there it can derive coordinate tables, coordinate views, vector baselines,
and learning workflows when they are justified by the metric-space object.

## What You Can Compute

METRIC is organized around operations that make sense once a finite record set
has one authoritative metric. The exact set of promoted functions changes by
release, but the project surface is meant to cover these finite-space tasks:

- nearest-neighbor search and range queries
- representative selection and finite-space reduction
- density groups, components, and singular-record diagnostics
- entropy and intrinsic-structure diagnostics
- correlation and dependence between paired metric spaces
- sampling and metric-space walks over finite geometry
- graph, table, tree, and indexed representations of the same source space
- modified and derived metric spaces, including coordinate spaces when useful
- native evidence export and visual inspection of finite-space structure

## Documentation Hierarchy

METRIC documentation is organized by the object being computed, not by a list of
algorithms.

| Level | What belongs here | Examples |
| --- | --- | --- |
| Level 1: finite metric-space object | Records, metrics, spaces, entropy, correlation, sampling, and transformation as questions about a finite set `(X, d)`. | "What is the record?", "What is the metric?", "What property or derived space is being computed?" |
| Level 2: framework components | C++ namespaces and components that own those questions. | `mtrc::record`, `mtrc::metric`, `mtrc::space`, `mtrc::stats::properties`, `mtrc::stats::correlate`, `mtrc::stats::sample`, `mtrc::modify::map`, `mtrc::modify::compose`, `mtrc::solve`. |
| Level 3: concrete implementations | Algorithms, estimators, papers, and solver components inside a Level-2 component. | TWED, Wasserstein/EMD, MGC, diffusion coordinates, parametric coordinate solver components, PCFA, DBSCAN density grouping, MDS. |

Names such as diffusion coordinates, coordinate solver, MGC, and entropy are described through this
hierarchy, not used as the organizing principle. Entropy is a property of an
existing finite metric space; MGC is a dependence test between aligned metric
spaces; diffusion-coordinate workflows construct derived coordinate spaces; the
native parametric solver code is used by derived metric-space mappings.

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

**Featured hero — process-curve condition monitoring.** Real, CC BY 4.0 UCR
anomaly windows become a finite metric space over process curves, where a
nearest-record query recovers the right operating condition that a padded
point-vector baseline misses.

[![Process-curve finite metric space overview](docs/examples/assets/process-curve-external/hero-overview.svg)](docs/examples/process-curve-external-gallery.md)

- real UCR anomaly process windows as records
- metric-space query recovery: 16/16 queries match the expected role
- padded-vector baseline miss: 16/16 queries pick the wrong role
- reproducible native C++ evidence: CSV tables and SVG figures from one
  checked-in executable

Full evidence: [process-curve external gallery](docs/examples/process-curve-external-gallery.md).

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

When coordinates are useful, METRIC treats them as derived spaces. Diffusion-coordinate
geometry and parametric coordinate maps are used as ways to produce coordinate views
from an existing metric space, while the source metric remains explicit.

Docs:
[parametric diffusion coordinates pipeline](docs/examples/parametric-diffusion-coordinate-pipeline-workflow.md).

## C++ Quickstart

```cpp
#include <metric/workflow.hpp>

#include <string>
#include <vector>

namespace app {

using MachineRecord = mtrc::record::ComposedRecord<std::string, std::vector<double>, int>;

struct MachineMetric {
    mtrc::Edit<char> event_code{};
    mtrc::Wasserstein<double> spectrum{mtrc::Wasserstein<double>::on_line(4)};
    mtrc::DiscreteMetric<double> state{1.0};

    auto operator()(const MachineRecord& a, const MachineRecord& b) const -> double
    {
        return static_cast<double>(event_code(a.template field<0>(), b.template field<0>())) +
               spectrum(a.template field<1>(), b.template field<1>()) +
               state(a.template field<2>(), b.template field<2>());
    }
};

} // namespace app

namespace mtrc::core {

template <> struct metric_traits<::app::MachineMetric> {
    static constexpr auto law = metric_law::metric;
    static constexpr auto records = record_kind::structured;
    static constexpr bool thread_safe = true;

    static auto cache_key(const ::app::MachineMetric&) -> std::string
    {
        return "app::MachineMetric:edit+wass-line4+discrete";
    }
};

} // namespace mtrc::core

int main()
{
    const std::vector<std::string> event = {"pump_ok", "pump_warn", "valve_ok", "valve_warn"};
    const std::vector<std::vector<double>> spectrum = {
        {0.70, 0.20, 0.10, 0.00},
        {0.64, 0.24, 0.12, 0.00},
        {0.00, 0.10, 0.25, 0.65},
        {0.00, 0.08, 0.28, 0.64},
    };
    const std::vector<int> state = {0, 1, 0, 1};

    auto imported = mtrc::record::import_mixed_records(event, spectrum, state);
    auto space = mtrc::space::build_checked(imported.records, app::MachineMetric{});

    const auto query =
        mtrc::record::compose_record(std::string("valve_warning"),
                                     std::vector<double>{0.0, 0.08, 0.27, 0.65}, 1);
    auto nearest = mtrc::space::query::nearest(space, query);
    auto diagnosis = mtrc::stats::diagnose_space(space);
    auto representatives = mtrc::modify::represent::represent(space, 2);

    return nearest.id == space.id(3) && diagnosis.discoverable_metric &&
           representatives.size() == 2 ? 0 : 1;
}
```

This is a finite metric space over mixed records: symbolic event codes,
distribution-like sensor mass, and discrete machine state remain typed fields.
The application supplies one metric over record pairs; METRIC handles the finite
space, pair values, query, diagnostics, and representative reduction. No
coordinate artifact is required before METRIC can compute with the records.

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
steps after the TL;DR workflow:

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

To consume METRIC without a prior install, fetch it directly:

```cmake
include(FetchContent)
FetchContent_Declare(metric GIT_REPOSITORY https://github.com/metric-space-ai/metric.git GIT_TAG main)
FetchContent_MakeAvailable(metric)
target_link_libraries(program PRIVATE metric::metric)
```

## Python Binding

Python is an adapter layer over the native implementation, not a second METRIC
engine. Its job is to adapt Python records and callables to finite metric
spaces, expose native result objects, preserve lineage and provenance, and make
build-specific availability explicit.

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

`metric.available()` reports what the installed wheel supports. Use it, together
with the Python API documentation, for the exact support matrix of a given build:

```python
import metric

flags = metric.available()
print(flags["neighbors"])
print(flags["embed"])
```

The README describes the project model. Build-specific support and promotion
status live in the API and stability documents.

## Project Status And Scope

This README describes METRIC as a project: the object model, public vocabulary,
and intended computational surface. The exact promoted release surface is tracked
separately in:

- [Supported Surface](docs/supported-surface.md)
- [Stability Labels](docs/stability.md)
- [Production Readiness Roadmap](docs/engine/production-readiness-roadmap.md)
- [Changelog](CHANGELOG.md)

New capabilities should enter the public surface only after they have a finite
metric-space interpretation, native C++ implementation, deterministic tests or
examples, result metadata, and documentation.

METRIC is meant to support both open-ended work and shipped systems: exploratory
experiments, data-analysis notebooks, research prototypes, reproducible evidence
pipelines, and product code that needs metric-space search, diagnostics,
modification, or visualization. The visual pipeline is part of that framework:
C++ exports finite-space evidence, and METRIC-owned browser components inspect
that evidence without becoming a second algorithm implementation.

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
