# Pipeline Interoperability Implementation Plan

Status: implementation plan
Scope: framework and API interoperability
Non-scope: performance engineering, large-scale approximate indexing, benchmark tuning

## Current Progress

Implemented:

- Lane 0: this plan exists and is linked from `docs/engine/index.md`.
- Lane 1: core pipeline-domain concepts exist:
  `RecordMetricSpaceLike`, `PairwiseDistanceProviderLike`,
  `CoordinateRecordLike`, `CoordinateMetricLike`, `CoordinateSpaceLike`,
  `MappingResultLike`, and `CoordinateMappingResultLike`.
- Lane 3: `NeighborSet` carries additive search provenance, and in-sample
  `MappingResult` search/range routes source `RecordId` queries through mapped
  target spaces while reporting mapping provenance. Direct strategy paths
  (`exact_scan`, `distance_table`, `cover_tree`, `knn_graph`, runtime policies)
  now receive the same route annotation as factory-built results. Search
  provenance keeps the existing route string and adds structured route/domain
  enums so downstream code can distinguish source-metric, pairwise-provider,
  neighbor-index, sampled, and mapped-space results without parsing strings.
  Mapped search also carries per-neighbor source lineage for many-to-one mapping
  results. Provider-ID batches, records+metric batches, and MappingResult-ID
  batches are adapters over the single-query paths and preserve the same
  provenance.
  `MappingResult` search is lineage-only: raw source-record out-of-sample
  queries require a future derived-artifact API, not the result artifact.
- Lane 4: public entropy is coordinate-space/mapping-result oriented; direct
  `entropy(records, source_metric)` is unavailable. `entropy(MappingResult)`
  reports the consumed mapping/derived-space representation instead of the
  mapping algorithm name. Coordinate-space admission is a conservative
  `CoordinateMetricLike` policy: coordinate-shaped records also need an
  explicitly admitted coordinate-neighborhood metric family, not merely any
  metric law.
- Lane 5: `MappingResult` now has a reusable runtime contract guard. Mapped
  search/range and mapped entropy reject malformed lineage/provenance before
  consuming a mapping result. The guard rejects blank metadata, empty
  per-target lineage rows, row-local duplicate source IDs, representative IDs
  outside their own source-lineage row, unique lineage counts above the declared
  source count, and non-empty derived spaces with zero source count. It
  deliberately does not reject duplicate source IDs across target rows,
  duplicate representative IDs, or sparse `RecordId` values; those are valid for
  generated/expanded records and source spaces that have erased records.
- Lane 6: C++ `compare`/`correlate` now expose provider and records+metric
  convenience overloads over existing MGC implementations, and
  `mgc_significance` has a records+metric convenience path. Python remains
  intentionally narrower and documents MGC as unpromoted there.
- Lane 8: C++ docs and examples no longer present entropy as a direct
  arbitrary-source-metric shortcut. The condition-monitoring example now maps
  the healthy TWED source space to an explicit coordinate view before entropy.
- Lane 7: promoted Python `MappingResult` producers now carry
  `metric_status`, `out_of_sample_supported`, and `validity` metadata. Python
  docs and `metric.correlation.Entropy` examples state that entropy is
  not a promoted `Space` source-metric shortcut. Python `NeighborResult` now
  carries search route and provenance metadata, `Space.neighbors(...,
  representation=...)` preserves the explicit representation name, and searches
  on mapped Python spaces can report mapping context without claiming an
  approximate backend.
- Unsupported public combinations are gated through positive SFINAE/static
  assertions and runtime rejection tests in the existing smoke-test style. No
  dedicated compile-fail fixture is introduced for this tranche.

New smoke tests:

- `metric_pipeline_contracts_smoke`
- `search_pipeline_interoperability_smoke`

Known next work:

- keep docs/tests aligned as new mapping families are promoted; records+metric,
  provider-ID, and MappingResult-ID batch-search paths are now covered.
