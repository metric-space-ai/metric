# Scale-Safe Performance Plan

METRIC should remove the operational pain of finite metric spaces. When a caller
hands it a large data set, it must not blindly allocate an `O(n^2)` table or enter
an unbounded `O(n^3)` diagnostic path. The runtime should choose a safe exact
plan when possible, downgrade to an explicit approximation when needed, and fail
early with a useful diagnostic only when no safe plan exists.

This plan targets the promoted `mtrc::space`, `mtrc::stats`, and `mtrc::modify`
surface. Legacy containers can be optimized opportunistically, but the promoted
finite-space API is the priority.

## Current Implementation Status (2026-06-26)

The first implementation waves have landed the crash-prevention guardrails for
the promoted finite-space surface. Treat the status below as the working ledger:
bounded sampled/blocking paths are in place, and C++ search now has an explicit
Landmark/Pivot approximation that builds in `O(n * p)` instead of all-pairs.
Python now has an initial automatic Landmark route for repeated/batch
approximate source-metric search, and blocked exact storage has an explicit
disk-spill mode for evicted blocks with an optional hard spill-byte quota.
Python `RuntimePolicy` now also carries memory, distance-evaluation, and dense
record budgets through `Space.plan(...)`, dense matrix export, exact query,
describe, compare/correlate, intrinsic-dimension, and direct
pairwise/search/diagnostic helper paths. Clustering, outlier, representative,
reduce, and compress Python adapters now preflight the same budgets before
native exact metric work, including medoid/separated/coverage convenience helpers
and uniform-density resample diagnostics. Accepted Python radius-compression and
uniform-density budgets now account for follow-on assignment/diagnostic phases,
and derived Python spaces are built with `validate="none"` and `cache="none"` so
post-operation construction does not bypass the caller's budget.
Python parametric-diffusion mapping adapters now carry `RuntimePolicy` and
`max_*` budgets into dense target-construction preflights before native binding
calls, native parametric-diffusion bindings/specs now enforce dense-record,
memory, and distance-evaluation budgets before dense target construction, and
precomputed `intrinsic_dimension_from_distances(...)` input is guarded by
dense-record and memory budgets before `list(...)` materialization, and each
precomputed matrix row is now materialized under the same bounded square-matrix
contract before native conversion can exhaust row generators.
Python parametric-diffusion artifact `transform(...)`/`inverse_transform(...)`
and mapping-pipeline `neighbor_recall(...)` methods now also accept the same
runtime/max-budget arguments and preflight unsized/batched native work before
delegating to artifact bindings.
The Python native-binding compatibility retry now removes only one unsupported
optional budget keyword at a time and does not hide native validation
`TypeError`s that mention budget fields.
Python clustered-space mapping now preflights the cluster representative
distance matrix with the caller's runtime/memory/distance/dense-record budgets
before evaluating source-space distances or materializing clustering assignment
iterators, and batch representative resolution uses indexed SourceId lookup
instead of one linear `ids.index(...)` scan per cluster. Assignment and medoid
sequences are now bounded by `record_count + 1` and `cluster_count + 1`
sentinels, so arbitrary result-like iterators cannot be exhausted before
validation fails. Direct deterministic
Python mapping through `Space.map(transform=...)`
and `metric.operators.map_space(...)` now bounds record/source-id
materialization with the same runtime/dense-record budget before invoking the
caller transform.
Research-only Python experimental diagnostics now accept `RuntimePolicy`/`max_*`
budgets, bound iterator materialization by `max_dense_records`, refuse
over-budget exact Python loops before the first metric call, and reuse
distance-distribution drift pair distances instead of recomputing the same
all-pairs samples twice. Distance-distribution prototypes now generate pair
ordinals lazily, translate ordinals in logarithmic time, and preflight the
result-value memory budget before metric calls. Radius-parameter iterators for hierarchical
experimental diagnostics are now bounded before exact work planning proceeds.
`metric_graph_spanner(...)` now preflights pair-list materialization memory and
includes its follow-on all-pairs shortest-path checks in the same work budget
before metric calls start.
`metric_measure_drift(...)` now also bounds source/target weight iterator
materialization to the already admitted record count plus one mismatch sentinel,
so oversized weight generators are not exhausted before validation fails.
Direct Python dense/native helper paths now also bound unsized iterator
materialization before `list(...)` for pairwise matrix export, DBSCAN/outlier and
density-filter helpers, k-medoids, farthest-first representatives,
representative/reduce/compress helpers, uniform-density resampling,
regular distribution-preserving thinning, radius-coverage compression, medoid,
separated-representative, coverage-representative, exact compare/correlate,
intrinsic-dimension, describe-structure, and exact live neighbor/range helpers.
Direct approximate Python `nearest_neighbors(..., exact=False)` and
`range_neighbors(..., exact=False)` also inherit the default dense-record
materialization cap when the caller provides only a distance budget. Python
low-level `metric.correlation.MGC`, `MGC_direct`, and `Entropy` native adapters
now accept `RuntimePolicy`/`max_*` budgets, materialize unsized inputs under the
same dense-record cap, and refuse over-budget exact dense work before entering
the native binding.
explicit TreeIndex facades no longer copy `space.records` during construction,
and native-only GraphIndex construction now refuses before touching source records.
Explicit Python Landmark/Pivot search (`exact=False, representation="landmark"`)
now enforces the same `max_memory_bytes` preflight as automatic Landmark routing:
over-budget `O(n * p)` landmark-distance storage refuses at planning time before
the `_PythonLandmarkIndex` build or any metric call.
Python `Space(...)`/`FiniteMetricSpace(...)` construction now also bounds unsized
record iterator materialization with `max_dense_records` before validation,
cache construction, ID indexing, or metric calls.
Python `Space.neighbors(...)`/`knn(...)`/`range(...)`/`within_radius(...)` now
also budget-check unsized batch query iterators incrementally instead of
materializing all queries before the `query_count * record_count` plan is known,
and approximative query batches are capped by the same dense-record
materialization budget before any metric calls. `Space.pairwise(ids=...)` now
checks dense pairwise budgets while materializing unsized ID iterators, before
building the RecordId-position list. Direct Python `compare_spaces(...)` now
also materializes caller-supplied `matched_ids`, `dropped_left_ids`, and
`dropped_right_ids` under the dense-record budget before invoking the native
correlation binding. Python `Space.from_csv(...)`, `Space.from_tsv(...)`,
`Space.from_numpy_file(...)`, and `Space.from_dataframe(...)` now preflight the
source row count against `max_dense_records` before native file import,
`array.tolist()`, or `dataframe.to_dict(...)` can materialize oversized inputs.
Python `Space.to_csv(...)` and `Space.to_tsv(...)` now also bound per-record
field and header materialization before handing rows to the native writer, so a
single unsized row/header iterator cannot hang export.
Legacy C++ `MatrixSpace`/`FiniteSpace` and `GraphSpace` entry points are now
preserved as explicit exact representations with safe default build budgets,
while `space::index` direct helpers no longer route through a hidden eager
`MatrixSpace` build.
Direct C++ `SymmetricDistanceTable` construction now has safe default
dense-record, memory, and distance-evaluation budgets, with zero budgets kept as
the explicit unbounded opt-in; the `runtime_guard` overload preserves those safe
defaults instead of treating cancellation-only construction as an unbounded opt-in.
Policy-driven C++ `execution_context` materialized shared providers now preflight
distance-evaluation budgets before building symmetric or dense distance tables,
with exact blocked fallback still available when chunking is explicitly allowed.
Policy-driven C++ live `describe_structure(space, policy)` now also preflights
the exact describe plan before pair-summary/intrinsic loops, so low
distance-evaluation budgets refuse before metric calls even without
materialization.
Finite metric dynamics (`metric_transition`) and Redif measure dynamics now add
memory and source-distance-evaluation budgets to their dense matrix guards, so
low budgets refuse before constructing the distance matrix or calling the metric.
Redif uses a conservative multi-matrix memory estimate for retained frames and
operator work buffers. Direct coordinate diffusion through
`modify::dynamics::diffuse(...)` now applies the same memory and `n * n`
distance-evaluation guard before dense distance-matrix conversion.
Direct C++ `CoverTreeIndex` construction and `storage::cover_tree(...)` now have
safe default build-distance and snapshot-memory budgets, and policy-driven
`using_cover_tree(...)` search/range planning uses a conservative worst-case
build estimate before constructing the index.
Free-query C++ `knn_batch(...)` and `range_batch(...)` now expose budgeted
RuntimePolicy overloads that preflight `query_count * record_count` before the
first metric call, including container convenience wrappers. C++ RecordId batch
search on metric spaces now exposes the same budgeted overloads, so repeated
in-space query IDs no longer bypass the batch preflight.
C++ `space::query::try_nearest(...)` and `nearest(...)` now also accept explicit
search strategies/runtime policies, so ergonomic repeated free-query code can use
the same budget gates as `k_nearest(...)`.
Mapped C++ `knn_batch(mapping, ...)` and `range_batch(mapping, ...)` now build a
per-batch SourceId-to-target-position index instead of scanning all mapping
lineage rows for every query.
Those mapped batch paths also expose RuntimePolicy overloads and preflight
`query_count * mapped_record_count` before mapped-space metric calls.
Direct C++ `KnnGraphIndex::refresh_after_append(...)` now preflights append
refresh distance work before mutating the existing graph snapshot, with a safe
default refresh-evaluation budget and zero as the explicit unbounded opt-in.
Direct C++ `LandmarkIndex` construction now has safe default build-distance and
projection-memory budgets, while policy-driven Landmark search forwards explicit
runtime budgets into the temporary or shared Landmark provider.
C++ exact-lazy `compress(..., policy)` no longer bypasses `execution_plan`:
count-based, radius-coverage, and k-medoids compression preflight live exact
work before constructing providers or calling the metric, and the live
compression estimate is no longer the previous `O(n)` fallback.
Materialized search/range, for both
`RecordId` and free query objects, can now dispatch bounded `chunked_space_view`
refinement when both chunking and approximate fallback are allowed.
Direct provider-level C++ affinity propagation now refuses oversized dense
message-passing matrices before allocation and before the first provider
distance call, closing the remaining low-level clustering path that could build
multiple `n x n` matrices even when the caller supplied a provider directly.
Direct provider-level C++ `kmedoids(provider, ...)` and `dbscan(provider, ...)`
now preflight exact clustering record/evaluation work before initialization,
region queries, medoid recomputation, or any provider distance call.
Direct provider-level C++ DBSCAN outlier detection and
`nearest_neighbor_outliers(provider, ...)` now refuse oversized exact provider
work before DBSCAN region queries, nearest-neighbor scoring, or any provider
distance call. Explicit `MetricSpace` runtime-policy paths remain policy-budgeted.
Direct provider-level C++ representative helpers now refuse oversized exact
provider work before distance calls for farthest-first/coverage/k-center
selection, radius coverage, and medoid selection. Exact-live runtime-policy
planning now charges `record_count * requested_count` representative work, and
the provider guard enforces the caller's distance budget on the final live path.
`stats::structural_analysis::representative_indices(...)` now includes the
follow-on sample-assignment pass in its exact budget and forwards the accepted
budget into `stats::sample::farthest_first(...)`. Sampled high-count
representative fallback keeps a set of selected/candidate RecordIds, avoiding
the previous `O(count^2)` duplicate checks.
Exact-live C++ uniform-density `thin/equalize(..., policy)` now charges a
conservative multi-phase resample estimate before constructing the live provider:
radius-net selection, full-source assignment, source/target nearest-neighbor
diagnostics, and source/target local-volume diagnostics are covered by the
`max_distance_evaluations` gate. The high-level default exact path keeps the
existing small-input threshold, while explicit runtime policies refuse
over-budget resampling before the first metric call.
Promoted C++ `cluster_diagnostics(...)` now keeps small inputs exact, samples
oversized diagnostics by default with non-exact metadata, and exact-only options
refuse before metric calls.
Direct C++ sampling helpers (`farthest_first`, `metric_walk`, and
`regular_sample`) now preflight their expected `record_count * sample_count`
distance work before metric calls. `farthest_first_options` and
`metric_walk_options` expose an explicit distance-evaluation budget, while
`regular_sample` has the same safe default guard for its lineage assignment pass.
Direct C++ `stats::correlate::mgc(...)` exact entry points now refuse oversized
record-container and `MetricSpace` inputs before the first metric call, and
full-sample `mgc_estimate(...)` cannot accidentally turn a large estimate into
an unbounded exact dense MGC run. The default large `compare(...)`/`correlate(...)`
path remains the sampled fallback, while explicit exact MGC reports a clear
`mgc_estimate(...)`/`compare(...)` fallback hint instead of crashing.
Legacy direct C++ `mtrc::MGC`, `MGC_direct`, their public dense scratch helpers,
`xcorr`, and the free `distance_matrix(...)` helper now also have safe default
resource options for exact dense work. `mgc_resource_options{0, 0, 0}` remains
the explicit unbounded opt-in for callers intentionally using the old low-level
surface.
MGC permutation significance now also preflights `permutations * n * n` dense
permutation matrix work before constructing distance matrices or entering the
permutation loop, with `max_permutation_matrix_cells=0` kept as the explicit
unbounded opt-in for callers that intentionally request very large Monte-Carlo
runs.
Promoted C++ `stats::properties::entropy(space)` now has scale-safe defaults:
small coordinate metric spaces stay exact, while oversized defaults evaluate a
deterministic sampled entropy, mark the result non-exact, and report sample
metadata. Exact-only entropy options refuse before metric calls when the record
count exceeds the exact default.
Legacy direct C++ `Entropy`, `EntropySimple`, `VMixing`, and `VMixing_simple`
now expose safe default `entropy_resource_options`, refuse oversized tree/list
and sample work before metric calls or combined-list construction, and reject
`sampleSize == 0` before the old estimator helpers can divide by zero.
Promoted C++ `local_volume_profile(space, radii)` now uses the same bounded
deterministic per-source target sampling as `local_volume(space, radius)` for
oversized defaults, reusing one sampled distance pass across all requested radii
instead of scanning `n * n` directed pairs per call.
Direct C++ provider-level `local_volume(provider, radius)` now also routes
through the same sampled default instead of always taking the exact `n * n`
path, and exact-only provider options for `local_volume`/`local_volume_profile`
refuse oversized inputs before any provider distance call unless the caller
explicitly opts into unbounded exact work.
Multi-radius local-volume profile helpers now also cap their `record_count *
radii_count` scratch storage. The exact profile path and the chunked profile path
refuse over-budget helper matrices before allocation or pair traversal, while
`max_profile_cells=0` remains the explicit unbounded opt-in.
C++ clustered-space derivation now preflights the representative distance table
before calling the source metric: `ClusteredSpaceOptions` caps representative
count, table memory, and `representatives * representatives` metric calls, with
zero-budget fields reserved as explicit unbounded exact opt-ins.
C++ `modify::expand::expand(space)` now preflights the exact nearest-neighbour
midpoint scan before metric calls through `ExpandOptions`, and its duplicate
synthetic-pair tracker is linear in emitted pairs instead of a `record_count *
record_count` boolean matrix.
Higher-quality ANN variants, automatic provider tuning beyond the current
sampled/Landmark selector, advanced out-of-core placement/tuning beyond the
conservative spill-byte quota, and a broader wall-clock benchmark suite remain
roadmap extensions rather than prerequisites for avoiding `O(n^2)` crashes.
Confidence diagnostics now cover sampled distribution/local-volume paths,
chunked local-volume/profile fallbacks, C++ Landmark/Pivot holdout recall, and
Python sampled/Landmark search recall with query-level standard error, though
broader provider-specific calibrated intervals remain a quality extension.

