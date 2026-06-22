# C++ API

The current C++ surface is native C++ and uses the `mtrc` namespace. Python and
other languages are adapter layers over this implementation.

Recommended includes:

```cpp
#include <metric/workflow.hpp>
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

    auto space = mtrc::space::build_checked(records, mtrc::Edit<char>{});
    auto nearest = mtrc::space::query::k_nearest(space, std::string("cut"), 2);

    return nearest.empty() ? 1 : 0;
}
```

The records are not converted to vectors first. `Edit<char>` defines the metric
for this finite record set.

## Practical Workflow

`<metric/workflow.hpp>` is the recommended application-level include. It
aggregates the Level-1 surfaces without moving ownership into a monolithic API.

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
    auto reduced = mtrc::modify::represent::represent(space, 2);

    return nearest.id == space.id(3) && diagnosis.discoverable_metric &&
           reduced.size() == 2 ? 0 : 1;
}
```

The practical workflow starts from typed records. Here one record contains a
symbolic event code, a distribution-like sensor spectrum, and a discrete machine
state. The metric is the single rule for comparing record pairs; the space then
provides pair values, search, diagnostics, and derived representative sets.

The workflow pieces stay separate:

| Step | Namespace | Meaning |
| --- | --- | --- |
| import and validate records | `mtrc::record` | bring finite recordings into C++ |
| choose/admit a true metric | `mtrc::metric` | metric catalog, custom metrics, admission status |
| construct and mutate spaces | `mtrc::space` | records plus one authoritative metric |
| read pair values and query | `mtrc::space::distances`, `mtrc::space::query` | ergonomic access over storage/search internals |
| inspect a space | `mtrc::stats` | profile, search, sampling, properties, correlation, structure |
| create derived spaces | `mtrc::modify` | represent, reduce, resample, map, dynamics, compose |

`mtrc::stats` never mutates the source space. `mtrc::modify` creates changed or
derived spaces and reports source lineage where the operation supports it.
`mtrc::stats::diagnose_space` is the compact starting point for inspecting one
space: it returns the profile, metric-law/admission status, one deterministic
neighbor check, and k-NN outlier scores without mutating the source space.

## Record Import And Mixed Records

`mtrc::record` owns the computer-side record shape before any metric is chosen.
For homogeneous vector-like records, `import_records_from_buffer` converts a
flat row-major binding buffer into C++ records. For mixed records,
`inspect_record_columns` and `validate_record_columns` check parallel field
columns before they are composed.

CSV and TSV helpers cover the common "file of numeric records" path without a
separate example parser:

```cpp
mtrc::CsvReadOptions options;
options.has_header = true;

auto records = mtrc::record::read_csv<double>("vectors.csv", options);
auto space = mtrc::space::build_checked(records, mtrc::Euclidean<double>{});

mtrc::CsvWriteOptions out;
out.header = {"x", "y", "z"};
mtrc::record::write_csv("vectors.normalized.csv", records, out);
```

The reader rejects ragged rows by default, reports parse failures with
line/column context, and rejects non-finite floating-point values unless
`require_finite` is disabled explicitly. Set `require_uniform_dimension = false`
when the record family is intentionally ragged.

```cpp
std::vector<std::string> name = {"pump", "valve"};
std::vector<std::vector<double>> spectrum = {{0.2, 0.8}, {0.7, 0.3}};
std::vector<int> state = {1, 2};

auto report = mtrc::record::inspect_record_columns(name, spectrum, state);
auto imported = mtrc::record::import_mixed_records(name, spectrum, state);

auto space = mtrc::space::build_checked(imported.records, composed_metric);
```

The returned `RecordColumnReport` gives row count, field count, per-field
sizes, and field-specific issues such as "field 2 has 3 records; expected 2".
This keeps record-shape failures readable at binding/UI boundaries. Metric
selection still happens later under `mtrc::metric`.

## Metric Discovery

`mtrc::metric::discover_metrics(record_kind)` returns a runtime catalog report
for the METRIC-provided metrics that match a record family. The status and law
fields are derived from the same `metric_admission` and `metric_traits` registry
used by metric-only algorithms, so discovery does not rely on class names.

```cpp
auto report = mtrc::metric::discover_metrics(mtrc::record_kind::sequence);

