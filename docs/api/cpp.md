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

## Redif Metric Dynamics

Redif treats noise as dynamics over atom measures of a finite metric space. The
public C++ surface lives in `<metric/engine.hpp>` and
`<metric/modify/dynamics/redif.hpp>`:

```cpp
mtrc::redif_options options;
options.neighbors = 2;
options.iterations = 4;
options.euler_step = 0.25;
options.adaptive_geometry = true;
options.scale_policy = mtrc::redif_scale_policy::mean_local_distance;

auto removed = mtrc::redif_remove_noise(space, options);
auto added = mtrc::redif_add_noise(space, options);
auto ranked = mtrc::redif_outliers(space, options);
```

`redif_remove_noise` returns inverse metric dynamics as measure paths over the
original atoms. `redif_add_noise` runs the forward metric-induced dynamics over
the same operator family. The result carries inspectable stationarity, operator
diagnostics, entropy diagnostics, per-step stability, exact transport metadata,
and compact path summaries.
`RedifOperatorDiagnostics` includes degree/scale/affinity ranges, connected
component count, reversibility, transition row-sum bounds, and
`spectral_gap_proxy="minimum_transition_escape_probability"` with its numeric
proxy value. This proxy is an audit signal, not an exact eigenvalue gap.

For adapter layers that already own a distance matrix, use
`redif_remove_noise_from_distance_matrix(...)`,
`redif_add_noise_from_distance_matrix(...)`, and
`redif_operator_from_distance_matrix(...)`. They are still exact dense Redif
routes and honor the same dense-budget and stability options.

For exact local-relation providers, use
`redif_sparse_operator_from_exact_local_relation(...)` or
`redif_sparse_operator_from_exact_neighbor_provider(...)`. The provider must
return exactly the directed `k` local relation that dense Redif would derive.
`RedifSparseOperator` stores sparse affinity/transition rows and can be converted
with `redif_operator_from_sparse_operator(...)` for dense-reference parity tests.
`RedifLocalRelationDiagnostics` and `RedifOperatorDiagnostics` report relation
representation, exactness, relation entry counts, and relation distance
evaluations.

For explicitly non-exact bounded candidate relations,
`redif_remove_noise_from_sampled_distance_matrix(...)` accepts
`redif_sampled_relation_options`. Its result is marked non-exact when the
candidate set is smaller than the finite space; diagnostics report candidate
count, candidate universe, candidate fraction, chunk size, chunk count, and
local relation distance evaluations. This path approximates only the local
relation.

Transport path work is governed by `max_transport_problems`. Optional support
truncation requires `allow_transport_support_truncation=true` plus
`max_transport_support_atoms` or `transport_support_mass_floor`; when positive
mass is discarded, `RedifTransportDiagnostics` marks the path non-exact and
reports discarded mass.

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
wants to explain why a quarantined distance is not routed as a metric
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
| `mtrc::numeric` | `metric/numeric/...` | low-level numeric contracts; METRIC's native numerical core with historical source provenance documented centrally; no external numeric dependency boundary |

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

auto save_options = mtrc::space::persistence::artifact_options{};
save_options.max_materialized_pair_distances = 10000; // explicit budget for saved pair values

mtrc::space::persistence::save(artifact, space, save_options);
auto loaded = mtrc::space::persistence::load<std::string>(artifact, mtrc::Edit<char>{});