| Area | Status | Evidence |
| --- | --- | --- |
| Dense budget guards | Implemented for promoted and legacy crash paths | `resource_budget`, `execution_plan`, `estimate_cost`, dense `DistanceTable` budget refusals, safe-by-default `distance_table_options`, safe-by-default direct `SymmetricDistanceTable` budgets, legacy `MatrixSpace`/`FiniteSpace` and `GraphSpace` build-budget refusals, direct kNN-graph build budget refusals, direct LandmarkIndex build/memory budget refusals, exact-lazy C++ compression preflight refusals, exact-live C++ uniform-density resample budget refusals, direct C++ sample-work and representative-provider budget refusals, C++ clustered-space representative-table budget refusals, C++ expand nearest-neighbour midpoint budget refusals plus linear emitted-pair tracking, legacy direct C++ MGC/Entropy/VMixing budget refusals, native parametric-diffusion target-construction budget refusals, materialized preflight guards, C++ search/range/outlier/representative/density-resample distance-evaluation budget preflight including kNN-graph build costs and representative assignment work, blocked exact fallbacks for materialized search/range/compare and structural/modify operators, cooperative `max_runtime_ms` guards for C++ storage/search/context paths, guarded structural loops in groups/outliers/density_filter, Python dense materialization refusals, Python strict-validation budget refusal, Python query/compare/diagnostic/group/outlier/representative/compress/resample/mapping/import/export budget guards, Python batch-query iterator materialization guards, Python direct-helper iterator materialization guards, direct Python `metric.correlation` native-adapter budget guards including unsized adapter materialization, research-only Python experimental materialization/exact-loop/value-memory guards including metric-graph-spanner pair-list and shortest-path work preflights, precomputed-distance matrix and row guards, accepted-budget multipliers for Python radius-compression and uniform-density follow-on phases, and Python `RuntimePolicy` budget propagation for plan/materialization/query/helper paths exist. |
| Lazy storage | Implemented | Lazy `DistanceTable` uses sparse directed cache storage, keeps construction memory bounded, and has a default sparse-cache memory cap unless callers explicitly pass a zero memory budget. |
| RecordId lookup | Implemented | Metric spaces, distance tables, and snapshots use indexed RecordId lookup. |
| Position hot loops | Implemented for promoted hot paths | Live/table providers, matrix export, describe sampling, k-NN/range provider paths, k-medoids, DBSCAN, DBSCAN outlier scoring, and nearest-neighbor outlier scoring use position-based distance access when the provider exposes it, with ID-based fallback for generic providers. |
| k-NN selection | Implemented for core path | `take_nearest_neighbors` uses a direct min scan for `k == 1` and `nth_element` for small prefixes. |
| Workflow reuse | Implemented for current providers | `space::execution_context` shares live, dense, symmetric materialized, blocked materialized, or Landmark/Pivot providers across operators. A reusable `RegularSamplePlan` centralizes deterministic sampled-candidate routing for search/compress, approximate sampled contexts cache the keyed sample plan across `neighbors()` and `range()` calls, and landmark contexts build the `O(n * p)` provider once for repeated `neighbors()`/`range()` calls. The approximate context now uses the same sampled-vs-Landmark selector as `estimate_cost`, with a reuse-oriented query horizon, so small metric spaces stay sampled while large repeated metric search reuses Landmark/Pivot. |
| Safer exact storage | Implemented for dense, blocked, and explicit spill-capable exact providers | `SymmetricDistanceTable` stores only triangular off-diagonal slots for admitted symmetric metrics and is used by `execution_context`. `BlockedDistanceTable` provides lazy block-pair materialization, optional LRU block spill to disk, optional hard spill-byte quota enforcement, and is planner-selected for materialized search/range/compare, workflow contexts, and structural/modify operators that opt into exact chunking fallback. |
| Index/graph naming | Implemented for current fallbacks | Exact cover-tree and k-NN graph policies keep their index representation names. Budget downgrades that actually execute sampled search now report `sampled_metric_space` or `metric_space_sample`, not a placeholder index name. |
| Large-data defaults | Implemented for promoted defaults plus guarded legacy/export paths | Default `find_groups(space, ...)`, `find_outliers(space, ...)`, `nearest_neighbor_outliers(space, ...)`, `density_filter(space, ...)`, `cluster_diagnostics(...)`, `find_representatives(space, ...)`, `represent(space, ...)`, `compress(space, ...)`, `thin(space, uniform_density(...))`, `equalize(space, uniform_density(...))`, `compare(space, space)`, `correlate(space, space)`, `distance_distribution(space, ...)`, `local_volume(space, ...)`, `local_volume_profile(space, ...)`, `profile(...include_local_volume...)`, `entropy(space)`, `clustered_space(space, clustering)`, `expand(space)`, mapping neighbor-preservation/out-of-sample diagnostics, Python and native parametric-diffusion mapping adapters, direct Python `metric.correlation` native adapters, research-only Python experimental finite-space diagnostics, legacy `Space::from_records(...)`/`MatrixSpace`/`GraphSpace`, legacy direct C++ `MGC`/`Entropy`/`VMixing`, direct `DistanceTable(space)` / `SymmetricDistanceTable(space)` / `storage::matrix(space)` / `distances::materialize(space)` / `storage::make(...distance_table...)` entries, direct `KnnGraphIndex` / `storage::knn_graph(...)` / `storage::make(...knn_graph...)` entries, direct `LandmarkIndex` / `storage::landmark_index(...)` entries, direct `space::index` graph/shortest-path/pair-index helpers, direct low-level clustering APIs including provider-level `kmedoids`, `dbscan`, affinity propagation, representative providers, and outlier provider scans, diffusion/diffusion-coordinate target helpers, explicit `space::distances::pairs(...)` collection, low-level dense matrix conversion helpers, dense matrix export helpers, Python file/DataFrame/NumPy import/export adapters, `intrinsic_dimension(space)`, direct exact C++ `stats::correlate::mgc(...)`, MGC permutation significance, and `intrinsic_dimension_from_distances(...)` now avoid the worst unbounded exact paths. Dense-only MGC significance refuses large default `MetricSpace` inputs before metric calls. |
| Approximation | Implemented with sampled, landmark, and chunked quality diagnostics | `describe_structure`, `diagnose_space`, `compare`, `find_neighbors`, `range`, `find_groups`, `find_outliers`, `nearest_neighbor_outliers`, `density_filter`, `find_representatives`, `represent`, `compress`, `thin/equalize` uniform-density resampling, `distance_distribution`, `local_volume`, `entropy`, and mapping neighbor-preservation/out-of-sample diagnostics have explicit non-exact sampled fallbacks where appropriate. Default large `compare`/`correlate` uses bounded `mgc_estimate` instead of dense exact MGC; direct exact `mgc(...)` keeps exact semantics but refuses oversized default work before metric calls and points callers to `compare(...)`/`mgc_estimate(...)`; `mgc_significance` still requires dense exact pairwise distances and therefore preflight-refuses large `MetricSpace` defaults rather than pretending a sampled p-value is calibrated. C++ `find_neighbors`/`range` can opt into `using_landmark_index(...)`, which stores landmark distances, uses triangle-inequality lower bounds for bounded candidates, and reports non-exact Landmark diagnostics with bounded holdout recall calibration and query-level standard error. Python sampled and Landmark search recall diagnostics now also report query-level standard error and 95% confidence radius when calibration runs. Search exposes sampled candidate diagnostics and bounded recall calibration in C++ and Python when budget permits; entropy reports deterministic sample metadata, distribution, sampled local-volume, and chunked local-volume/profile fallbacks report diagnostic standard error and 95% confidence radius; sampled kNN outliers report sample/candidate/evaluation diagnostics. Higher-quality ANN and non-Landmark provider-specific calibrated intervals remain future improvements. |
| Divide and conquer | Explicit chunked execution plus bounded search/distribution/local-volume/compare/represent/compress/resample paths implemented | `chunked_view` preserves RecordIds, exposes bounded local pair iteration, per-chunk representatives, representative-pair iteration, and plan diagnostics that compare local chunk work against dense all-pairs work. It executes local chunk kNN/range, representative kNN, representative candidate generation, and bounded representative-refined kNN/range through explicit APIs. `estimate_cost`/runtime diagnostics surface a `chunked_workflow_plan` for workflow intents that opt into chunking fallback, including dense-pair baseline and bounded local-plus-representative work. Materialized `find_neighbors`/`range` requests with both `allow_chunking` and `allow_approximate` can now dispatch bounded `chunked_space_view` refinement instead of building a dense table; `RecordId` search uses representative-refined chunk pairs, while free query objects rank representatives and refine only selected chunks. `chunked_distance_distribution(...)`, `chunked_local_volume(...)`, and `chunked_local_volume_profile(...)` provide explicit bounded diagnostics over local chunk pairs plus representative pairs; chunked local-volume/profile include diagnostic standard error and 95% confidence radius, and the profile route reuses one bounded pair traversal for multiple radii. Materialized `compare(..., policy)` can now downgrade to a bounded non-exact `chunked_space_view` MGC estimate; materialized `find_representatives(..., policy)` and `represent(..., policy)` can downgrade to sampled or chunked representative candidates; default large `compress(..., radius_coverage)` and materialized count-based `compress(..., farthest_first/coverage/k_center, policy)` plus radius-coverage compression can downgrade to non-exact `chunked_space_view` or `sampled_metric_space`; materialized k-medoids compression can fall back to sampled medoids plus bounded live assignment when approximation is allowed; and materialized uniform-density `thin/equalize` resampling can downgrade to sampled or chunked radius-net candidates with bounded diagnostics. Exact-only chunking still uses blocked exact fallback. Broader automatic placement for remaining modify/resample strategies remains future work. |
| Streaming construction | Append API plus conservative Landmark and kNN graph refresh implemented | `space::streaming::append_batch` appends records directly to a live `MetricSpace`, keeps RecordIds monotonic/stable across erases, reports versions/progress/cancellation, and explicitly records zero distance evaluations and no dense all-pairs materialization. `LandmarkIndex::refresh_after_append(...)` can extend an append-only snapshot by projecting only appended records onto the existing landmark set. `KnnGraphIndex::refresh_after_append(...)` integrates appended records by updating old rows only against appended candidates and building rows for the appended records, so the refresh does `O(delta * n)` directed distance work instead of a hidden dense rebuild, and it refuses over-budget refreshes before mutating the snapshot or calling the metric. Both providers return rebuild-required diagnostics when the source space is no longer append-only. Exact tables, blocked providers, and caches remain explicit rebuild/refresh steps. |
| Benchmarks | Smoke-level targets/trends/guardrails implemented | Benchmark reports include distance-evaluation and memory fields plus automated target, trend, scale-guardrail, out-of-core readiness, and optional wall-clock trend tables. `benchmark_report_smoke` runs under CTest and exercises one-off kNN, batch kNN, repeated provider reuse, lazy sparse cache hit/miss, sampled describe, chunked workflow, sampled distance-distribution rows, spill-disabled refusal evidence with observed `CountingMetric` budgets, one explicitly synthetic wall-clock metadata row, and one small measured `std::chrono` wall-clock row with no performance threshold. Disk-spill execution is covered in the blocked-table storage smoke. A broader multi-platform wall-clock trend/perf benchmark suite remains future work. |
| Python UX | Implemented for sampled safe defaults plus explicit and automatic Landmark routing | `Space.plan(...)`, `Space.describe_plan(...)`, dense materialization preflight, sampled over-budget `describe()` with diagnostics, neighbor/range query-budget guards, compare/correlate and direct diagnostic preflight guards, clustering/outlier/representative/compress/resample/mapping/import/export guards, accepted-budget follow-on phase accounting, derived-space `validate="none"`/`cache="none"` construction, `RuntimePolicy` budget fields, `exact=False` sampled neighbor/range routing, sampled candidate diagnostics, bounded recall calibration, and Landmark/Pivot search exist. Exact runtime policies now carry `max_memory_bytes`, `max_distance_evaluations`, and `max_dense_records` into `Space.plan(...)`, `Space.to_matrix()`, `Space.pairwise_distances()`, `Space.distance_matrix_numpy()`, `Space.neighbors()`, `Space.describe()`, `Space.compare()`/`Space.correlate()`, `Space.groups()`/`Space.outliers()`/`Space.representatives()`/`Space.reduce()`/`Space.compress()`, `Space.thin()`/`Space.uniform_density_sample()`/`Space.equalize()`, `Space.map(transform=...)`, `Space.map(strategy=ParametricDiffusionCoordinates(...))`, `Space.from_csv(...)`/`from_tsv(...)`/`from_numpy_file(...)`/`from_dataframe(...)`, `Space.to_csv(...)`/`to_tsv(...)`, `metric.mappings.derive_parametric_diffusion_coordinates(...)`, parametric-diffusion artifact `transform(...)`/`inverse_transform(...)`, `metric.mapping_pipeline.derive_mapping_pipeline(...)`/artifact `neighbor_recall(...)`, research-only `metric.experimental` diagnostics, and direct `pairwise_distance_matrix(...)` / `nearest_neighbors(...)` / `range_neighbors(...)` / `compare_spaces(...)` / `correlate_spaces(...)` / `describe_structure(...)` / `intrinsic_dimension(...)` / `intrinsic_dimension_from_distances(...)` / `map_space(...)` / `kmedoids(...)` / `dbscan(...)` / `find_outliers(...)` / `representative_indices(...)` / `find_representatives(...)` / `reduce_space(...)` / `compress_space(...)` / `medoid_index(...)` / `medoid(...)` / `separated_representative_indices(...)` / `coverage_representative_indices(...)` / `thin_space(...)` / `uniform_density_sample_space(...)` / `equalize_space(...)` helpers, with explicit method arguments taking precedence. `exact=False, representation="landmark"` forces Landmark/Pivot search and now obeys explicit memory budgets, while repeated or batch source-metric Python search (`query_count > 1`) auto-selects `landmark_index` only for known native/admitted metrics when budgets and record count admit bounded candidates. Unmarked Python callables stay on sampled search unless Landmark is explicitly requested. The Python sampled and Landmark routes mark results non-exact and report query-level recall standard error plus 95% confidence radius when calibration fits the remaining budget. The Python Landmark route builds in `O(n * p)`, refines bounded lower-bound candidates, and reports `landmark_index` diagnostics/provenance, including bounded recall calibration against a larger lower-bound reference window when the post-refinement budget permits it. Broader ANN tuning remains future work. |