- If a generic ball tree is promoted later, treat it as a source-metric search
  index that requires `metric_law::metric` and reports `"ball_tree_index"`.
  Coordinate spaces may use that same generic strategy because they are metric
  spaces. A coordinate-only backend should be a separate strategy/name such as
  `coordinate_ball_tree` / `"coordinate_ball_tree_index"` gated by
  `CoordinateSpaceLike`.

## Goal

METRIC must behave like a pipeline framework, not like a loose collection of
unrelated algorithms. The source object is always a finite metric space:

```text
records + source metric = source finite metric space
```

All later objects are explicit pipeline products:

```text
source space
  -> storage/index representation
  -> derived or mapped space
  -> downstream operator
```

The implementation goal is to make every public operator declare which pipeline
stage it consumes and to prevent hidden conversions. A user may search directly
with the original metric, or may explicitly map/embed the space into a Euclidean
coordinate representative and then run coordinate-space operators. Both paths
must preserve lineage and result provenance.

## Core Invariants

1. Source metric is canonical.
   The original `MetricSpace<Record, Metric>` owns the domain records, the
   authoritative metric, `RecordId` identity, and version.

2. Representations do not replace the space.
   Distance tables, live distances, search trees, graphs, and caches are
   representations of a particular space version. They must report exactness,
   representation kind, and stale status.

3. Mapping is explicit.
   Any algorithm that requires coordinates or a Euclidean-style metric must be
   reached through `map`, `embed`, or a `MappingResult`. No public API should
   silently turn source records plus a source metric into coordinates.

4. Operator domains are explicit.
   Each public operator belongs to exactly one or more declared domains:
   `source_space`, `pairwise_distance_provider`, `coordinate_space`,
   `mapping_result`, or `record_metric_convenience`.

5. Record lineage survives every pipeline step.
   Derived spaces, representative spaces, embeddings, and compressed spaces must
   keep source-to-target and target-to-source `RecordId` lineage where possible.

6. Search supports two deliberate routes.
   Direct route: search the source space using the source metric and a compatible
   metric index. Representative route: explicitly embed/map to a coordinate
   space, then search that derived space. Results must state which route was
   used.

7. Entropy is coordinate-space only.
   The current kpN entropy estimator uses metric neighborhoods plus local
   Gaussian coordinate volumes. The public path is therefore:

   ```text
   source space -> map/embed -> coordinate metric space -> entropy
   ```

   Direct `entropy(records, source_metric)` must stay unavailable.

## Operator Domain Matrix

| Component | Operator family | Source space | Pairwise provider | Records + metric convenience | Mapping result | Coordinate space required |
| --- | --- | --- | --- | --- | --- | --- |
| `stats::search` | `find_neighbors`, `range`, batch search | yes | selected ID queries | yes, for source-space search | yes, when mapped space is searched | optional |
| `stats::properties` | `describe_structure`, `profile`, `distance_distribution`, `local_volume`, `density` | yes | yes | yes | no special path needed | no |
| `stats::properties` | `entropy` | only if already coordinate-like | no | no | yes | yes |
| `stats::properties` | `intrinsic_dimension` | yes | yes | yes | no special path needed | no |
| `stats::sample` | `regular_sample`, `farthest_first`, `metric_walk` | yes | yes | yes | no special path needed | no |
| `stats::structural_analysis` | groups, outliers, representatives | yes | yes | yes | optional derived-space route | no |
| `stats::correlate` | MGC, significance | yes | yes | yes | optional if comparing derived spaces | no |
| `modify::map` | deterministic and derived maps | source required | optional internals | no hidden conversion | produces mapping result | target may be coordinate |
| `modify::reduce/represent/compress` | derived finite spaces | yes | optional internals | convenience only if lineage is preserved | result may feed later stages | no |

## Parallel Workstreams

The work is designed for parallel threads or subagents. Each lane should stay
inside its file ownership unless an integration note explicitly says otherwise.

### Lane 0: Contract Coordinator

Purpose: own vocabulary and merge order.

Files:

- `docs/engine/pipeline-interoperability-plan.md`
- `docs/engine/mental-model.md`
- `docs/engine/operators.md`
- `docs/engine/mappings.md`
- `docs/engine/index.md`

