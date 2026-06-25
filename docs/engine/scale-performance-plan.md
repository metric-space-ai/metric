# Scale-Safe Performance Plan

METRIC should remove the operational pain of finite metric spaces. When a caller
hands it a large data set, it must not blindly allocate an `O(n^2)` table or enter
an unbounded `O(n^3)` diagnostic path. The runtime should choose a safe exact
plan when possible, downgrade to an explicit approximation when needed, and fail
early with a useful diagnostic only when no safe plan exists.

This plan targets the promoted `mtrc::space`, `mtrc::stats`, and `mtrc::modify`
surface. Legacy containers can be optimized opportunistically, but the promoted
finite-space API is the priority.

## Current Implementation Status (2026-06-25)

The first implementation waves have landed the crash-prevention guardrails for
the promoted finite-space surface. Treat the status below as the working ledger:
bounded sampled/blocking paths are in place, and C++ search now has an explicit
Landmark/Pivot approximation that builds in `O(n * p)` instead of all-pairs.
Python now has an initial automatic Landmark route for repeated/batch
approximate source-metric search, and blocked exact storage has an explicit
disk-spill mode for evicted blocks with an optional hard spill-byte quota.
Materialized search/range, for both
`RecordId` and free query objects, can now dispatch bounded `chunked_space_view`
refinement when both chunking and approximate fallback are allowed.
Higher-quality ANN variants, automatic provider tuning beyond the current
sampled/Landmark selector, advanced out-of-core placement/tuning beyond the
conservative spill-byte quota, and a broader wall-clock benchmark suite remain
roadmap extensions rather than prerequisites for avoiding `O(n^2)` crashes.
Confidence diagnostics now cover sampled distribution/local-volume paths and
chunked local-volume/profile fallbacks, though provider-specific calibrated
intervals remain a quality extension.