## Crash-Prevention Completion Audit (2026-06-26)

This audit treats the crash-prevention part of the plan as implemented for the
promoted finite-space surface and the identified direct legacy crash paths. The
remaining items below this section are quality, tuning, or broader provider
extensions, not prerequisites for avoiding accidental `O(n^2)`/worse crashes.

- P0 guardrails are implemented: dense memory/distance/dense-record budgets are
  enforced before dense construction, native adapters, exact diagnostics, and
  Python iterator materialization. Evidence: `resource_budget`,
  `execution_plan`, Python `RuntimePolicy`, dense-table/default helper guards,
  and targeted refusal tests with zero metric calls.
- P1 hidden constants are implemented for the promoted hot paths: `RecordId`
  lookup is indexed, provider loops prefer positional distance access, k-NN uses
  partial selection, snapshot lookup is indexed, and legacy matrix/graph
  constructors now have safe default build budgets.
- P2 workflow reuse is implemented for current providers:
  `space::execution_context` reuses live/materialized/blocked/Landmark providers,
  sampled search plans are keyed and reused, and repeated approximate search can
  use an `O(n * p)` Landmark/Pivot provider instead of repeated dense builds.
- P3 safer exact representations are implemented: symmetric triangular tables,
  blocked exact providers, optional bounded disk spill, safe direct table/index
  construction, and exact policy refusals before provider construction where the
  accepted representation would exceed budget.
- P4 approximation-over-crash defaults are implemented for search, compare,
  structural diagnostics, entropy, local volume/profile, distribution,
  representatives, compression, outliers, density filtering, mapping
  diagnostics, Python sampled/Landmark routes, and research-only experimental
  diagnostics. Downgraded results are marked non-exact and carry diagnostics.
- P5 divide-and-conquer is implemented for the covered workflows: chunked views
  bound local quadratic work, use representatives for cross-chunk refinement,
  and are wired into materialized search/range, compare, representatives,
  compression, local-volume/profile, distance distribution, and uniform-density
  resampling when policies allow approximation/chunking.
- Direct legacy closure is implemented for the audited crash paths:
  C++ `MGC`/`MGC_direct`/`distance_matrix`, C++ `Entropy`/`VMixing`, Python
  `metric.correlation`, Python mapping artifacts, Python native import/export
  adapters, legacy `MatrixSpace`/`GraphSpace`/`KNNGraph`, direct table/index
  helpers, and `space::index` graph/shortest-path helpers all either guard by
  default or remain explicitly low-level streaming/unbounded opt-ins.

Verification run for this audit:

- `ctest --test-dir build/core -R '^(engine_mapping_artifact_smoke|search_pipeline_interoperability_smoke|engine_chunked_space_smoke|stats_user_api_properties_smoke|stats_production_readiness_smoke|stats_user_api_correlate_smoke|engine_cross_space_dependency_smoke|modify_expand_synthesis_smoke|metric_entropy_estimator_smoke|metric_mgc_properties_smoke|parametric_diffusion_coordinate_pipeline_smoke|native_diffusion_coordinate_targets_smoke)$' --output-on-failure --timeout 180`
- `PYTHONPATH=python/pkg /Users/michaelwelsch/.local/bin/python3.12 -m unittest` with the targeted revival API budget, iterator, correlation, mapping-artifact, import/export, experimental, and Landmark tests listed in the implementation log.
- `git diff --check` over the touched C++ correlation/statistics, Python
  correlation/mapping/space, tests, and this plan.

Remaining risk posture and roadmap:

- Large default `compress(space, count)` now uses bounded sampled assignment for
  high target counts and identity compression for `count == n`.
- Explicit pair collection through `space::distances::pairs(...)` now has a
  default in-memory pair budget; unbounded streaming remains available through
  `for_each_pair(...)`.