Tasks:

1. Keep the operator domain matrix current as other lanes discover mismatches.
2. Decide final public terms for `source_space`, `coordinate_space`,
   `mapping_result`, and `record_metric_convenience`.
3. Review every lane for hidden source-metric-to-coordinate conversions.
4. Maintain a running list of intentionally blocked API combinations.

Acceptance:

- The docs describe the pipeline before naming algorithms.
- Every coordinate-only operator has an explicit mapping/embedding precondition.
- The plan links from `docs/engine/index.md`.

### Lane 1: C++ Concept And Type Contract

Purpose: make the type system reflect the pipeline.

Primary files:

- `metric/core/concepts.hpp`
- `metric/core/result.hpp`
- `metric/core/metric_space.hpp`
- optional new file: `metric/core/operator_domain.hpp`

Tasks:

1. Add or refine concepts for:
   - `RecordMetricSpaceLike`
   - `PairwiseDistanceProviderLike`
   - `CoordinateRecordLike`
   - `CoordinateSpaceLike`
   - `MappingResultLike`
2. Keep `MetricSpaceLike` broad only if existing code needs it; otherwise split
   narrow concepts so algorithms fail with useful static assertions.
3. Add domain metadata helpers for result provenance if current string fields
   are not enough.
4. Add static assertion smoke tests proving that opaque domain records work for
   distance-only operators but are rejected by coordinate-only operators.

Acceptance:

- Coordinate-only code does not instantiate on opaque source records.
- Distance-only code accepts arbitrary records plus compatible metric callables.
- Errors mention the missing pipeline stage rather than template internals.

Suggested tests:

```sh
cmake --build build/core --target metric_contracts_smoke metric_pipeline_contracts_smoke stats_records_metric_interop_smoke -j2
ctest --test-dir build/core -R "metric_(contracts|pipeline_contracts)_smoke|stats_records_metric_interop_smoke" --output-on-failure
```

### Lane 2: Source-Space And Provider Operators

Purpose: normalize operators that should work directly on the source metric.

Primary files:

- `metric/stats/properties/describe.hpp`
- `metric/stats/properties/distribution.hpp`
- `metric/stats/properties/local_volume.hpp`
- `metric/stats/properties/profile.hpp`
- `metric/stats/properties/intrinsic_dimension.hpp`
- `metric/stats/sample/sample.hpp`
- `metric/stats/structural_analysis/groups.hpp`
- `metric/stats/structural_analysis/outliers.hpp`
- `metric/space/select/representatives.hpp`

Tasks:

1. Ensure each distance-only operator supports the intended triad:
   - `Space`
   - `PairwiseDistances`
   - `records + metric` convenience when lineage can be generated safely
2. Ensure all result objects report representation:
   - `"records"` for convenience-created source spaces
   - `"metric_space"` for direct spaces
   - `"pairwise_distances"` or the concrete representation for providers
3. Remove any operator-specific duplicate wrappers that conflict with the
   unified domain object.
4. Add tests with non-vector source records and custom metrics.

Acceptance:

- Opaque source records work for distance-only diagnostics, sampling, groups,
  outliers, and representatives.
- The implementation does not embed or coordinate-transform records implicitly.

Suggested tests:

```sh
cmake --build build/core --target stats_user_api_properties_smoke stats_user_api_structural_smoke stats_records_metric_interop_smoke -j2
ctest --test-dir build/core -R "stats_user_api_(properties|structural)_smoke|stats_records_metric_interop_smoke" --output-on-failure
```

### Lane 3: Search Pipeline

Purpose: make direct metric search and representative embedded search explicit.

Primary files:

- `metric/stats/search/neighbors.hpp`
- `metric/stats/search/nearest.hpp`
- `metric/stats/search/options.hpp`
- `metric/space/storage/strategy.hpp`
- `metric/space/storage/*index*.hpp`
- docs under `docs/engine/strategies.md` and `docs/engine/representations.md`

Tasks:

1. Classify every search strategy by domain:
   - source metric strategy
   - metric-law-required source strategy
   - coordinate-space strategy
   - mapped-space representative strategy