| Area | Status | Evidence |
| --- | --- | --- |
| Dense budget guards | Implemented for promoted crash paths | `resource_budget`, `execution_plan`, `estimate_cost`, dense `DistanceTable` budget refusals, materialized preflight guards, C++ search/range/outlier distance-evaluation budget preflight including kNN-graph build costs, blocked exact fallbacks for materialized search/range/compare and structural/modify operators, cooperative `max_runtime_ms` guards for C++ storage/search/context paths, guarded structural loops in groups/outliers/density_filter, Python dense materialization refusals, Python strict-validation budget refusal, and Python query-budget guards exist. |
| Lazy storage | Implemented | Lazy `DistanceTable` uses sparse directed cache storage and keeps construction memory bounded. |
| RecordId lookup | Implemented | Metric spaces, distance tables, and snapshots use indexed RecordId lookup. |
| Position hot loops | Implemented for promoted hot paths | Live/table providers, matrix export, describe sampling, k-NN/range provider paths, k-medoids, DBSCAN, DBSCAN outlier scoring, and nearest-neighbor outlier scoring use position-based distance access when the provider exposes it, with ID-based fallback for generic providers. |
| k-NN selection | Implemented for core path | `take_nearest_neighbors` uses a direct min scan for `k == 1` and `nth_element` for small prefixes. |
| Workflow reuse | Implemented for current providers | `space::execution_context` shares live, dense, symmetric materialized, blocked materialized, or Landmark/Pivot providers across operators. A reusable `RegularSamplePlan` centralizes deterministic sampled-candidate routing for search/compress, approximate sampled contexts cache the keyed sample plan across `neighbors()` and `range()` calls, and landmark contexts build the `O(n * p)` provider once for repeated `neighbors()`/`range()` calls. The approximate context now uses the same sampled-vs-Landmark selector as `estimate_cost`, with a reuse-oriented query horizon, so small metric spaces stay sampled while large repeated metric search reuses Landmark/Pivot. |
| Safer exact storage | Implemented for dense, blocked, and explicit spill-capable exact providers | `SymmetricDistanceTable` stores only triangular off-diagonal slots for admitted symmetric metrics and is used by `execution_context`. `BlockedDistanceTable` provides lazy block-pair materialization, optional LRU block spill to disk, optional hard spill-byte quota enforcement, and is planner-selected for materialized search/range/compare, workflow contexts, and structural/modify operators that opt into exact chunking fallback. |
| Index/graph naming | Implemented for current fallbacks | Exact cover-tree and k-NN graph policies keep their index representation names. Budget downgrades that actually execute sampled search now report `sampled_metric_space` or `metric_space_sample`, not a placeholder index name. |
| Large-data defaults | Implemented for promoted defaults plus guarded matrix export | Default `find_groups(space, ...)`, `find_outliers(space, ...)`, `nearest_neighbor_outliers(space, ...)`, `density_filter(space, ...)`, `compress(space, ...)`, `thin(space, uniform_density(...))`, `equalize(space, uniform_density(...))`, `compare(space, space)`, `correlate(space, space)`, `distance_distribution(space, ...)`, `local_volume(space, ...)`, `local_volume_profile(space, ...)`, `profile(...include_local_volume...)`, direct low-level clustering APIs, diffusion/diffusion-coordinate target helpers, explicit `space::distances::pairs(...)` collection, low-level dense matrix conversion helpers, dense matrix export helpers, and `intrinsic_dimension(space)` now avoid the worst unbounded exact paths. Dense-only MGC significance refuses large default `MetricSpace` inputs before metric calls. |
| Approximation | Implemented with sampled, landmark, and chunked quality diagnostics | `describe_structure`, `diagnose_space`, `compare`, `find_neighbors`, `range`, `find_groups`, `find_outliers`, `nearest_neighbor_outliers`, `density_filter`, `compress`, `thin/equalize` uniform-density resampling, `distance_distribution`, and `local_volume` have explicit non-exact sampled fallbacks where appropriate. Default large `compare`/`correlate` uses bounded `mgc_estimate` instead of dense exact MGC; `mgc_significance` still requires dense exact pairwise distances and therefore preflight-refuses large `MetricSpace` defaults rather than pretending a sampled p-value is calibrated. C++ `find_neighbors`/`range` can opt into `using_landmark_index(...)`, which stores landmark distances, uses triangle-inequality lower bounds for bounded candidates, and reports non-exact Landmark diagnostics with bounded holdout recall calibration. Search exposes sampled candidate diagnostics and bounded recall calibration in C++ and Python when budget permits, including recall standard error and 95% confidence radius; distribution, sampled local-volume, and chunked local-volume/profile fallbacks report diagnostic standard error and 95% confidence radius; sampled kNN outliers report sample/candidate/evaluation diagnostics. Higher-quality ANN and provider-specific calibrated intervals remain future improvements. |
| Divide and conquer | Explicit chunked execution plus bounded search/distribution/local-volume/compare/compress/resample paths implemented | `chunked_view` preserves RecordIds, exposes bounded local pair iteration, per-chunk representatives, representative-pair iteration, and plan diagnostics that compare local chunk work against dense all-pairs work. It executes local chunk kNN/range, representative kNN, representative candidate generation, and bounded representative-refined kNN/range through explicit APIs. `estimate_cost`/runtime diagnostics surface a `chunked_workflow_plan` for workflow intents that opt into chunking fallback, including dense-pair baseline and bounded local-plus-representative work. Materialized `find_neighbors`/`range` requests with both `allow_chunking` and `allow_approximate` can now dispatch bounded `chunked_space_view` refinement instead of building a dense table; `RecordId` search uses representative-refined chunk pairs, while free query objects rank representatives and refine only selected chunks. `chunked_distance_distribution(...)`, `chunked_local_volume(...)`, and `chunked_local_volume_profile(...)` provide explicit bounded diagnostics over local chunk pairs plus representative pairs; chunked local-volume/profile include diagnostic standard error and 95% confidence radius, and the profile route reuses one bounded pair traversal for multiple radii. Materialized `compare(..., policy)` can now downgrade to a bounded non-exact `chunked_space_view` MGC estimate; materialized count-based `compress(..., farthest_first/coverage/k_center, policy)` and radius-coverage compression can downgrade to non-exact `chunked_space_view` or `sampled_metric_space`; materialized k-medoids compression can fall back to sampled medoids plus bounded live assignment when approximation is allowed; and materialized uniform-density `thin/equalize` resampling can downgrade to sampled or chunked radius-net candidates with bounded diagnostics. Exact-only chunking still uses blocked exact fallback. Broader automatic placement for remaining modify/resample strategies remains future work. |
| Streaming construction | Append API plus conservative Landmark and kNN graph refresh implemented | `space::streaming::append_batch` appends records directly to a live `MetricSpace`, keeps RecordIds monotonic/stable across erases, reports versions/progress/cancellation, and explicitly records zero distance evaluations and no dense all-pairs materialization. `LandmarkIndex::refresh_after_append(...)` can extend an append-only snapshot by projecting only appended records onto the existing landmark set. `KnnGraphIndex::refresh_after_append(...)` integrates appended records by updating old rows only against appended candidates and building rows for the appended records, so the refresh does `O(delta * n)` directed distance work instead of a hidden dense rebuild. Both providers return rebuild-required diagnostics when the source space is no longer append-only. Exact tables, blocked providers, and caches remain explicit rebuild/refresh steps. |
| Benchmarks | Smoke-level targets/trends/guardrails implemented | Benchmark reports include distance-evaluation and memory fields plus automated target, trend, scale-guardrail, out-of-core readiness, and optional wall-clock trend tables. `benchmark_report_smoke` runs under CTest and exercises one-off kNN, batch kNN, repeated provider reuse, lazy sparse cache hit/miss, sampled describe, chunked workflow, sampled distance-distribution rows, spill-disabled refusal evidence with observed `CountingMetric` budgets, one explicitly synthetic wall-clock metadata row, and one small measured `std::chrono` wall-clock row with no performance threshold. Disk-spill execution is covered in the blocked-table storage smoke. A broader multi-platform wall-clock trend/perf benchmark suite remains future work. |
| Python UX | Implemented for sampled safe defaults plus explicit and automatic Landmark routing | `Space.plan(...)`, `Space.describe_plan(...)`, dense materialization preflight, sampled over-budget `describe()` with diagnostics, neighbor/range query-budget guards, `exact=False` sampled neighbor/range routing, sampled candidate diagnostics, bounded recall calibration, and Landmark/Pivot search exist. `exact=False, representation="landmark"` forces Landmark/Pivot search, while repeated or batch source-metric Python search (`query_count > 1`) now auto-selects `landmark_index` when budgets and record count admit bounded candidates. The Python Landmark route builds in `O(n * p)`, refines bounded lower-bound candidates, marks results non-exact, and reports `landmark_index` diagnostics/provenance, including bounded recall calibration against a larger lower-bound reference window when the post-refinement budget permits it. Broader ANN tuning remains future work. |