for (const auto& entry : report.entries) {
    if (entry.discoverable()) {
        // entry.name, entry.domain, entry.gate, entry.recoding
    }
}
```

By default the report lists discoverable true metrics only. Set
`discovery_options::include_quarantine` or `include_rejected` when an application
wants to explain why a shipped compatibility distance is not routed as a metric
and which alternative should be used.

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

The user-facing helpers under `mtrc::space` add a coherent workflow over a
finite metric space without exposing the low-level storage classes. They reuse
`mtrc::space::storage` and `mtrc::stats::search` and add no new algorithms.

| Component | Purpose |
| --- | --- |
| `mtrc::space::SpaceBuilder` / `build` / `build_checked` | safer staged/checked construction with opt-in validation |
| `mtrc::space::records` | batch insert/erase/replace, ID/position validation, `mutation_report` |
| `mtrc::space::distances` | `value` / `row` / `pairs` and lazy/materialized providers |
| `mtrc::space::query` | `nearest` / `k_nearest` / `within` (facade over `stats::search`) |
| `metric/space/lineage.hpp` (in `mtrc::space`) | sub-space/merge lineage lookup (`parent_record_id`, `merge_origin`) and `merge_checked` |
| `mtrc::space::cache` | stale detection (`is_stale`, `require_fresh`) and `rebuild` / `refresh` |
| `mtrc::space::persistence` | native finite-space artifacts with records, RecordIds, metric identity, version, and optional pair values |

```cpp
auto space = mtrc::space::SpaceBuilder<std::string, mtrc::Edit<char>>(mtrc::Edit<char>{})
                 .add_all(records).require_non_empty().require_true_metric().build();
mtrc::space::records::insert(space, more_records);          // stable RecordIds
auto near = mtrc::space::query::nearest(space, std::string("cut"));
auto row  = mtrc::space::distances::row(space, space.id(0));
```

These helpers are aggregated by `<metric/space/user_api.hpp>` and are also
available through `<metric/engine.hpp>`. See `docs/engine/metric-space.md` and
`docs/engine/representations.md` for the full workflow.

### Native Space Artifacts

`mtrc::space::persistence` saves a finite metric space as a native artifact:
records, stable `RecordId`s, the next unused RecordId, the space version, the
metric cache key, the metric law string, and optional materialized pair values.
The metric itself is supplied explicitly on load; arbitrary C++ callables are
not serialized as code.

```cpp
std::stringstream artifact;

mtrc::space::persistence::save(artifact, space);
auto loaded = mtrc::space::persistence::load<std::string>(artifact, mtrc::Edit<char>{});

assert(loaded.space.id(0) == space.id(0));
assert(loaded.space.version() == space.version());
```

The default `record_text_codec<T>` supports streamable records and preserves
`std::string` payloads verbatim. Applications with structured records can pass
a codec with `encode(record) -> std::string` and `decode(payload) -> Record`.
`verify_distances` checks that saved materialized pair values still agree with
the supplied metric, so an artifact cannot silently load against the wrong
metric implementation.

Derived-space lineage is preserved through the same namespace. `save` is
overloaded for `Subspace` and `MergedSpace`; `load_subspace` and
`load_merged_space` restore both the finite space and the existing lineage rows.
If a derived space was structurally changed after creation, export refuses the
stale lineage instead of writing a misleading artifact.

```cpp
auto sub = mtrc::space::select_subspace(space, {space.id(0), space.id(2)});

mtrc::space::persistence::save(artifact, sub);
auto loaded_sub = mtrc::space::persistence::load_subspace<std::string>(
    artifact, mtrc::Edit<char>{});

auto parent_id = mtrc::space::parent_record_id(loaded_sub.subspace,
                                               loaded_sub.subspace.space.id(0));