assert(loaded.space.id(0) == space.id(0));
assert(loaded.space.version() == space.version());
```

The default `record_text_codec<T>` supports streamable records and preserves
`std::string` payloads verbatim. Applications with structured records can pass
a codec with `encode(record) -> std::string` and `decode(payload) -> Record`.
Materialized pair values are included by default only under
`artifact_options::max_materialized_pair_distances`. Set
`include_pair_distances=false` for record/id/version-only artifacts, or set
`allow_unbounded_pair_distances=true` only when a full all-pairs snapshot is an
intentional, capacity-planned export.
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
auto sub_save_options = mtrc::space::persistence::artifact_options{};
sub_save_options.max_materialized_pair_distances = 10000;

mtrc::space::persistence::save(artifact, sub, sub_save_options);
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
- structured metrics: Edit, strict `Wasserstein` transport (with permissive `EMD` kept only as a non-admitted `metric_law::distance` route), TWED
- distribution metrics: Cramer-von Mises, Kolmogorov-Smirnov, Random EMD (quarantined; not an admitted metric)
- space-related metrics: Riemannian distance helpers

Questionable or non-metric functions are not part of normal metric discovery.
Temporary quarantine code lives under `metric/metric/quarantine/...` until
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
distance-only stats workflow is computed from metric values, not coordinates.
Coordinate-space estimators are explicit pipeline consumers: first map or embed
the source space, then run the coordinate operator on the derived space. Those
operators also require an admitted coordinate-neighborhood metric family through
`CoordinateMetricLike`; numeric/indexable records plus an arbitrary
`metric_law::metric` are not enough.

| Namespace | Examples |
| --- | --- |
| `mtrc::stats::search` | nearest-neighbor, range, and batch queries (`knn`, `range`, `knn_batch`, `range_batch`, `find_neighbors`) |
| `mtrc::stats::sample` | deterministic sampling and metric-space walks (`regular_sample`, `farthest_first`, `metric_walk`) |
| `mtrc::stats::properties` | `profile`, distance-value distribution, coordinate-space entropy, intrinsic dimension, local-volume, structure summaries |
| `mtrc::stats::correlate` | dependence/correlation between paired spaces and its significance; MGC is one implementation |
| `mtrc::stats::structural_analysis` | groups, clusters, outliers, representatives, and cluster validity diagnostics |

`mtrc::stats::properties::profile(space)` returns one cohesive `StatsProfile`:
record count, pair count, zero-distance pair count, the metric-value range and
average, the intrinsic (expansion) dimension, and clear status flags. Optional
distance-distribution and local-volume sections are computed only when requested
through `profile_options`, so `profile` has no hidden expensive behavior. The
`distance_distribution` diagnostic reports quantiles and a histogram over the
pairwise metric values and rejects non-finite values (a NaN has no defined
order). Entropy is the coordinate-space exception in this group: the current kpN
estimator requires an embedded coordinate space or mapping result with an
admitted coordinate-neighborhood metric because it uses metric neighborhoods
plus local Gaussian coordinate volumes. Its
`EntropyResult` carries an explicit `status` (`valid` -- including a valid
negative value -- `too_few_records`, `degenerate`, or `estimator_failure`) and
the effective (clamped) neighbor count and approximation order. Intrinsic
dimension is a finite-space growth diagnostic, not a manifold-dimension
guarantee.

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
`mtrc::space::storage::using_knn_graph(...)` is preflighted as an index build:
`estimate_cost` charges the full directed `n * (n - 1)` build work plus query
work before constructing `KnnGraphIndex`, so low `max_distance_evaluations`
budgets refuse before metric calls.
For large metric search where an exact scan or dense table is not acceptable,
`mtrc::space::storage::using_landmark_index(candidate_limit,
mtrc::space::storage::approximate())` builds an explicit non-exact
Landmark/Pivot provider, reports `landmark_index`, stores only `O(n * p)`
landmark distances, and refines a bounded candidate set with the exact metric.
Search diagnostics report whether bounded recall calibration ran. When it runs,
`approximation_quality` includes the holdout query count, bounded
reference-candidate evaluation count, matched/reference counts, and recall
estimate with `standard_error` and `confidence_radius_95`; otherwise `reason`
says why calibration was skipped, for example an empty request or
`max_distance_evaluations` guard.
For admitted metric or pseudo-metric spaces, multi-query approximate planning
and `space::execution_context(space, mtrc::space::storage::approximate())`
can select the same Landmark/Pivot provider automatically for repeated search.
Automatic Landmark planning derives conservative landmark/candidate defaults
from `n`, `query_count`, and evaluation budget, compares the Landmark estimate
against sampled search, and leaves explicit `using_landmark_index(...)`
candidate limits unchanged.
For exact workflows that must avoid resident dense all-pairs storage, a
budgeted distance-table policy can opt into blocked execution and explicit
disk spill:

```cpp
mtrc::space::storage::resource_budget budget;
budget.max_memory_bytes = 64 * 1024 * 1024;
budget.allow_chunking = true;
budget.allow_out_of_core_spill = true;
auto policy = mtrc::space::storage::with_resource_budget(
    mtrc::space::storage::using_distance_table(), budget);