Remaining risk posture and roadmap:

- Large default `compress(space, count)` now uses bounded sampled assignment for
  high target counts and identity compression for `count == n`.
- Explicit pair collection through `space::distances::pairs(...)` now has a
  default in-memory pair budget; unbounded streaming remains available through
  `for_each_pair(...)`.
- `FiniteSpace::pairwise_distances(...)` and
  `space::index::pairwise_distance_matrix(...)` now guard dense `n x n` matrix
  output before allocation; the convenience operator preflights before building
  the matrix-backed space.
- Low-level `provider_dense_distance_matrix(...)`,
  `provider_symmetric_distance_matrix(...)`, and
  `metric_space_dense_distance_matrix(...)` now guard dense matrix conversion
  before allocation while keeping explicit `dense_distance_matrix_options{0}` as
  the unbounded opt-in.
- Promoted `space::storage::using_knn_graph(...)` planning now charges the full
  `n * (n - 1)` directed index build plus query work before constructing
  `KnnGraphIndex`, so low distance-evaluation budgets refuse before metric
  calls instead of underestimating the build as one live scan.
- Legacy exact graph helpers now guard exhaustive graph construction and
  stretch diagnostics: `exact_knn_graph(...)` and `exact_radius_graph(...)`
  refuse over-budget directed pair work before metric calls, radius construction
  no longer materializes a full pair-index list, and
  `graph_stretch_diagnostics(...)` preflights both direct metric comparisons and
  dense all-pairs shortest-path storage/closure.