- Direct dense table materialization through `DistanceTable(space)`,
  `SymmetricDistanceTable(space)`, `space::storage::matrix(space)`,
  `space::distances::materialize(space)`, and
  `space::storage::make(space, distance_table{})` now uses safe default table
  budgets and refuses oversized eager materialization before metric calls. Passing
  zero table budgets remains the explicit unbounded opt-in for low-level callers.
  `execution_context(space, using_distance_table(...))` now also checks the
  caller's `max_distance_evaluations` before constructing either the symmetric
  triangular provider or the full dense provider.
- `FiniteSpace::pairwise_distances(...)` and
  `space::index::pairwise_distance_matrix(...)` now guard dense `n x n` matrix
  output before allocation; the convenience operator preflights before building
  the matrix-backed space.
- Legacy `MatrixSpace`/`FiniteSpace` and `GraphSpace` constructors/mutations now
  guard their exact distance-matrix builds by default. `space::index` direct
  `nearest_neighbors(...)`, `range_neighbors(...)`, `exact_knn_graph(...)`,
  `exact_radius_graph(...)`, `graph_stretch_diagnostics(...)`,
  `graph_metric_pair_indices(...)`, and shortest-path helpers no longer build a
  hidden `MatrixSpace` before doing their advertised work.
- Low-level `provider_dense_distance_matrix(...)`,
  `provider_symmetric_distance_matrix(...)`, and
  `metric_space_dense_distance_matrix(...)` now guard dense matrix conversion
  before allocation while keeping explicit `dense_distance_matrix_options{0}` as
  the unbounded opt-in.
- Promoted `space::storage::using_knn_graph(...)` planning now charges the full
  `n * (n - 1)` directed index build plus query work before constructing
  `KnnGraphIndex`, so low distance-evaluation budgets refuse before metric
  calls instead of underestimating the build as one live scan.
- Promoted `space::storage::using_cover_tree(...)` planning now charges a
  conservative `n * n` exact build estimate plus query work before constructing
  `CoverTreeIndex`, and direct `CoverTreeIndex(space)` /
  `space::storage::cover_tree(space)` have safe default build-distance and
  memory budgets. Passing `cover_tree_index_options{0, 0}` remains the explicit
  unbounded low-level opt-in.
- Free-query C++ `stats::search::knn_batch(space, queries, ..., policy)` and
  `range_batch(space, queries, ..., policy)` now preflight batch exact work as
  `query_count * record_count`; the record-container wrappers forward the same
  policy and refuse before metric calls when the budget is too low.
- Ergonomic C++ `space::query::try_nearest(...)` and `nearest(...)` now expose
  policy/strategy overloads for both RecordId and free-query records, so callers
  do not need to drop down to `find_neighbors(..., policy)` just to enforce a
  budget.
- Mapped C++ batch search now builds one SourceId lookup index per batch and
  reuses it across `knn_batch(mapping, ...)` / `range_batch(mapping, ...)`
  queries, avoiding the previous `O(q * source_lineage)` pre-search scan.
- Direct `KnnGraphIndex(space, k)`, `space::storage::knn_graph(...)`,
  `space::storage::graph(...)`, and `space::storage::make(...knn_graph...)` now
  have a default build-distance budget and refuse oversized graph construction
  before metric calls. `k == 0` builds an empty graph without evaluating
  distances, and `quality_against(provider)` samples large graph rows by default
  instead of validating every row when `sample_count` is omitted.
  `KnnGraphIndex::refresh_after_append(...)` also has a default append-refresh
  distance-evaluation budget and refuses before mutating `ids_`, `records_`, or
  `adjacency_` when the admitted append would exceed it.
- Direct `LandmarkIndex(space, ...)` and `space::storage::landmark_index(...)`
  now have default build-distance and projection-memory budgets before snapshot
  copy/projection work starts. `landmark_index_options{..., 0, 0}` remains the
  explicit unbounded opt-in, and policy-driven Landmark search forwards explicit
  runtime distance/memory budgets into the provider.
- Legacy exact graph helpers now guard exhaustive graph construction and
  stretch diagnostics: `exact_knn_graph(...)` and `exact_radius_graph(...)`
  refuse over-budget directed pair work before metric calls, radius construction
  no longer materializes a full pair-index list, and
  `graph_stretch_diagnostics(...)` preflights both direct metric comparisons and
  dense all-pairs shortest-path storage/closure.
- Python `validate="strict"` now preflights its full `n * n` distance scan with
  `max_distance_evaluations` and `max_dense_records`, so constructor validation
  can refuse before the first metric call on large inputs.
- Python exact `RuntimePolicy` budget fields are now visible in diagnostics and
  enforce the same preflight guards across planning, dense materialization,
  exact search/range, exact describe, exact compare/correlate, intrinsic
  dimension, clustering, outliers, representative selection, reduce/compress,
  precomputed intrinsic-dimension input, parametric-diffusion mapping,
  clustered-space mapping,
  medoid/separated/coverage convenience helpers, uniform-density resampling,
  research-only experimental diagnostics, and direct
  pairwise/search/diagnostic/modify/mapping helpers; explicit per-call budget
  arguments remain the override. Radius-compression and uniform-density
  resampling estimates now include assignment and diagnostic phases,
  experimental distance-distribution drift reuses pair distances instead of
  recomputing them, generates pair ordinals lazily, and refuses oversized
  value-vector memory before metric calls, direct dense/native helper iterators
  are bounded before `list(...)` across representative/compress/diagnostic helpers,
  `compare_spaces(... matched_ids/dropped_*_ids)` is bounded before native correlation,
  precomputed distance rows are bounded before native conversion,
  CSV/TSV/NumPy/DataFrame imports preflight source row counts before materializing,
  TreeIndex/GraphIndex facades do not materialize source records before
  native-only diagnostics, clustered-space assignment iterators are not read
  before representative-matrix budget refusal and assignment/medoid result-like
  iterators are capped when the representative matrix is admitted, and derived target spaces do not
  inherit strict validation or materialized caches from the source space.
- Python batch query iterators are budgeted while they are read: exact
  neighbor/range batches refuse on the first query count that exceeds the
  `query_count * record_count` plan, and non-exact batches still stop at the
  dense-record materialization cap before invoking the metric.
- Python `Space(...)`/`FiniteMetricSpace(...)` refuses unsized record iterators
  after `max_dense_records + 1` reads instead of exhausting the iterator before
  validation/cache budget checks.
- Python `Space.pairwise(ids=...)` budgets oversized ID iterators before building
  the position list, and direct `nearest_neighbors(..., exact=False)` /
  `range_neighbors(..., exact=False)` keep a default materialization cap even
  when only a distance-evaluation budget is supplied.
- Native parametric-diffusion coordinate target construction now carries the same
  dense-record, memory, and distance-evaluation budget axes through
  `DiffusionCoordinateSpec`, `DiffusionOptions`, pybind entry points, and
  serialized artifact metadata. Smoke tests prove `max_memory_bytes` and
  `max_distance_evaluations` refuse before any metric calls. Python's optional
  native-budget retry removes only unsupported binding keywords one at a time and
  preserves real native validation `TypeError`s.
- Sampled distribution, nearest-neighbor outliers, compare/correlate, sampled
  local-volume defaults, chunked compare, chunked local-volume/profile,
  sampled/chunked representative selection, chunked compress,
  sampled/chunked uniform-density resampling, and mapping diagnostics now expose bounded
  diagnostics/routes; C++ Landmark/Pivot search and Python sampled/Landmark
  search add query-level holdout recall intervals. MGC estimate and
  non-Landmark provider-specific ANN routes still report coarser quality than
  search recall calibration.
- C++ exact-lazy compression now shares the `execution_plan` budget gate used by
  materialized compression. Count-based compression estimates representative
  selection plus assignment as `2 * n * count`; radius-coverage and k-medoids
  exact-live paths use a conservative quadratic estimate and refuse before
  provider construction or metric calls when the budget is too low.
- Remaining roadmap items are quality/performance extensions: higher-quality ANN
  backends, automatic provider tuning beyond the current sampled/Landmark
  selector, broader chunked workflow placement for additional modify/resample
  strategies, advanced out-of-core placement/tuning beyond the current hard
  spill-byte quota, and a broader real wall-clock trend benchmark suite.

## Goals

- Prevent accidental memory exhaustion from dense all-pairs storage.
- Prevent hidden `O(n^2)` and worse behavior from surprising default paths.
- Reuse one expensive pairwise computation across many operators.
- Prefer bounded approximations over crashes for large spaces.
- Make cost visible before execution: estimated distance evaluations, memory,
  exactness, representation, and fallback reason.
- Keep exact behavior available and deterministic for moderate data sets.

## Non-Goals

- Do not promise that every metric admits sublinear exact search.
- Do not make approximate results look exact.
- Do not silently change mathematical contracts: every downgraded path must mark
  result exactness and representation.
- Do not optimize older APIs before the promoted surface is protected.

## Current Risk Map

| Risk | Current behavior | Failure mode |
| --- | --- | --- |
| Dense `DistanceTable` | Default table options cap eager dense records/memory and lazy cache growth; direct symmetric tables also cap dense records, memory, and distance evaluations; explicit zero budgets preserve the unbounded low-level opt-in | Deliberately unbounded low-level calls can still exhaust RAM if used on large spaces |
| Legacy exact representations | `MatrixSpace`/`FiniteSpace` and `GraphSpace` now cap default exact matrix builds, direct `space::index` search/graph helpers avoid hidden eager MatrixSpace materialization, and shortest-path/pair-index helpers refuse oversized dense work by default | Deliberately unbounded legacy opt-ins remain exact and can still exhaust resources |
| `RecordId` lookup | Metric spaces, distance tables, and snapshots keep indexed lookup for promoted paths | Any remaining deliberately low-level helper that bypasses the provider APIs must preserve this invariant when promoted |
| k-NN selection | `take_nearest_neighbors` uses direct min selection for `k == 1` and `nth_element` for small prefixes | Broader provider-specific candidate-generation costs still dominate large exact scans |
| Per-call materialization | Runtime policies build tables/indexes inside each operator call | Workflows repeat the same `O(n^2)` build |
| Unbounded diagnostics | `describe_structure`, default `intrinsic_dimension`, promoted `entropy`, precomputed `intrinsic_dimension_from_distances`, `distance_distribution`, `local_volume`, `local_volume_profile`, direct provider local-volume diagnostics, chunked/exact profile helper storage, `cluster_diagnostics`, mapping neighbor-preservation/out-of-sample diagnostics, and research-only Python experimental diagnostics now use sampling, guarded dense input/materialization, shared scans, or preflight refusal when exact work exceeds their default threshold | Provider-specific calibrated intervals and broader quality reporting remain improvements |
| Approximation surface | Core structural/search/modify paths have sampled fallbacks and search recall calibration; C++ search also has an explicit Landmark/Pivot candidate provider with bounded holdout calibration; calibrated search recall, sampled distribution/local-volume, and chunked local-volume/profile report standard error and 95% confidence radius | Some large workflows still fail or return coarse approximations instead of bounded, measured degradation |
| Snapshot lookup | `distance_table_snapshot` keeps lazy RecordId and cell-key indexes for expected `O(1)` `position_of`, `has_distance`, and `distance` lookups while preserving the compact cell vector | Broader restored-provider reuse remains a workflow-placement concern, not a snapshot lookup bottleneck |
| Graph/index naming | Exact index policies report index representations; sampled budget downgrades report sampled representations; explicit Landmark/Pivot search reports `landmark_index`; C++ multi-query approximate planning auto-selects Landmark/Pivot for admitted metrics when the calibrated estimate is no worse than sampled search, and approximate execution contexts auto-select Landmark/Pivot for admitted metrics | Broader cross-language ANN provider choice and confidence tuning remain roadmap items |