2. Make unsupported strategy/space combinations fail explicitly.
   Example: a coordinate-only index must require a coordinate/mapped space, not
   silently consume opaque source records.
3. Define how `MappingResult` search works:
   - in-sample `RecordId` query maps through lineage
   - raw source-record query is rejected; out-of-sample search requires a
     separate derived-artifact API that can actually transform the query
4. Preserve result provenance:
   - source metric route
   - derived coordinate route
   - mapping name and strategy
   - exactness and representative/approximation status
5. Keep performance work out of this lane except where a strategy must expose a
   domain precondition.

Acceptance:

- A user can intentionally choose source-space search or mapped-space search.
- A result never hides whether neighbors came from the original metric or a
  Euclidean representative space.
- Invalid combinations produce domain errors, not template explosions.

Suggested tests:

```sh
cmake --build build/core --target engine_nearest_operators_smoke stats_user_api_search_sample_smoke space_search_parity_smoke -j2
ctest --test-dir build/core -R "engine_nearest_operators_smoke|stats_user_api_search_sample_smoke|space_search_parity_smoke" --output-on-failure
```

### Lane 4: Coordinate Operators And Entropy

Purpose: enforce the coordinate-space precondition for entropy and similar
operators.

Primary files:

- `metric/stats/properties/entropy.hpp`
- `metric/correlation/entropy.hpp`
- `tests/core_smoke/*entropy*`
- docs under `docs/engine/operators.md`

Tasks:

1. Keep the public `entropy` surface limited to:
   - coordinate-like `Space`
   - `MappingResult` whose target space is coordinate-like
2. Keep direct estimator tests on `mtrc::Entropy` separate from framework API
   tests on `stats::properties::entropy`.
3. Add compile-time guardrails for coordinate-like records:
   - `size()`
   - indexed coordinate access
   - finite numeric coordinate values where feasible
4. Keep coordinate-metric admission explicit through `CoordinateMetricLike`:
   - do not require exact `Euclidean`
   - do require an admitted coordinate-neighborhood metric family
   - do not treat every `metric_law::metric` over numeric/indexable records as
     entropy-safe coordinate geometry
5. Make failure messages explain the missing map/embed stage.

Acceptance:

- `entropy(records, source_metric)` is unavailable.
- `entropy(mapping_result)` works and reports the mapping representation.
- `entropy(space)` works only for coordinate-like spaces.
- Tests demonstrate source space -> map/embed -> entropy.

Suggested tests:

```sh
cmake --build build/core --target stats_entropy_status_smoke metric_entropy_properties_smoke engine_statistics_operators_smoke -j2
ctest --test-dir build/core -R "stats_entropy_status_smoke|metric_entropy_properties_smoke|engine_statistics_operators_smoke" --output-on-failure
```

### Lane 5: Mapping And Embedding Results

Purpose: make derived spaces first-class pipeline objects.

Primary files:

- `metric/modify/map/map.hpp`
- `metric/modify/map/embed.hpp`
- `metric/modify/map/pcfa.hpp`
- `metric/modify/map/parametric_diffusion_coordinates.hpp`
- `metric/modify/compose/*`
- `metric/core/result.hpp`

Tasks:

1. Audit every mapping result for:
   - target metric law
   - source lineage
   - target-to-source lineage
   - out-of-sample support
   - inverse support
   - validity text
   - the global contract policy: reject malformed local lineage only; do not
     reject cross-row source/representative reuse or sparse `RecordId` values.
2. Standardize names for mapping and strategy strings.
3. Ensure deterministic transforms, PCFA, MDS, parametric diffusion coordinate, and native
   coordinate-solver outputs all produce result objects that downstream operators can
   consume uniformly.
4. Add preservation diagnostics where already available:
   - neighbor recall
   - distance-profile correlation
   - stress
   - finite-coordinate diagnostics
5. Avoid adding new solvers in this lane; this lane only standardizes pipeline
   output contracts.

Acceptance:

- Any promoted mapping result can feed search or coordinate-space operators
  when its target space satisfies the operator domain.