- Python `validate="strict"` now preflights its full `n * n` distance scan with
  `max_distance_evaluations` and `max_dense_records`, so constructor validation
  can refuse before the first metric call on large inputs.
- Sampled distribution, nearest-neighbor outliers, compare/correlate, sampled
  local-volume defaults, chunked compare, chunked local-volume/profile, chunked
  compress, and sampled/chunked uniform-density resampling now expose bounded
  diagnostics/routes; MGC estimate and provider-specific ANN routes still report
  coarser quality than search recall calibration.
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
| Dense `DistanceTable` | Allocates `n * n` optional cells, even in lazy mode | Large spaces can exhaust RAM before doing useful work |
| Linear `RecordId` lookup | `position_of(id)` scans the ID vector | Pairwise loops become much slower than their algorithmic label suggests |
| Full sorting for small k | k-NN sorts all candidates | `k=1` still pays `O(n log n)` instead of selection cost |
| Per-call materialization | Runtime policies build tables/indexes inside each operator call | Workflows repeat the same `O(n^2)` build |
| Unbounded diagnostics | `describe_structure`, default `intrinsic_dimension`, `distance_distribution`, `local_volume`, and `local_volume_profile` now use sampling or shared scans when exact work exceeds their default threshold | Provider-specific calibrated intervals and broader quality reporting remain improvements |
| Approximation surface | Core structural/search/modify paths have sampled fallbacks and search recall calibration; C++ search also has an explicit Landmark/Pivot candidate provider with bounded holdout calibration; calibrated search recall, sampled distribution/local-volume, and chunked local-volume/profile report standard error and 95% confidence radius | Some large workflows still fail or return coarse approximations instead of bounded, measured degradation |
| Snapshot lookup | Snapshot cells are searched linearly | Persisted/materialized artifacts are expensive to query as providers |
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
error and 95% confidence radius. Remaining work is higher-quality ANN selection
and provider-specific calibrated intervals beyond the current sampled, chunked,
and Landmark holdout diagnostics.

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
selected representatives, avoiding dense materialization when approximation is
explicitly allowed. Materialized k-medoids compression can also avoid a dense
assignment provider by falling back to sampled medoids plus bounded live
assignment when approximation is explicitly allowed. Uniform-density
`thin/equalize` resampling can avoid full-source radius-net diagnostics by using
sampled candidates or chunk representatives, then assigning records only against
that bounded representative set. Remaining work is broader automatic chunked
workflow placement beyond the current search, distribution, local-volume,
compare, count-based compression, radius-coverage compression, sampled
k-medoids compression, and uniform-density resampling paths, especially other
modify/resample strategies.

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
candidate evaluations, matched/reference counts, and recall estimate without
building a dense all-pairs table. If calibration is skipped, the result reason
states the guard, such as `k=0`, an empty reference window, or
`max_distance_evaluations`. `space::execution_context` reuses the same landmark
provider across repeated search calls, and automatic approximate contexts use
the same sampled-vs-Landmark selector as `estimate_cost` with a reuse-oriented
query horizon. Multi-query `estimate_cost(..., query_count, approximate())` also
plans `landmark_index` for admitted metrics only when the calibrated Landmark
estimate is no worse than sampled search.
Implicit Landmark candidate and landmark counts are derived conservatively from
`n`, query count, and budget while explicit `using_landmark_index(...)`
candidate limits are preserved. A conservative append-only maintenance surface,
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
the provider current without materializing a dense all-pairs table. If the
append report or live space no longer matches the provider snapshot, both
providers return a rebuild-required diagnostic before any Metric call. Existing
exact tables, blocked providers, and caches remain explicit rebuild/refresh
steps after ingestion.

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
return bounded non-exact results when `exact=False` is requested. Single-query
or small-record approximate requests may stay on sampled live candidates with
candidate, sample-fraction, distance-budget, and recall diagnostics. Repeated or
batch source-metric approximate requests (`query_count > 1`) now auto-select a
deterministic Python Landmark/Pivot route when the distance and memory budgets
admit bounded candidates. Passing `exact=False, representation="landmark"` still
forces that route. It stores `O(n * p)` landmark distances, ranks candidates by
lower bounds, refines a bounded candidate set with exact metric calls, marks
results non-exact, and exposes `landmark_index` diagnostics/provenance including
bounded recall calibration against a larger lower-bound reference window when
the post-refinement distance budget allows it. Exact
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
- Error messages include estimated bytes and suggested fallback.
- Repeated workflow operators can share one materialized provider.
- k-NN with `k=1` does not sort all candidates.
- `RecordId` lookup remains correct after insert, replace, erase, subspace, merge,
  save, and load.