## Design Principle: Cost-Directed Execution

Every high-level operator should run through a small planning layer before it
allocates or evaluates distances:

```text
intent + record_count + metric traits + caller policy + budget
    -> execution_plan
    -> exact provider, approximate provider, chunked provider, or refusal
```

The plan should carry:

- `intent`: neighbors, range, groups, outliers, describe, compare, embed, map.
- `record_count` and optional query count.
- estimated distance evaluations.
- estimated memory bytes.
- exactness: exact or approximate.
- representation: live, dense table, triangular table, sparse cache, cover tree,
  graph, sample, sketch, chunked.
- budget decision: allowed, downgraded, or refused.
- explanation string suitable for diagnostics and logs.

## Phase 0: Guardrails Before Speed

Add hard runtime budgets before adding new algorithms.

### P0.1 Memory Budget For Dense Representations

Introduce `space::storage::resource_budget`:

```cpp
struct resource_budget {
    std::size_t max_memory_bytes;
    std::size_t max_distance_evaluations;
    std::size_t max_dense_records;
    bool allow_approximate;
    bool allow_chunking;
    bool allow_out_of_core_spill;
    std::size_t max_runtime_ms;
    std::size_t max_spill_bytes;
};
```

Default behavior:

- Dense `DistanceTable` refuses construction when estimated bytes exceed budget.
- Error message reports `records`, `estimated_bytes`, `budget_bytes`, and the
  suggested safer representation.
- Policies can opt into `allow_approximate`, `allow_chunking`, and explicit
  `allow_out_of_core_spill`.

### P0.2 Make Lazy Truly Safe

Change lazy distance storage from dense `vector<optional<Distance>>` to one of:

- sparse pair cache keyed by normalized pair for symmetric metrics.
- sparse directed pair cache for non-symmetric distances.
- optional dense backing only when `n <= max_dense_records`.

Lazy must mean low memory at construction time.

### P0.3 Cost Diagnostics Everywhere

Add `estimate_cost(space, intent, policy)` and expose it through:

- C++ diagnostics.
- Python `Space.plan(...)` / `Space.describe_plan(...)`.
- benchmark reports.

No operator should allocate dense storage before cost estimation.

### P0.4 Large-Data Defaults

For automatic policy:

- small/medium spaces: exact dense or exact index when beneficial.
- large spaces: live scan for one-off queries, approximate index for repeated
  queries, sample/sketch for diagnostics.
- huge spaces: chunked or refused with a plan report.

## Phase 1: Remove Hidden Constant Bombs

### P1.1 O(1) RecordId Position Lookup

Store an ID-to-position map in `MetricSpace` and snapshot providers.

Options:

- `std::unordered_map<RecordId, std::size_t>` with a `RecordId` hash.
- dense vector map when IDs are compact enough.

Update on insert/erase/replace:

- insert: append position.
- replace: unchanged.
- erase: update shifted positions or use swap-delete only if ordering contract is
  explicitly changed. Preserve order by default.

Expected impact:

- `MetricSpace::distance(id, id)` becomes actual metric cost plus `O(1)` lookup.
- `DistanceTable::distance(id, id)` cache hits become `O(1)`.
- all pairwise loops lose a hidden `O(n)` factor.

### P1.2 Position-Based Hot Loops

Add internal provider APIs:

```cpp
distance_at_position(lhs_position, rhs_position)
record_at_position(position)
```

Use them in dense table construction, matrix export, k-NN scans, clustering,
DBSCAN, and diagnostics. Public APIs can remain ID-based.

Implementation status: live/table providers, matrix export, describe sampling,
k-NN/range provider paths, k-medoids, DBSCAN, DBSCAN outlier scoring, and
nearest-neighbor outlier scoring prefer `distance_at_position` where available
and fall back to ID distance for generic providers. Structural smoke tests use a
position-counting provider to prove the promoted structural paths avoid
ID-distance calls when a positional provider is supplied.

### P1.3 Partial Selection For k-NN

Replace full sort in `take_nearest_neighbors` with:

- `std::nth_element` + sort the selected prefix for deterministic output.
- bounded max-heap for streaming candidate generation.
- direct min scan for `k == 1`.

This keeps deterministic tie-breaking while avoiding full candidate sort.

### P1.4 Snapshot Indexing

Give `distance_table_snapshot` an indexed provider form:

- build a pair-key map once on restore.
- support `has_distance` and `distance` in expected `O(1)`.
- preserve the compact serialized cell list.

Implementation status: `distance_table_snapshot` stores the compact serialized
cell vector plus mutable lazy indexes for source `RecordId -> position` and
`(lhs, rhs) -> cell position`. `contains`, `position_of`, `has_distance`, and
`distance` rebuild the relevant index only when the underlying vector size
changes, so copied/moved or manually populated snapshots preserve compact storage
while avoiding repeated linear scans. `engine_distance_matrix_snapshot_smoke`
covers manual, eager, lazy, copied, and moved snapshots.

## Phase 2: Reuse Work Across Operators

### P2.1 Workflow Context

Introduce a reusable context:

```cpp
auto ctx = mtrc::space::execution_context(space, budget);
ctx.neighbors(query, k);
ctx.groups(options);
ctx.outliers(options);
ctx.describe();
```

The context owns reusable representations:

- live provider.
- lazy sparse cache.
- dense/triangular table when allowed.
- blocked materialized provider when dense exact storage exceeds budget and
  chunking fallback is enabled.
- Landmark/Pivot provider for approximate metric search.
- neighbor index.
- sampled sketch.

High-level operators should accept either a `Space` or a provider/context.

Implementation status: `space::execution_context` now reuses live, dense,
symmetric, or blocked pairwise providers across neighbor, range, grouping,
outlier, and describe calls. It exposes provider diagnostics and refuses dense
materialization before metric calls when chunking fallback is not enabled.
Approximate contexts cache keyed `RegularSamplePlan` sketches across compatible
`neighbors()` and `range()` calls. Landmark contexts build the `O(n * p)`
provider once and reuse it across repeated `neighbors()` and `range()` calls.

### P2.2 Materialize Once, Consume Many

A workflow with `m` operators over the same records should pay one shared build:

```text
before:  m * O(N^2) pairwise work
after:   O(N^2) build + m * cheap provider reads
```

For chunked data with `N = m * b`, prefer:

```text
before:  O(N^2) = O(m^2 * b^2)
after:   m * O(b^2) + merge/index/refinement cost
```

This is the divide-and-conquer path: bound each local quadratic block, then use
representatives, pivots, or an index to connect blocks.

### P2.3 Provider-First Operator Overloads

Add and document overloads that consume existing providers:

- `find_neighbors(provider, query_id, k)`.
- `find_groups(provider, options)`.
- `find_outliers(provider, options)`.
- `describe_structure(provider)`.
- `compare(left_provider, right_provider)`.

Avoid policy overloads that rebuild the provider inside every call when the
caller already has one.

## Phase 3: Safer Exact Representations

### P3.1 Triangular Symmetric Distance Table

For admitted symmetric metrics:

- store only `n * (n - 1) / 2` off-diagonal cells.
- return zero for diagonal.
- mirror `(lhs, rhs)` and `(rhs, lhs)`.

Keep directed dense storage for non-symmetric distances.

### P3.2 Blocked Distance Table

Add a block provider:

- fixed block size selected by memory budget.
- materialize one block or block pair at a time.
- optional LRU cache for hot blocks.
- spill blocks to disk only when an explicit out-of-core policy is enabled.

This protects large runs from allocating the entire matrix at once.

Implementation status: `BlockedDistanceTable` provides lazy block-pair
materialization, a small LRU cache, and optional disk spill for evicted blocks
when `allow_out_of_core_spill` or provider options enable it explicitly. A
spilled block reloads from disk without re-running the metric, while the default
provider remains memory-only. `execution_plan` now selects the blocked provider
for materialized search, range, compare, groups, outliers, density_filter, and compress
when dense exact storage exceeds the memory budget and exact chunk/block
processing is allowed. `execution_context` also selects it for shared workflow
providers. When an exact blocked/chunked fallback is admitted with
`allow_out_of_core_spill`, `execution_plan` and `RuntimeDiagnostics` now surface
the concrete out-of-core placement summary before execution: spill enabled,
block size, block count, planned spill blocks, max resident block cap, resident
byte estimate, spill byte estimate, memory-bounded status, max spill-byte quota,
and the explicit policy requirement. Remaining work is to route reusable
index/sketch providers through the same planning surface for more intents and to
add advanced placement tuning such as compression, mmap, or platform-specific
policies.
The benchmark report records spill-disabled refusal before any metric calls;
`engine_blocked_distance_table_smoke` covers explicit disk-spill execution and
reloads without additional metric calls plus quota refusal before a second spill
write.

### P3.3 Exact Tree/Metric Index Contracts

Clarify and harden tree/index behavior:

- cover tree only for admitted true metrics.
- no index build for one external query unless the planner predicts reuse.
- indexed strategy should not build a graph and then perform a full scan unless
  the diagnostic says so.

Implementation status: exact cover-tree and k-NN graph policies keep explicit
index representation names. When a budgeted exact request downgrades into the
current sampled fallback, `estimate_cost` now reports `sampled_metric_space` or
`metric_space_sample` instead of a placeholder approximate-index name, so plan
reports describe the representation that will actually execute. Reuse-aware ANN
selection now has an initial C++ path for admitted metrics: multi-query
approximate planning and approximate execution contexts can auto-select
`landmark_index`. Explicit `using_landmark_index(...)` search also reports and
executes `landmark_index`.

## Phase 4: Approximation Instead Of Crashing

Approximate behavior must be first-class, explicit, and measured.

### P4.1 Approximate Neighbor Index

Add an approximate nearest-neighbor provider with clear recall diagnostics:

- candidate technologies: HNSW, navigable small-world graph, VP-tree/ball-tree
  fallback, or metric pivots.
- supports `build`, `knn`, `range candidate generation`, and `quality_against`.
- result exactness is `false` unless exact refinement over all candidates is done.

### P4.2 Approximate Range Queries

Range queries are harder than k-NN. Use two modes:

- conservative candidate mode: approximate index proposes candidates, exact
  metric filters them. May miss true neighbors, so mark approximate.
- verified mode: use metric bounds/pivots where possible; exact only when recall
  can be guaranteed.

### P4.3 Approximate Diagnostics

Large `describe_structure`, intrinsic dimension, entropy, local volume, and
distribution summaries should use samples by default when exact cost exceeds
budget.

Expose:

- sample count.
- seed.
- confidence/variance estimate where available.
- exactness flag.
- reason for downgrade.

### P4.4 Approximate Clustering And Outliers

Replace crash-prone exact all-pairs defaults with staged algorithms:

- sample representatives.
- cluster representatives exactly.
- assign full records by nearest representative.
- refine boundary records only.
- for DBSCAN-like behavior, use approximate radius candidates plus exact local
  filtering.