auto table = mtrc::space::storage::make_blocked_distance_table(space, policy);
```

The default blocked provider is memory-only. With `allow_out_of_core_spill`,
evicted LRU blocks are persisted and later reloaded without re-running the
metric; results remain exact, while the planner still refuses unsafe dense
materialization unless the caller opts into bounded block execution.
`estimate_cost` and `diagnostics_for_space` report the out-of-core plan before
execution, including `spill_enabled`, `spill_block_count`,
`planned_spill_blocks`, `max_resident_blocks`, resident/spill byte estimates,
and whether explicit spill policy was required.
For materialized search/range that may trade exactness for bounded work, enable
both chunking and approximate fallback. The planner reports `chunked_space_view`,
the result is marked `exact=false`, and the operator runs bounded chunk
refinement instead of constructing a dense table. `RecordId` queries refine
representative-selected chunk pairs; free query objects rank chunk
representatives and refine only selected chunks:

```cpp
mtrc::space::storage::resource_budget search_budget;
search_budget.max_dense_records = 1024;
search_budget.allow_chunking = true;
search_budget.allow_approximate = true;
auto search_policy = mtrc::space::storage::with_resource_budget(
    mtrc::space::storage::using_distance_table(), search_budget);
auto plan = mtrc::space::storage::estimate_cost(space, "neighbors", search_policy);
auto neighbors = mtrc::find_neighbors(space, space.id(0), 10, search_policy);
```

Exact-only policies that allow chunking continue to use the blocked exact
provider; the planner does not silently return approximate results for an exact
contract.
Batch search is a thin adapter over the single-query path: source spaces support
record-value and `RecordId` batches, records+metric convenience inputs report
`"records"` representation, pairwise providers support `RecordId` batches, and
mapping results support source-`RecordId` batches while preserving mapped-space
provenance and per-neighbor source lineage. A `MappingResult` is a lineage
artifact, not a mapping-artifact handle: raw source-record out-of-sample mapped
search is intentionally not a public overload.

`mtrc::stats::correlate` tests dependence between two paired finite metric
spaces. MGC is a dependence statistic in `[-1, 1]`, **not a metric or distance**.
The C++ public surface accepts spaces, pairwise providers, and records+metric
convenience inputs for the aligned-by-position MGC statistic through
`mgc(...)`, `compare(...)`, and `correlate(...)`. `mgc_significance` turns the
point statistic into an upper-tail p-value via a seeded permutation test (an
explicit, reproducible result with statistic, p-value, permutation count, and
seed) over the same space/provider/records+metric domains. Large default
`compare(space, space)` / `correlate(space, space)` calls preflight dense MGC
work and use the bounded sampled estimate route when the exact matrix would
exceed the default dense budget. `mgc_significance(space, space)` requires dense
exact pairwise distances for its permutation p-value and therefore refuses
oversized `MetricSpace` inputs before metric calls; pass an explicit provider or
lower the record count when exact significance is intended. Spaces can also be
aligned by shared record identity with `mgc_by_record_id`, which reports
explicit dropped-pair counts. `mtrc::stats::structural_analysis` adds `cluster_diagnostics`
(silhouette and intra/inter-cluster distances, well-defined for any metric space)
and `nearest_neighbor_outliers` (a k-NN distance isolation score) alongside the
existing DBSCAN, k-medoids, affinity-propagation, outlier, and representative
implementations.

`mtrc::space::distances::pairs(...)` materializes unordered pair values and has a
default pair-count guard before allocation or metric evaluation. Use
`for_each_pair(...)` for streaming large pair traversals, or pass
`pair_collection_options{0}` only for an intentional unbounded in-memory
collection.
The explicit dense-matrix helpers `FiniteSpace::pairwise_distances(...)` and
`mtrc::space::index::pairwise_distance_matrix(...)` similarly guard their
materialized `n x n` matrix output with `pairwise_matrix_options`; pass
`pairwise_matrix_options{0}` only when an unbounded dense matrix is intentional.
Exact graph helpers are also budgeted before metric evaluation:
`exact_knn_graph(...)`, `exact_knn_graph_edges(...)`,
`exact_radius_graph(...)`, and `exact_radius_graph_edges(...)` guard exhaustive
directed `n * (n - 1)` metric work with `exact_graph_options`. Pass
`exact_graph_options{0}` only for intentional unbounded exact construction.
`graph_stretch_diagnostics(...)` uses `graph_stretch_options` to guard the
direct metric-pair comparisons, dense shortest-path matrix cells, and
all-pairs shortest-path closure estimate before allocating or invoking the
metric; `graph_stretch_options{0, 0, 0}` is the explicit unbounded opt-in.
The lower-level storage conversion helpers
`provider_dense_distance_matrix(...)`, `provider_symmetric_distance_matrix(...)`,
and `metric_space_dense_distance_matrix(...)` use
`dense_distance_matrix_options` for the same reason.

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

- `mtrc::modify::map`: coordinate maps, PCFA, derived native maps, and
  parametric coordinate maps
- `mtrc::modify::reduce`: representative-space and reduction work
- `mtrc::modify::resample`: density-filter/resampling work
- `mtrc::modify::compose`: inspectable workflows such as the native parametric diffusion coordinate
  demonstrator

`CompressionResult` reports source lineage, source-to-representative
assignments, nearest-representative distances, representative multiplicities,
and normalized representative weights. The retained records remain source
records, so promoted compression is a weighted finite metric-measure summary,
not a vector-centroid reduction. Count-based compression accepts
`space::select::coverage` and `space::select::k_center` as explicit aliases for
the promoted coverage objective, while `space::select::radius_coverage{radius}`
chooses the representative count from a metric radius. `k_medoids_options`
selects source medoids for average-assignment compression.

`modify::resample::thin(space, count, preserve_distribution{...})` and the
`distribution_sample` alias perform deterministic distribution-preserving
thinning by regular source-order sampling. They return `MappingResult` lineage
over retained source records and preserve the source metric on the subset.
`thin(space, uniform_density{radius})` and `uniform_density_sample` perform the
metric-only uniform-density route as a maximal radius net: retained records are
source records, radius-separated, and radius-cover the original finite space,
so empirical density is intentionally flattened. The result diagnostics report
the requested radius, coverage radius, average assignment distance, before/after
average nearest-neighbor distance, local-density drift, and before/after local
volume count and density at the requested radius.
`equalize(space, uniform_density{radius})` exposes the same metric-only
construction as the density-normalization intent and returns a
`MappingResult` with `mapping="equalize"`.

parametric diffusion coordinates and native coordinate solvers are not a separate algorithm collection inside METRIC.
They are implementations used to build derived maps or demonstrator workflows
from an existing finite metric space. diffusion-coordinate target/diffusion helpers now carry
a conservative `max_dense_records` default and refuse before building dense
distance, transition, or probability matrices when that default would be
exceeded; set the limit explicitly only when dense work is intended.

MGC and entropy use the same rule from the stats side: entropy describes one
finite metric space, while MGC measures dependence between two aligned finite
metric spaces. Neither API creates a new record metric.

## Solve And Numeric

`mtrc::solve` owns native C++ solvers used by higher-level metric-space
operations. Current homes include:

- `mtrc::solve::parametric::dnn` for native calibrated-function solvers
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
`mtrc::PipelineValidationError`. They use standard C++
exception handling while preserving METRIC-specific failure causes.

## Quarantine Boundary

Some older adapter names remain while examples and bindings are migrated.
Current documentation uses the `mtrc` namespace and the Level-1 source layout
above. Non-metric functions are not documented as metrics, and algorithmic work
belongs in native C++ rather than Python or another binding layer.