- `describe_structure` over budget returns an approximate result or a clear
  refusal, never an accidental cubic run.
- Approximate results are marked non-exact and include quality diagnostics.
- Runtime-limited search/materialization cancels cooperatively before a full
  scan/build completes.
- Runtime-limited structural loops in groups/outliers/density_filter cancel
  cooperatively before completing unbounded scans.
- Large default `local_volume(space, radius)` samples instead of executing all
  `n * n` directed pair evaluations, and reports non-exact quality diagnostics.
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
- Materialized k-medoids compression with approximation fallback can return
  `sampled_k_medoids` over `sampled_metric_space`, assigns records without a
  dense distance table, and a counting-metric smoke proves observed calls remain
  below the dense all-pairs baseline.
- Radius-coverage compression with approximation fallback can return
  `sampled_radius_coverage` or `chunked_radius_coverage`, uses bounded candidate
  sets before assignment, and a counting-metric smoke proves observed calls
  remain below the dense all-pairs baseline.
- Uniform-density `thin/equalize` resampling with approximation fallback can
  return `sampled_uniform_density_radius_net` or
  `chunked_uniform_density_radius_net`, computes source density diagnostics over
  bounded candidate sets, and a counting-metric smoke proves observed calls
  remain below the dense all-pairs baseline. Large default uniform-density
  sampling also downgrades to `sampled_metric_space` instead of running full
  all-pairs diagnostics.
- Streaming `append_batch` preserves existing and newly assigned `RecordId`s,
  reports version/progress/cancellation diagnostics, and a counting-metric smoke
  proves ingestion performs zero distance evaluations instead of hidden dense
  all-pairs work.
- Landmark append refresh consumes a streaming append report, extends only the
  appended rows of the landmark projection matrix, and a counting-metric smoke
  proves refresh performs `delta * landmark_count` evaluations while non-append
  mutations return a rebuild-required diagnostic without evaluating distances.
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
  reports bounded holdout recall calibration or a concrete skip reason, and an
  execution-context smoke proves repeated queries reuse the same landmark
  provider instead of rebuilding it per call.

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
- sampled/chunked uniform-density resampling compared to dense all-pairs
  diagnostic work.
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

- Default memory budget value for C++ and Python.
- How far the in-core spill provider should go beyond the current block/count,
  byte-estimate diagnostics, and hard byte quota versus optional advanced
  out-of-core extensions with placement policies, compression, mmap, or
  platform-specific tuning.
- How to tune Landmark/Pivot automatic approximate defaults and recall
  confidence intervals across C++, Python, metric families, and workload sizes.
- How much automatic downgrade is acceptable in C++ defaults versus Python
  convenience APIs.
- Whether `MatrixSpace` should be preserved as-is, deprecated, or routed
  through the promoted providers.