Exact algorithms remain available behind budget checks.

Implementation status: sampled fallbacks now cover compare, neighbors, range,
groups, DBSCAN outliers, nearest-neighbor outliers, density_filter, compress, describe,
distance distribution, local volume, diagnose, and default intrinsic dimension.
Default large `compare`/`correlate` routes preflight dense MGC work and use
bounded `mgc_estimate`; large default `mgc_significance` refuses before metric
calls because the permutation p-value path still requires dense exact pairwise
distances.
`local_volume_profile` now shares one pairwise scan across radii instead of
repeating it per radius, and default large `local_volume(space, radius)` samples
per source instead of evaluating all `n * n` directed pairs. Search sampled
fallbacks report candidate count,
candidate universe, candidate fraction, distance evaluations, and bounded recall
calibration when the budget has room for a holdout/reference window. Distribution
sampling reports sample fraction, diagnostic standard error, and a 95%
confidence radius; sampled nearest-neighbor outliers report sample size,
candidate universe, and observed distance evaluations; sampled local-volume and
chunked local-volume/profile report candidate/sample fractions plus diagnostic
standard-error and 95% confidence-radius estimates. Search and compression share
a deterministic `RegularSamplePlan` for sampled candidate routing, and
`execution_context` reuses
the same plan across compatible approximate search calls. C++ search can also
use `using_landmark_index(...)` to build an `O(n * p)` landmark snapshot, rank
candidates by triangle-inequality lower bounds, and refine only the bounded
candidate set with the exact metric. Calibrated search recall reports standard
error and 95% confidence radius; C++ Landmark/Pivot holdout calibration and
Python sampled/Landmark calibration use per-query recall variance instead of
only an aggregate hit ratio. Remaining work is higher-quality ANN selection and
provider-specific calibrated intervals beyond the current sampled, chunked, C++
Landmark, and Python search holdout diagnostics.

## Phase 5: Divide-And-Conquer Execution

### P5.1 Chunked Space View

Introduce a chunked provider:

```text
records -> chunks -> local summaries -> merge/refine
```

Requirements:

- stable `RecordId` identity across chunks.
- local exact pairwise work bounded by chunk size.
- representative records per chunk.
- plan diagnostics reporting dense all-pairs work versus bounded local chunk work
  and representative merge work.
- optional cross-chunk candidate generation via pivots/index.

Implementation status: `chunked_view` is now a snapshot helper with stable source
RecordIds, chunk iteration, exact local pair callbacks, representative IDs and
records, representative-pair callbacks, stale-version reporting, and
`chunked_work_diagnostics`. It exposes explicit execution APIs:
`local_neighbors(k)`, `local_range(radius)`, `representative_neighbors(k)`,
`representative_candidates(candidate_chunks_per_chunk)`,
`refined_neighbors(k, candidate_chunks_per_chunk)`, and
`refined_range(radius, candidate_chunks_per_chunk)`. The refined operators first
run local exact pair work, then rank cross-chunk candidates by representative
distances, then exactly evaluate only the selected cross-chunk record pairs. They
report the dense baseline, local pair work, representative work, refinement
chunk/candidate/evaluation counts, and exact/non-exact status; results are
globally exact only when the selected candidate chunk pairs cover all cross-chunk
pairs. The same count-based plan diagnostics are wired into `estimate_cost` and
`RuntimeDiagnostics` for `workflow`/`workflow_context` planning when the runtime
policy allows chunking fallback. Materialized `find_neighbors` and `range`
requests now have an execution-integrated chunked search downgrade: when the
dense table exceeds budget and the policy explicitly allows both chunking and
approximation, the planner reports `chunked_space_view` and the operator avoids
constructing a dense table. `RecordId` search runs representative-refined chunk
pairs, while free query objects score chunk representatives and refine only the
selected chunks. Exact-only chunking continues to use the blocked exact provider,
so the exact contract is not silently weakened. The smoke coverage proves a
space with `N = m * b` executes bounded chunked work below the dense
`N * (N - 1) / 2` baseline for representative settings. Distribution diagnostics
now also have an explicit `chunked_distance_distribution(...)` route that samples
local chunk pairs plus representative pairs and reports the dense baseline,
bounded evaluation count, fractions, and diagnostic standard error. Local-volume
diagnostics have matching explicit `chunked_local_volume(...)` and
`chunked_local_volume_profile(...)` routes that count exact local chunk pairs
plus representative pairs, report the dense baseline and bounded evaluation
fraction, and reuse one bounded pair traversal across multiple profile radii.
Materialized `compare(..., policy)` can downgrade to a bounded non-exact
`chunked_space_view` MGC estimate when dense materialization exceeds budget and
the policy explicitly allows both chunking and approximation. Materialized
`compress(..., farthest_first/coverage/k_center, policy)` can also downgrade to
non-exact `chunked_space_view` under the same guard, with bounded chunk work
plus a conservative assignment estimate. Radius-coverage compression can use a
sampled candidate set or chunk representatives before assigning against the
selected representatives, and the default large `compress(..., radius_coverage)`
route now uses the same sampled representative guard instead of bypassing it
through a live provider. Materialized k-medoids compression can also avoid a dense
assignment provider by falling back to sampled medoids plus bounded live
assignment when approximation is explicitly allowed. Uniform-density
`thin/equalize` resampling can avoid full-source radius-net diagnostics by using
sampled candidates or chunk representatives, then assigning records only against
that bounded representative set. Direct `find_representatives` and
`represent` calls can use the same sampled or chunked candidate pools and report
non-exact `RepresentativeSet` metadata without building dense tables. Remaining
work is broader automatic chunked workflow placement beyond the current search,
distribution, local-volume, compare, representative selection, count-based
compression, radius-coverage compression, sampled k-medoids compression, and
uniform-density resampling paths, especially other modify/resample strategies.

### P5.2 Pivot And Landmark Distances

Use landmarks to avoid all-pairs evaluation:

- pick pivots by farthest-first or reservoir sampling.
- store `O(n * p)` distances.
- use triangle inequality bounds for pruning.
- use embeddings/sketches for candidate generation, then exact metric refinement.

For true metrics, this creates a path toward logarithmic or subquadratic query
behavior without changing record semantics.

Implementation status: `metric/space/storage/landmark_index.hpp` implements a
deterministic farthest-first Landmark/Pivot index for admitted metric or
pseudo-metric spaces. It snapshots records and RecordIds, stores `O(n * p)`
landmark distances, ranks candidates by lower-bound distance to the query, and
refines only the bounded candidate set with exact metric evaluations. The C++
runtime policy `space::storage::using_landmark_index(candidate_limit,
approximate())` routes `find_neighbors` and `range` through this provider,
reports `landmark_index`, marks results non-exact, and exposes candidate/eval
diagnostics. Landmark search now also runs a bounded holdout calibration when
the request and distance-evaluation budget allow it: a small deterministic set
of source queries is compared against a capped lower-bound reference window, so
diagnostics report whether calibration ran, holdout query count, reference
candidate evaluations, matched/reference counts, recall estimate, query-level
standard error, and 95% confidence radius without building a dense all-pairs
table. If calibration is skipped, the result reason states the guard, such as
`k=0`, an empty reference window, or `max_distance_evaluations`.
`space::execution_context` reuses the same landmark
provider across repeated search calls, and automatic approximate contexts use
the same sampled-vs-Landmark selector as `estimate_cost` with a reuse-oriented
query horizon. Multi-query `estimate_cost(..., query_count, approximate())` also
plans `landmark_index` for admitted metrics only when the calibrated Landmark
estimate is no worse than sampled search.
Implicit Landmark candidate and landmark counts are derived conservatively from
`n`, query count, and budget while explicit `using_landmark_index(...)`
candidate limits are preserved. Direct Landmark construction now preflights
`admitted_landmarks * (n - 1)` build evaluations and `n * admitted_landmarks`
projection storage before copying records or calling the metric; zero build and
memory budgets are the explicit unbounded opt-in. Policy-driven Landmark search
passes explicit runtime distance/memory budgets into temporary and shared
providers. A conservative append-only maintenance surface,
`LandmarkIndex::refresh_after_append(...)`, now extends the existing landmark
projection matrix by evaluating only `delta * landmark_count` distances for
appended records. It keeps the current landmark set fixed, updates the snapshot
version/cache key, reports the exact refresh distance count, and returns
`rebuild_required` with a reason when RecordIds are no longer a prefix or the
space version changed without a matching append-only suffix. Rebuild is
recommended, not hidden, when a full farthest-first pass would be needed to
choose additional landmarks after growth. Remaining work is richer ANN
confidence intervals and incremental maintenance for remaining provider
families.

### P5.3 Streaming Construction

Support ingestion APIs that never require the full all-pairs matrix:

- append batches.
- update indexes incrementally where possible.
- rebuild expensive exact structures only when requested.
- provide progress and cancellation hooks.

Implementation status: `metric/space/streaming.hpp` provides
`mtrc::space::streaming::append_batch` / `mtrc::space::append_batch` for live
spaces. The first cut is intentionally conservative: it appends records through
`MetricSpace::insert`, reports requested/appended counts, old/new size,
version-before/version-after, appended `RecordId`s, progress fraction, and
cancellation state, and does not evaluate pair distances or materialize a dense
provider. `mtrc::space::streaming::refresh_after_append(provider, report)` is a
small forwarding hook for providers that expose append-refresh semantics. The
Landmark provider implements that hook for append-only versions by evaluating
only appended records against existing landmarks. The kNN graph provider also
implements it conservatively: old rows are re-ranked only with appended
candidates, appended rows are built by scanning the current snapshot, and the
report separates old-row and appended-row distance counts. This keeps the work
at `old_size * delta + delta * (new_size - 1)` directed evaluations and leaves
the provider current without materializing a dense all-pairs table when admitted.
The refresh plan is preflighted before snapshot mutation; over-budget kNN graph
refreshes return a rebuild-required diagnostic with the estimated counts and
perform zero metric calls. If the append report or live space no longer matches
the provider snapshot, both providers return a rebuild-required diagnostic
before any Metric call. Existing exact tables, blocked providers, and caches
remain explicit rebuild/refresh steps after ingestion.

## Phase 6: Python And User Experience

### P6.1 Safe Defaults In Python

Python should never materialize a huge matrix by accident:

- `space.to_matrix()` runs a budget estimate first.
- exact dense materialization beyond budget raises a clear exception.
- `space.neighbors(...)` should auto-select live/index/approx based on query
  count and budget.
- `space.describe()` samples when exact cubic diagnostics exceed budget.