- Missing inverse/out-of-sample support is explicit and test-covered.

Suggested tests:

```sh
cmake --build build/core --target engine_map_intent_smoke engine_pcfa_mapping_smoke metric_space_mapping_pipeline_smoke parametric_diffusion_coordinate_artifact_smoke -j2
ctest --test-dir build/core -R "engine_map_intent_smoke|engine_pcfa_mapping_smoke|metric_space_mapping_pipeline_smoke|parametric_diffusion_coordinate_artifact_smoke" --output-on-failure
```

### Lane 6: Correlation And Cross-Space Alignment

Purpose: make paired-space operators follow the same input-shape rules.

Primary files:

- `metric/stats/correlate/correlation.hpp`
- `metric/stats/correlate/compare.hpp`
- `metric/stats/correlate/significance.hpp`
- tests under `tests/core_smoke/*correlate*`

Tasks:

1. Normalize public entry points across:
   - `records + metric`
   - `Space`
   - `PairwiseDistances`
2. Keep alignment rules explicit:
   - positional alignment
   - `RecordId` alignment
   - mapping-derived alignment
3. Ensure result provenance carries both left and right domains and
   representations.
4. Add tests where one side is a source space and the other is a mapped space.

Acceptance:

- MGC and significance tests state how records are aligned.
- Compare wrappers do not lag behind the underlying MGC overloads.

Suggested tests:

```sh
cmake --build build/core --target stats_user_api_correlate_smoke stats_correlate_dependence_fixtures_smoke engine_compare_intent_smoke -j2
ctest --test-dir build/core -R "stats_user_api_correlate_smoke|stats_correlate_dependence_fixtures_smoke|engine_compare_intent_smoke" --output-on-failure
```

### Lane 7: Python Surface

Purpose: mirror the C++ pipeline contract in Python.

Primary files:

- `python/pkg/metric/spaces.py`
- `python/pkg/metric/operators.py`
- `python/pkg/metric/results.py`
- `python/src/*`
- `docs/api/python.md`

Tasks:

1. Ensure Python `Space` keeps source metric semantics visible.
2. Expose pipeline-friendly forms:
   - `space.neighbors(...)` for source-space search
   - `space.map(...).space` or mapping result for coordinate operators
   - no direct `entropy(records, metric)` shortcut if C++ does not have one
3. Update error messages to say which pipeline stage is missing.
4. Align docs with promoted/native binding availability.
5. Add tests for unsupported strategies that currently exist only as reserved
   vocabulary.

Acceptance:

- Python docs and runtime errors match the C++ domain matrix.
- A Python user can tell whether an operation ran on original metric space or
  mapped coordinate space.

Suggested tests:

```sh
pytest python/tests -q
```

### Lane 8: Documentation And Examples

Purpose: teach the pipeline shape through examples.

Primary files:

- `docs/engine/*.md`
- `docs/api/cpp.md`
- `docs/api/python.md`
- `examples/engine/*`
- `examples/core/*`

Tasks:

1. Add a canonical pipeline example:

   ```text
   source records + source metric
   -> source-space search
   -> explicit embedding
   -> mapped-space search
   -> entropy on mapped coordinates
   ```

2. Show the difference between:
   - source metric neighbors
   - Euclidean representative neighbors after embedding
3. Make the entropy example start from a mapping result.
4. Remove docs that describe coordinate-only algorithms as generic source-space
   properties.
5. Link this plan from the engine docs index while it is active.

Acceptance:

- A reader can predict which API accepts source records, providers, spaces, or
  mapping results before reading templates.
- Entropy examples never skip the embedding stage.

## Integration Protocol

1. Lane 0 lands terminology first.
2. Lanes 1, 2, 3, 4, 5, 6, 7, and 8 can run in parallel after Lane 0 terms are
   stable.
3. Any lane adding or removing an overload must update the operator domain
   matrix in this file.
4. Every lane must add or update tests that use at least one non-vector source
   record type unless the lane is explicitly coordinate-only.
5. Do not mix performance changes into interoperability work. Index internals
   can be touched only to expose domain preconditions or provenance.
