# Scale-Safe Performance Plan

METRIC should remove the operational pain of finite metric spaces. When a caller
hands it a large data set, it must not blindly allocate an `O(n^2)` table or enter
an unbounded `O(n^3)` diagnostic path. The runtime should choose a safe exact
plan when possible, downgrade to an explicit approximation when needed, and fail
early with a useful diagnostic only when no safe plan exists.

This plan targets the promoted `mtrc::space`, `mtrc::stats`, and `mtrc::modify`
surface. Legacy containers can be optimized opportunistically, but the promoted
finite-space API is the priority.

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
- Do not optimize legacy APIs before the promoted surface is protected.

## Current Risk Map

| Risk | Current behavior | Failure mode |
| --- | --- | --- |
| Dense `DistanceTable` | Allocates `n * n` optional cells, even in lazy mode | Large spaces can exhaust RAM before doing useful work |
| Linear `RecordId` lookup | `position_of(id)` scans the ID vector | Pairwise loops become much slower than their algorithmic label suggests |
| Full sorting for small k | k-NN sorts all candidates | `k=1` still pays `O(n log n)` instead of selection cost |
| Per-call materialization | Runtime policies build tables/indexes inside each operator call | Workflows repeat the same `O(n^2)` build |
| Unbounded diagnostics | `describe_structure` and intrinsic dimension can enter cubic work | "Inspect this space" can become the most expensive operation |
| Approximation surface | Approximate policies are mostly rejected | Large exact requests fail instead of degrading usefully |
| Snapshot lookup | Snapshot cells are searched linearly | Persisted/materialized artifacts are expensive to query as providers |
| Graph/index naming | Some graph strategies build expensive structures but still scan for external queries | Users pay index build cost without query-speed benefit |

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
};
```

Default behavior:

- Dense `DistanceTable` refuses construction when estimated bytes exceed budget.
- Error message reports `records`, `estimated_bytes`, `budget_bytes`, and the
  suggested safer representation.
- Policies can opt into `allow_approximate` and `allow_chunking`.

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
- neighbor index.
- sampled sketch.

High-level operators should accept either a `Space` or a provider/context.

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

### P3.3 Exact Tree/Metric Index Contracts

Clarify and harden tree/index behavior:

- cover tree only for admitted true metrics.
- no index build for one external query unless the planner predicts reuse.
- indexed strategy should not build a graph and then perform a full scan unless
  the diagnostic says so.

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
- optional cross-chunk candidate generation via pivots/index.

### P5.2 Pivot And Landmark Distances

Use landmarks to avoid all-pairs evaluation:

- pick pivots by farthest-first or reservoir sampling.
- store `O(n * p)` distances.
- use triangle inequality bounds for pruning.
- use embeddings/sketches for candidate generation, then exact metric refinement.

For true metrics, this creates a path toward logarithmic or subquadratic query
behavior without changing record semantics.

### P5.3 Streaming Construction

Support ingestion APIs that never require the full all-pairs matrix:

- append batches.
- update indexes incrementally where possible.
- rebuild expensive exact structures only when requested.
- provide progress and cancellation hooks.

## Phase 6: Python And User Experience

### P6.1 Safe Defaults In Python

Python should never materialize a huge matrix by accident:

- `space.to_matrix()` runs a budget estimate first.
- exact dense materialization beyond budget raises a clear exception.
- `space.neighbors(...)` auto-selects live/index/approx based on query count and
  budget.
- `space.describe()` samples when exact cubic diagnostics exceed budget.

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
- Benchmarks report both distance evaluations and memory estimates.

## Benchmark Targets

Track these cases continuously:

- one-off k-NN query over large `n`.
- batch k-NN with many queries.
- repeated operators over the same provider.
- dense exact table build at increasing `n`.
- lazy sparse cache hit/miss behavior.
- describe/intrinsic-dimension exact vs sampled.
- chunked workflow with `N = m * b` compared to full exact `O(N^2)`.

## Open Decisions

- Default memory budget value for C++ and Python.
- Whether out-of-core spill belongs in core or an optional extension.
- Which approximate neighbor backend should be first.
- How much automatic downgrade is acceptable in C++ defaults versus Python
  convenience APIs.
- Whether legacy `MatrixSpace` should be preserved as-is, deprecated, or routed
  through the promoted providers.