```

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

`mtrc::stats` investigates an existing finite metric space. It inspects, ranks,
samples, compares, and describes a space; it never defines a metric, never
mutates the source space, and never creates a derived space (those are
`mtrc::space` and `mtrc::modify`). The five Level-2 components are user
workflows over one (or, for `correlate`, two paired) finite metric spaces.
Vector records are one convenient special case, not the conceptual basis: every
stats workflow is computed from metric values, not coordinates.

| Namespace | Examples |
| --- | --- |
| `mtrc::stats::search` | nearest-neighbor, range, and batch queries (`knn`, `range`, `knn_batch`, `range_batch`, `find_neighbors`) |
| `mtrc::stats::sample` | deterministic sampling and metric-space walks (`regular_sample`, `farthest_first`, `metric_walk`) |
| `mtrc::stats::properties` | `profile`, distance-value distribution, entropy, intrinsic dimension, local-volume, structure summaries |
| `mtrc::stats::correlate` | dependence/correlation between paired spaces and its significance; MGC is one implementation |
| `mtrc::stats::structural_analysis` | groups, clusters, outliers, representatives, and cluster validity diagnostics |

`mtrc::stats::properties::profile(space)` returns one cohesive `StatsProfile`:
record count, pair count, zero-distance pair count, the metric-value range and
average, the intrinsic (expansion) dimension, and clear status flags. Optional
distance-distribution and local-volume sections are computed only when requested
through `profile_options`, so `profile` has no hidden expensive behavior. The
`distance_distribution` diagnostic reports quantiles and a histogram over the
pairwise metric values and rejects non-finite values (a NaN has no defined
order). Entropy is a property of one space: a differential-entropy estimate whose
`EntropyResult` carries an explicit `status` (`valid` -- including a valid
negative value -- `too_few_records`, `degenerate`, or `estimator_failure`) and
the effective (clamped) neighbor count and approximation order. Intrinsic dimension is a finite-space
growth diagnostic, not a manifold-dimension guarantee.

Search can run directly over the space, in batches, or through an execution
representation; every result carries exactness and representation metadata and
breaks ties by `RecordId`:

```cpp
auto exact = mtrc::stats::search::find_neighbors(space, query, mtrc::count{3});
auto indexed = mtrc::stats::search::find_neighbors(
    space, query, mtrc::count{3}, mtrc::stats::search::cover_tree{});
auto close = mtrc::stats::search::range(
    space, space.id(0), radius, mtrc::stats::search::distance_table{});
auto batched = mtrc::stats::search::knn_batch(space, queries, 3); // one result per query
```

Range queries support the same explicit execution vocabulary as nearest-neighbor
queries where the representation can answer the request exactly:
`exact_scan`, `brute_force`, `distance_table` for `RecordId` queries,
`cover_tree`, `knn_graph`, and `mtrc::space::storage::policy`. A kNN graph range
query by `RecordId` is accepted only when the graph contains every possible
neighbor; otherwise it throws instead of returning an incomplete radius result.

`mtrc::stats::correlate` tests dependence between two paired finite metric
spaces. MGC is a dependence statistic in `[-1, 1]`, **not a metric or distance**.
`mgc_significance` turns the point statistic into an upper-tail p-value via a
seeded permutation test (an explicit, reproducible result with statistic,
p-value, permutation count, and seed); spaces can be aligned by position or, with
`mgc_by_record_id`, by shared record identity with explicit dropped-pair
reporting. `mtrc::stats::structural_analysis` adds `cluster_diagnostics`
(silhouette and intra/inter-cluster distances, well-defined for any metric space)
and `nearest_neighbor_outliers` (a k-NN distance isolation score) alongside the
existing DBSCAN, k-medoids, affinity-propagation, outlier, and representative
implementations.

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

Lazy `DistanceTable` instances are memoizing snapshots, not live views. After a
space mutation they throw `StaleRepresentationError` on reads until rebuilt, so
cached old values and freshly computed new-version values cannot be mixed.

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

Each Level-2 numeric area has a curated facade header so a caller can include
only the contract surface it needs:

```cpp
#include <metric/numeric/matrix.hpp>          // dense matrices, adaptors, views, reductions
#include <metric/numeric/linear_algebra.hpp>  // inv/det/llh/eigen/svd/solve (LAPACK optional)
#include <metric/numeric/sparse.hpp>          // CompressedMatrix / CompressedVector
```

The full set is `scalar`, `vector`, `matrix`, `sparse`, `linear_algebra`,
`graph`, `random`, `parallel`, and `io`; `<metric/numeric.hpp>` remains the full
umbrella. Default inclusion is header-only and dependency-free — BLAS is off by
default and LAPACK-backed routines are optional execution paths that only require
a link when called. See [Numeric promoted contracts](numeric-contracts.md) for
the full per-area map and stability classification.

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