6. Preserve user changes in the worktree. Do not revert unrelated files.

## Merge Gates

Gate A: Domain compile gate

```sh
cmake --build build/core --target metric_contracts_smoke metric_pipeline_contracts_smoke stats_records_metric_interop_smoke -j2
ctest --test-dir build/core -R "metric_contracts_smoke|metric_pipeline_contracts_smoke|stats_records_metric_interop_smoke" --output-on-failure
```

Gate B: Stats and search gate

```sh
cmake --build build/core --target stats_user_api_properties_smoke stats_user_api_search_sample_smoke stats_user_api_structural_smoke engine_nearest_operators_smoke search_pipeline_interoperability_smoke -j2
ctest --test-dir build/core -R "stats_user_api_(properties|search_sample|structural)_smoke|engine_nearest_operators_smoke|search_pipeline_interoperability_smoke" --output-on-failure
```

Gate C: Mapping and coordinate gate

```sh
cmake --build build/core --target engine_map_intent_smoke engine_density_filter_intent_smoke metric_space_mapping_pipeline_smoke stats_entropy_status_smoke engine_statistics_operators_smoke -j2
ctest --test-dir build/core -R "engine_map_intent_smoke|engine_density_filter_intent_smoke|metric_space_mapping_pipeline_smoke|stats_entropy_status_smoke|engine_statistics_operators_smoke" --output-on-failure
```

Gate D: Cross-space gate

```sh
cmake --build build/core --target stats_user_api_correlate_smoke engine_compare_intent_smoke engine_cross_space_dependency_smoke -j2
ctest --test-dir build/core -R "stats_user_api_correlate_smoke|engine_compare_intent_smoke|engine_cross_space_dependency_smoke" --output-on-failure
```

## Subagent Task Packets

Use these packets directly when dispatching work.

### Subagent A: Contract Audit

Task: inspect public C++ operator headers and classify each overload by the
domain matrix. Add missing concept/static-assert notes, but do not refactor
algorithms.

Deliverable:

- Patch to concepts or comments if needed.
- Updated matrix rows in this plan.
- One smoke test with opaque records.

### Subagent B: Search Pipeline

Task: make source-space search and mapped-space search explicit. Do not change
performance behavior. Add provenance fields or result strings only where needed.

Deliverable:

- Search API/domain cleanup.
- Tests proving source metric search and mapped coordinate search are
  distinguishable.

### Subagent C: Entropy And Coordinate Operators

Task: enforce entropy as post-embedding coordinate operator.

Deliverable:

- No public `entropy(records, source_metric)`.
- `entropy(MappingResult)` path.
- Tests showing source -> map -> entropy.

### Subagent D: Mapping Result Uniformity

Task: audit promoted mapping outputs and make them uniformly consumable by
downstream operators.

Deliverable:

- Mapping result contract fixes.
- Tests for lineage, target metric law, validity, and out-of-sample flags.

### Subagent E: Python Parity

Task: make Python surface match the C++ pipeline contract and error semantics.

Deliverable:

- Python API/docs patch.
- Tests for blocked direct coordinate-only shortcuts.

### Subagent F: Documentation Examples

Task: write one canonical pipeline example and remove misleading algorithm-list
language.

Deliverable:

- Updated C++ docs and examples.
- Entropy examples start from mapped coordinates.

## Open Decisions

No open interoperability policy decisions remain in this plan. Future promoted
operators or index backends should add new decisions here before implementation
when their pipeline stage or result provenance is ambiguous.

## Done Criteria

The interoperability work is complete when:

1. Every public operator documents its accepted pipeline stage.
2. Distance-only algorithms accept source spaces and providers consistently.
3. Coordinate-only algorithms require mapped/embedded coordinate spaces.
4. Search can run on original metric spaces or explicitly mapped representative
   spaces with distinct provenance.
5. Mapping results carry enough lineage and validity metadata to feed downstream
   operators safely.
6. C++ and Python user-facing APIs agree on blocked and supported paths.
7. Docs teach the pipeline shape before naming individual algorithms.