Implementation status: constructor-level `cache="materialized"` and explicit
matrix export paths refuse over-budget dense materialization before metric calls.
Neighbor and range queries refuse over-budget exact scans before metric calls, or
return bounded non-exact results when `exact=False` is requested. Exact
`RuntimePolicy` instances now carry `max_memory_bytes`,
`max_distance_evaluations`, and `max_dense_records` into `Space.plan(...)`,
`Space.to_matrix()`, `Space.pairwise_distances()`,
`Space.distance_matrix_numpy()`, `Space.neighbors()`, `Space.describe()`, and
direct `pairwise_distance_matrix(...)` / `nearest_neighbors(...)` /
`range_neighbors(...)` helpers. The same policy-budget preflight now protects
`Space.compare()`/`Space.correlate()` and direct `compare_spaces(...)`,
`correlate_spaces(...)`, `describe_structure(...)`, and `intrinsic_dimension(...)`
before native exact profile or diagnostic work starts. Explicit per-call budget
arguments override the policy. They also propagate to exact clustering, outlier,
representative, reduce, compress, medoid, separated/coverage convenience, and
uniform-density resample paths, including direct `kmedoids(...)`, `dbscan(...)`,
`find_outliers(...)`, `representative_indices(...)`, `find_representatives(...)`,
`reduce_space(...)`, `compress_space(...)`, `medoid_index(...)`,
`separated_representative_indices(...)`, `coverage_representative_indices(...)`,
`thin_space(...)`, `uniform_density_sample_space(...)`, and `equalize_space(...)`.
Research-only `metric.experimental` functions now accept the same budget fields,
bound iterator materialization before full `list(...)` conversion, preflight exact
Python-loop work before metric calls, and avoid the former double all-pairs pass
in `distance_distribution_drift(...)`.
Single-query or small-record approximate requests may stay on sampled live
candidates with candidate, sample-fraction, distance-budget, and recall
diagnostics. Repeated or batch source-metric approximate requests
(`query_count > 1`) now auto-select a deterministic Python Landmark/Pivot route
for known native/admitted metrics when the distance and memory budgets admit
bounded candidates; unmarked Python callables stay on sampled search by default.
Passing `exact=False, representation="landmark"` still forces that route. It stores
`O(n * p)` landmark distances, ranks candidates by
lower bounds, refines a bounded candidate set with exact metric calls, marks
results non-exact, and exposes `landmark_index` diagnostics/provenance including
bounded recall calibration against a larger lower-bound reference window with
query-level standard error when the post-refinement distance budget allows it.
Sampled Python search calibration uses the same query-level standard-error
method. Exact
`Space.describe()` over budget refuses before metric calls, while the default
budget-aware describe path returns sampled non-exact structure diagnostics with
sample size, pair fraction, distance evaluations, and average-distance standard
error where available. `cache="auto"` no longer eagerly materializes. Higher
quality ANN backends and cross-language provider tuning beyond the current
sampled/Landmark defaults remain future work.

### P6.2 User-Facing Plan Reports

Expose:

```python
plan = space.plan("neighbors", query_count=1000, exact=False)
print(plan)
```

Example report fields:

- selected representation.
- exactness.
- estimated memory.
- estimated metric evaluations.
- fallback chain.
- how to force exact mode.

## Implementation Order

1. Add budget and cost-estimation structs.
2. Add `RecordId` hash/index maps and position-based hot-loop APIs.
3. Make lazy `DistanceTable` sparse by default.
4. Replace full k-NN sorting with partial selection.
5. Add dense-table construction guards and default budgets.
6. Add provider/context reuse APIs.
7. Add triangular symmetric table.
8. Add approximate diagnostics sampling.
9. Add approximate neighbor provider.
10. Add chunked provider and divide-and-conquer workflows.

## Acceptance Tests

- Constructing a lazy table for a very large space does not allocate `O(n^2)`.
- Constructing an eager dense table over budget fails before allocation.
- Direct default `DistanceTable(space)`, `SymmetricDistanceTable(space)`,
  `space::storage::matrix(space)`, `space::distances::materialize(space)`, and
  `space::storage::make(space, distance_table{})` refuse oversized eager
  materialization before the first metric call, while explicit zero budgets remain
  the unbounded opt-in.
- Legacy `MatrixSpace`/`FiniteSpace` and `GraphSpace` refuse oversized default
  exact matrix builds before metric calls, while `matrix_space_options{0}` and
  `legacy_knn_graph_options{0}` remain explicit unbounded opt-ins for legacy
  users.
- Direct `space::index` helpers avoid hidden `Space::from_records(...)`
  materialization: pairwise matrix output is guarded before allocation,
  nearest/range helpers perform only their exact live scan, exact graph helpers
  preflight directed pair work before metric calls, and shortest-path/pair-index
  helpers refuse oversized dense `O(n^2)`/`O(n^3)` work by default.
- Provider-level `affinity_propagation(provider, ...)` refuses oversized dense
  `n x n` similarity/responsibility/availability matrices before allocation and
  before any provider distance call.
- Provider-level `kmedoids(provider, ...)` and `dbscan(provider, ...)` refuse
  oversized exact clustering work before initialization/region-query loops and
  before any provider distance call.
- Provider-level `find_outliers(provider, dbscan_options)` and
  `nearest_neighbor_outliers(provider, ...)` refuse oversized exact outlier work
  before DBSCAN region-query or nearest-neighbor scoring loops and before any
  provider distance call.
- `cluster_diagnostics(...)` samples oversized defaults with `exact=false` and
  sample metadata; exact-only options refuse before any provider distance call.
- Direct default `KnnGraphIndex(space, k)`, `space::storage::knn_graph(...)`,
  `space::storage::graph(...)`, and `space::storage::make(space, knn_graph{})`
  refuse oversized exact graph builds before the first metric call, while
  `knn_graph_index_options{..., 0}` remains the unbounded opt-in. `k == 0`
  creates an empty graph with zero distance evaluations.
- Direct default `LandmarkIndex(space, ...)` and
  `space::storage::landmark_index(...)` refuse oversized `O(n * p)` projection
  builds by distance-evaluation or memory budget before the first metric call,
  while `landmark_index_options` with zero build/memory budgets remains the
  explicit unbounded opt-in. Policy-driven Landmark search honors explicit
  runtime distance/memory budgets when constructing temporary or shared
  providers.
- Error messages include estimated bytes and suggested fallback.
- Repeated workflow operators can share one materialized provider.
- k-NN with `k=1` does not sort all candidates.
- Direct C++ `farthest_first`, `metric_walk`, and `regular_sample` refuse
  over-budget sample lineage work before any metric call; explicit low
  `max_distance_evaluations` options prove the guard for the adaptive samplers,
  and the default `regular_sample` guard protects large lineage assignment.
- `RecordId` lookup remains correct after insert, replace, erase, subspace, merge,
  save, and load.
- `describe_structure` over budget returns an approximate result or a clear
  refusal, never an accidental cubic run.
- Approximate results are marked non-exact and include quality diagnostics.
- Runtime-limited search/materialization cancels cooperatively before a full
  scan/build completes.
- Runtime-limited structural loops in groups/outliers/density_filter cancel
  cooperatively before completing unbounded scans.
- MGC permutation significance refuses over-budget `permutations * n * n`
  matrix work before dense distance construction or permutation loops; smoke
  coverage verifies zero metric calls for too-small permutation work budgets.
- Large default `local_volume(space, radius)` samples instead of executing all
  `n * n` directed pair evaluations, and reports non-exact quality diagnostics.
- Large default `local_volume_profile(space, radii)` uses one bounded sampled
  distance pass across all radii, marks the profile non-exact, and reports the
  same sample/evaluation diagnostics as sampled local volume.
- Direct provider `local_volume(provider, radius)` follows the same sampled
  default for oversized providers, and exact-only local-volume/profile options
  refuse before distance calls when they exceed `max_exact_records`.
- Exact and chunked multi-radius local-volume profiles cap
  `record_count * radii_count` helper storage and refuse before allocation/pair
  traversal when `max_profile_cells` is exceeded.
- C++ `clustered_space(...)` and `make_clustered_space_mapping(...)` refuse an
  over-budget representative distance table before source-metric calls; smoke
  coverage verifies the singleton-cluster worst case has zero metric calls when
  `max_distance_evaluations` is too low.
- C++ `modify::expand::expand(space)` refuses over-budget exact nearest-neighbour
  midpoint scans before source-metric calls and no longer allocates a
  `record_count * record_count` duplicate-pair matrix.
- Python `metric_graph_spanner(...)` refuses over-budget pair-list
  materialization and repeated shortest-path work before source-metric calls;
  regression coverage checks both memory and work-budget refusals leave the
  counting metric untouched.
- Direct Python `metric.correlation.MGC`, `MGC_direct`, and `Entropy` adapters
  accept `RuntimePolicy`/`max_*` budgets and refuse known-length over-budget
  dense native work before invoking the native binding.
- Benchmarks report both distance evaluations and memory estimates.
- `chunked_view` and workflow planner diagnostics report local chunk work,
  representative merge work, and explicit refinement candidate/evaluation counts.
  Their smoke coverage proves local exact pair iteration/planning and bounded
  representative-refined kNN/range perform `m * O(b^2)` plus representative work
  plus bounded cross-chunk refinement instead of a dense `O(N^2)` all-pairs scan
  for `N = m * b`. Explicit chunked distance-distribution, local-volume, and
  local-volume-profile coverage use local chunk pairs plus representative pairs
  below the same dense-pair baseline, with the profile reusing one bounded pair
  traversal across multiple radii.
- Materialized `compare`/`correlate` with both chunking and approximation
  fallback can select `chunked_space_view`, returns non-exact
  `chunked_mgc_estimate`, preserves original paired record counts, reports the
  bounded sample size/reason, and a counting-metric smoke proves it stays below
  dense compare work.
- Direct exact C++ `stats::correlate::mgc(...)` over large record containers or
  `MetricSpace` inputs refuses before any metric call with a fallback hint to
  `compare(...)`/`mgc_estimate(...)`; `mgc_estimate(...)` with a full-record
  sample over the same large input refuses instead of silently becoming an
  unbounded exact dense MGC run.
- Promoted C++ `stats::properties::entropy(space)` keeps small coordinate spaces
  exact, samples oversized defaults with `exact=false`, deterministic
  `sample_count`/`sample_seed` metadata, and refuses exact-only oversized options
  before any metric call.
- Materialized k-medoids compression with approximation fallback can return
  `sampled_k_medoids` over `sampled_metric_space`, assigns records without a
  dense distance table, and a counting-metric smoke proves observed calls remain
  below the dense all-pairs baseline.
- Exact-lazy `compress(..., policy)` refuses count-based, radius-coverage, and
  k-medoids compression before any metric call when `max_distance_evaluations`
  is too low; accepted count-based exact-live compression stays within the
  planned representative-selection plus assignment budget.
- Radius-coverage compression with approximation fallback can return
  `sampled_radius_coverage` or `chunked_radius_coverage`, uses bounded candidate
  sets before assignment, and a counting-metric smoke proves observed calls
  remain below the dense all-pairs baseline. Large default radius-coverage
  compression also downgrades to `sampled_metric_space` instead of running the
  provider-only full coverage pass.
- `find_representatives`/`represent` with approximation fallback can return
  `sampled_farthest_first`, `sampled_coverage`, `sampled_k_center`,
  `sampled_radius_coverage`, or chunked representative variants, and
  counting-metric smokes prove observed calls remain below the dense all-pairs
  baseline.
- Uniform-density `thin/equalize` resampling with approximation fallback can
  return `sampled_uniform_density_radius_net` or
  `chunked_uniform_density_radius_net`, computes source density diagnostics over
  bounded candidate sets, and a counting-metric smoke proves observed calls
  remain below the dense all-pairs baseline. Large default uniform-density
  sampling also downgrades to `sampled_metric_space` instead of running full
  all-pairs diagnostics.
