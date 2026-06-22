# C++ API

The current C++ surface is native C++ and uses the `mtrc` namespace. Python and
other languages are adapter layers over this implementation.

Recommended includes:

```cpp
#include <metric/metric/catalog.hpp>
#include <metric/engine.hpp>
```

Use narrower headers when building a library boundary, for example
`<metric/record.hpp>`, `<metric/space.hpp>`, or specific headers under
`<metric/stats/...>`, `<metric/modify/...>`, and `<metric/space/...>`.

## Finite Metric Space

```cpp
#include <metric/metric/catalog.hpp>
#include <metric/engine.hpp>

#include <string>
#include <vector>

int main()
{
    std::vector<std::string> records = {"cat", "cot", "coat", "dog"};

    auto space = mtrc::make_space(records, mtrc::Edit<char>{});
    auto nearest = mtrc::stats::search::find_neighbors(
        space,
        std::string("cut"),
        mtrc::count{2});

    return nearest.empty() ? 1 : 0;
}
```

The records are not converted to vectors first. `Edit<char>` defines the metric
for this finite record set.

## Namespace Map

The source tree follows the Level-1 METRIC vocabulary:

| Namespace | Current source home | Purpose |
| --- | --- | --- |
| `mtrc::record` | `metric/record/...` | record IDs and record-set helpers |
| `mtrc::space` | `metric/space/...` | finite-space storage, indexing, selection, split/merge work |
| `mtrc::metric` | `metric/metric/...` | true metric catalog, custom metrics, composition, admission |
| `mtrc::stats` | `metric/stats/...` | questions about an existing finite metric space |
| `mtrc::modify` | `metric/modify/...` | derived or modified finite metric spaces |
| `mtrc::solve` | `metric/solve/...` | native C++ solvers used by metric-space computations |
| `mtrc::numeric` | `metric/numeric/...` | low-level numeric contracts; METRIC's native numerical core with historical source provenance documented centrally; no external numeric dependency/public boundary |

## Records And Spaces

`mtrc::RecordId` gives stable identity inside a finite space. A
`mtrc::MetricSpace<Record, Metric>` owns records plus the selected metric and
answers `space.distance(lhs_id, rhs_id)`.

```cpp
auto id = space.id(0);
auto d = space.distance(space.id(0), space.id(1));
space.insert(std::string("catalog"));
space.replace(id, std::string("cot"));
space.erase(id);
```

Changing records updates the space version. Storage and index objects built
from the old version can report stale state.

## Metrics

Catalog metrics live under `metric/metric/catalog/...` and are aggregated by
`<metric/metric/catalog.hpp>`. The public metric surface is for true metrics or
metrics admitted under documented domain constraints.

Current catalog groups include:

- vector metrics: Euclidean, Manhattan, Chebyshev, P-norm, Ruzicka-style metrics
- structured metrics: Edit, strict `Wasserstein` transport (with permissive `EMD` kept only as `metric_law::distance` compatibility), TWED
- distribution metrics: Cramer-von Mises, Kolmogorov-Smirnov, Random EMD (quarantined; not an admitted metric)
- space-related metrics: Riemannian distance helpers

Questionable or non-metric functions are not part of normal metric discovery.
Temporary compatibility code lives under `metric/metric/quarantine/...` until
it is admitted with proof and tests or removed from the metric surface.

Custom C++ metrics are callables accepted by `mtrc::make_space`. Algorithms
that require metric guarantees rely on the caller or the catalog admission
metadata for non-negativity, identity, symmetry, and the triangle inequality.

## Stats

`mtrc::stats` investigates an existing finite metric space.

| Namespace | Examples |
| --- | --- |
| `mtrc::stats::search` | nearest-neighbor and range queries |
| `mtrc::stats::sample` | sampling and metric-space walks |
| `mtrc::stats::properties` | entropy, intrinsic dimension, structure summaries |
| `mtrc::stats::correlate` | dependence or correlation between paired spaces; MGC is one implementation |
| `mtrc::stats::structural_analysis` | groups, clusters, outliers, structural representatives |

Search can run directly over the space or through an execution representation:

```cpp
auto exact = mtrc::stats::search::find_neighbors(space, query, mtrc::count{3});
auto indexed = mtrc::stats::search::find_neighbors(
    space,
    query,
    mtrc::count{3},
    mtrc::stats::search::cover_tree{});
```

## Space Storage And Indexes

Execution representations belong under `mtrc::space::storage` and keep the
source finite metric space authoritative:

- `LiveDistances<Space>`: lazy pairwise metric access
- `DistanceTable<Space>`: eager or lazy pairwise metric-value table
- `CoverTreeIndex<Space>`: exact metric index for admitted true metrics
- `KnnGraphIndex<Space>`: graph-backed neighbor index
- `GraphTopology<Space>`: explicit graph topology over source records

These objects trade memory, update cost, and query speed. They do not replace
the metric; they are derived execution forms.

## Modify

`mtrc::modify` creates derived or modified finite metric spaces.

Current homes include:

- `mtrc::modify::map`: coordinate maps, PCFA, fitted native maps, autoencoder
  mappings, KOC/SOM-style adapters
- `mtrc::modify::reduce`: representative-space and reduction work
- `mtrc::modify::resample`: denoise/resampling work
- `mtrc::modify::compose`: inspectable workflows such as the native PHATE-AE
  demonstrator

PHATE, autoencoders, and DNNs are not a separate ML framework inside METRIC.
They are implementations used to build fitted maps or demonstrator workflows
from an existing finite metric space.

MGC and entropy use the same rule from the stats side: entropy describes one
finite metric space, while MGC measures dependence between two aligned finite
metric spaces. Neither API creates a new record metric.

## Solve And Numeric

`mtrc::solve` owns native C++ solvers used by higher-level metric-space
operations. Current homes include:

- `mtrc::solve::parametric::dnn` for native fitted-function solvers
- `mtrc::solve::laplacian` for graph/Laplacian solver code

`mtrc::numeric` remains domain-neutral: scalar, vector, matrix, sparse,
linear-algebra, graph, random, parallel, and IO primitives. It must not own
record, metric, space, stats, or modify semantics.

## Error Types

`<metric/engine.hpp>` exports METRIC engine errors such as
`mtrc::InvalidRuntimePolicyError`, `mtrc::RepresentationError`, and
`mtrc::PipelineValidationError`. They remain compatible with standard C++
exception handling while preserving METRIC-specific failure causes.

## Compatibility Boundary

Some older compatibility names remain while examples and bindings are migrated.
Current documentation uses the `mtrc` namespace and the Level-1 source layout
above. Non-metric functions are not documented as metrics, and algorithmic work
belongs in native C++ rather than Python or another binding layer.