- Exact-live uniform-density `thin/equalize(..., policy)` refuses too-low
  `max_distance_evaluations` before the first metric call, and an accepted
  budget covers the observed exact radius-net, assignment, nearest-neighbor, and
  local-volume diagnostic calls.
- Streaming `append_batch` preserves existing and newly assigned `RecordId`s,
  reports version/progress/cancellation diagnostics, and a counting-metric smoke
  proves ingestion performs zero distance evaluations instead of hidden dense
  all-pairs work.
- Landmark append refresh consumes a streaming append report, extends only the
  appended rows of the landmark projection matrix, and a counting-metric smoke
  proves refresh performs `delta * landmark_count` evaluations while non-append
  mutations return a rebuild-required diagnostic without evaluating distances.
- kNN graph append refresh consumes a streaming append report, preflights
  `old_size * delta + delta * (new_size - 1)` directed evaluations, refuses
  low-budget refreshes before any metric call while leaving the stale graph
  snapshot unchanged, and admits bounded refreshes with exact distance-count
  diagnostics.
- The benchmark report smoke keeps automated target/trend/guardrail rows for
  one-off kNN, batch kNN, repeated provider reuse, lazy sparse cache hit/miss,
  sampled describe, chunked workflow, sampled distance distribution, and
  out-of-core readiness/refusal evidence. The rows use `CountingMetric` and
  prove bounded smoke-sized work instead of dense exact `O(n^2)`, repeated
  `O(m * n^2)`, or full-pair baselines.
- Optional wall-clock trend report rows carry workload, record count,
  representation, elapsed-ns, distance-evaluation, memory-estimate, sample-count,
  platform-label, automated, passed, and notes fields. CTest smoke coverage uses
  one explicitly synthetic metadata row and one measured `std::chrono` row for a
  small distance-table build, asserting only nonzero `elapsed_ns` and report
  fields, not a real performance threshold.
- Landmark/Pivot search over a true metric reports `landmark_index`, builds in
  `O(n * p)`, refines only a bounded candidate set, marks results non-exact,
  reports bounded holdout recall calibration with query-level standard error or
  a concrete skip reason, and an execution-context smoke proves repeated queries
  reuse the same landmark provider instead of rebuilding it per call.
- Python sampled and Landmark search calibration report query-level recall
  standard error and 95% confidence radius when the calibration window fits the
  remaining distance budget.
- Python automatic Landmark routing is conservative: known native/admitted
  metrics can auto-select Landmark/Pivot for repeated bounded queries, while
  unmarked Python callables fall back to sampled search unless Landmark is
  explicitly requested.
- Python explicit Landmark routing refuses when the planned `O(n * p)`
  landmark-distance storage exceeds `max_memory_bytes`, before building the
  Python Landmark index or calling the metric.
- Python explicit Landmark tests prove `max_memory_bytes` refusal is surfaced in
  `Space.plan(...)` and `Space.neighbors(...)` with zero metric calls.
- Python exact `RuntimePolicy` budgets propagate into `Space.plan(...)`, dense
  matrix materialization/export, exact neighbor/range/describe/compare guards,
  intrinsic-dimension guards, clustering/outlier/representative/reduce/compress
  guards, medoid/separated/coverage convenience guards, uniform-density resample
  guards, research-only experimental diagnostic guards, and direct
  pairwise/search/diagnostic/modify helper functions.
  Counting-metric tests prove low policy budgets refuse before any metric call,
  pure regular-thinning paths avoid metric calls, accepted radius-compression and
  uniform-density budgets keep observed calls under the budget, derived spaces do
  not inherit strict validation/materialized cache work, and explicit per-call
  budgets override policy fields.
- Python direct helper tests prove oversized iterators are not fully converted via
  `list(...)` before refusal for pairwise matrix export, `Space.pairwise(ids=...)`,
  exact and sampled live nearest/range search, DBSCAN/outlier/density filtering,
  uniform-density resampling, regular distribution-preserving thinning,
  radius-coverage compression, medoid, separated-representative, and
  coverage-representative helpers.
- Python `Space(...)` constructor tests prove oversized unsized record iterators
  refuse at the dense-record budget and do not call the metric.
- Research-only `metric.experimental` functions refuse oversized exact
  distribution, drift, kNN recall, graph spanner, hierarchical net,
  density-equalization, metric-measure drift, and density-quotient work before
  any metric call. Iterator tests prove `max_dense_records` stops record
  materialization early, oversized hierarchical radii iterators stop at the
  work/materialization budget, oversized metric-measure weight iterators stop at
  the record-count mismatch sentinel, graph spanner accounts for pair-list memory
  and follow-on shortest-path work before metric calls, and sampled distance
  distribution remains available within a low distance-evaluation budget.
- Python parametric-diffusion mapping adapters refuse oversized dense target
  construction before native binding calls. `Space.map(strategy=...)`, direct
  `derive_parametric_diffusion_coordinates(...)`, and
  `derive_mapping_pipeline(...)` accept `RuntimePolicy`/`max_*` budgets, while
  the native binding receives admitted dense-record, memory, and
  distance-evaluation limits when the installed extension supports those
  keywords. Compatibility retry tests prove unsupported budget keywords are
  removed narrowly and validation `TypeError`s are not swallowed.
- Native `DiffusionCoordinateSpec`/`DiffusionOptions` enforce
  `max_dense_records`, `max_memory_bytes`, and `max_distance_evaluations` before
  building dense diffusion target matrices, and artifact manifests include those
  budget fields for auditability.
- `intrinsic_dimension_from_distances(...)` refuses oversized precomputed dense
  distance input by `max_dense_records` or `max_memory_bytes` before materializing
  `list(distances)`, and fake list-like tests prove refusal happens without
  iterating the rows.
- Mapping `neighbor_preservation` and `out_of_sample_neighbor_stability`
  diagnostics stay exact for small inputs, but default large runs deterministically
  sample rows/queries within `max_distance_evaluations`, mark results non-exact,
  report the sampled representation/reason, and counting-metric smokes prove
  observed source plus mapped evaluations remain under the default diagnostic
  budget.
- Python `mappings.clustered_space(...)`,
  `make_clustered_space_mapping(...).derive_from(...)`, and
  `mappings.derive_from(...)` accept runtime/budget overrides and refuse the
  cluster representative distance matrix before any source-metric calls or
  clustering-assignment iterator materialization when `max_distance_evaluations`,
  `max_memory_bytes`, or `max_dense_records` is too low. Representative SourceIds
  are resolved through an indexed lookup per derivation, not a repeated linear
  `ids.index(...)` scan. Additional generator tests prove overlong assignment
  and medoid sequences are stopped at the first over-budget element before any
  source metric call.
- Python `compare_spaces(... matched_ids=iterator)` and
  `compare_spaces(... dropped_left_ids/dropped_right_ids=iterator)` refuse
  oversized ID iterators before invoking the native correlation binding and
  before any metric call.
- Python `Space.from_csv(...)`, `Space.from_tsv(...)`,
  `Space.from_numpy_file(...)`, and `Space.from_dataframe(...)` refuse oversized
  sources before native file import, NumPy `tolist()`, or DataFrame `to_dict(...)`
  materializes the full input. `intrinsic_dimension_from_distances(...)` tests
  prove overlong row generators are stopped before native conversion.
- Research-only Python `distance_distribution_sketch(...)` and
  `distance_distribution_drift(...)` now have memory-budget tests proving value
  vector materialization refuses before the first metric call; their ordinal
  generation is lazy rather than `tuple(range(pair_count))`.
- C++ direct representative provider tests prove farthest-first, radius coverage,
  and medoid helpers refuse over-budget provider work with zero distance calls.
  Runtime-policy representative tests cover count-based and radius-coverage
  underestimates, and `representative_indices(...)` tests include the follow-on
  sample-assignment phase.
- C++ uniform-density resample tests prove `estimate_cost(..., "resample",
  policy)` rejects an old `n * n`-sized exact-live budget before metric calls,
  while the conservative accepted estimate keeps observed exact diagnostic calls
  under `max_distance_evaluations`.
- C++ local-volume provider tests prove oversized provider defaults sample below
  dense directed all-pairs work, and exact-only `local_volume`/
  `local_volume_profile` options refuse before provider distance calls.
- C++ chunked local-volume-profile tests prove a too-small `max_profile_cells`
  refuses before chunk pair traversal or metric calls, preventing bounded pair
  work from hiding an unbounded profile helper allocation.
- Python `Space.map(transform=...)` and direct `map_space(...)` refuse oversized
  record/source-id materialization before invoking the caller transform, and
  generator tests prove `max_dense_records` stops after the first over-budget
  record.
- kNN graph `quality_against(provider)` keeps exact validation for small graphs
  but samples large graphs when `sample_count` is omitted, reports the requested
  sample count and sampled flag, and a counting-metric smoke proves validation work
  is bounded below full-row quality comparison.

## Benchmark Targets

Automated CTest smoke rows now cover:

- one-off k-NN query over large `n`.
- batch k-NN with many queries.
- repeated operators over the same provider.
- default distance-distribution sampling over large `n`.
- lazy sparse cache hit/miss behavior.
- describe/intrinsic-dimension exact vs sampled.
- chunked workflow, chunked compare, planner-dispatched RecordId chunked search,
  and explicit representative-refined kNN/range with `N = m * b` compared to
  full exact `O(N^2)`.
- sampled/chunked representative selection and default large `represent(...)`
  compared to dense all-pairs work.
- sampled/chunked uniform-density resampling compared to dense all-pairs
  diagnostic work.
- sampled mapping neighbor-preservation and out-of-sample diagnostics compared to
  full row/query scans.
- direct kNN-graph build, kNN-graph append-refresh, and LandmarkIndex build
  refusals plus sampled graph quality diagnostics compared to full graph
  build/quality scans.
- Landmark/Pivot k-NN/range compared to dense all-pairs evaluation budgets.
- an explicitly synthetic wall-clock metadata row plus a small measured
  `std::chrono` wall-clock row, both without claiming timing performance.
- out-of-core readiness for spill-disabled dense materialization, including
  memory pressure, planned spill block count, explicit policy requirement, and
  refusal-before-metric-calls evidence. The blocked-table storage smoke covers
  reloads from disk without additional metric calls and quota refusal before a
  second spill write, while runtime-policy smoke checks planner/diagnostics
  spill fields without pre-execution metric calls.

Still track in the broader benchmark suite:

- dense exact table build at increasing `n`.
- multi-platform wall-clock trend/perf samples for representative metrics and
  larger workload sizes.

## Open Decisions

- Broader runtime-policy fallback alignment for any future Python adapters that
  still require exact native or Python-loop work.
- Whether native parametric-diffusion should accept a shared `resource_budget`
  object directly instead of parallel scalar budget fields in `DiffusionOptions`
  and pybind signatures.
- How far the in-core spill provider should go beyond the current block/count,
  byte-estimate diagnostics, and hard byte quota versus optional advanced
  out-of-core extensions with placement policies, compression, mmap, or
  platform-specific tuning.
- How to tune Landmark/Pivot automatic approximate defaults and extend recall
  confidence intervals across Python, metric families, and workload sizes.
- How much automatic downgrade is acceptable in C++ defaults versus Python
  convenience APIs.
- Whether guarded legacy `MatrixSpace`/`GraphSpace` should remain long-term
  compatibility APIs, gain deprecation warnings, or get migration shims toward
  promoted providers.
