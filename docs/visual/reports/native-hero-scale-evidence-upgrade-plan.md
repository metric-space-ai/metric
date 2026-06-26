# Native Hero Scale Evidence Upgrade Plan

Date: 2026-06-26
Branch: `codex/visual-engine-evidence-exporters`

This report is kept as the scale/evidence planning audit for native public
previews. It was refreshed after the native scale exporters were enlarged and
the Visual-Brief-aware performance gate was added. No generated `output/`
artifacts are committed by this report.

## Inputs And Gates

Read inputs:

- `docs/visual/visual-engine-masterplan.md`
- `docs/visual/metric-visual-engine-implementation-plan.md`
- `docs/visual/native-exporter-work-queue.md`
- `docs/visual/reports/native-exporter-quality-audit.md`
- `visual/hero-visual-briefs.manifest.json`
- `visual/tools/check-native-hero-evidence-scale.mjs`

Commands run:

```bash
node visual/tools/check-native-hero-evidence-scale.mjs
node visual/tools/check-public-gallery-evidence.mjs
```

Current gate results:

- `check-native-hero-evidence-scale`: `ok: true`, seven rows inspected, seven
  native-scale-ready rows. There are no current public-preview
  record-count blockers.
- `check-public-gallery-evidence`: `ok: true`, GRAE10 hash
  `464f6a90c36c1e9c6b4ec90068500dc226740d65b251918aca567f99d64d3d5e`,
  no public synthetic evidence, no synthetic example marked done.
- `check-visual-performance-large-scenes`: `ok: true`, seven Visual Brief rows
  matched, including the native-scale-ready `process-curve-external-hero`
  preview.

## Top Blockers

| Blocker | Concrete implication |
| --- | --- |
| Native previews are scale-ready but not screenshot-accepted | `condition-monitoring-hero`, `mixed-record-hero`, `cross-space-dependency-hero`, `dynamics-noise-hero`, `mapping-dimensionality-hero`, `relation-matrix-neighborhood` and `process-curve-external-hero` now meet their current native scale thresholds, but remain preview-only until screenshot review accepts their visual grammar and composition. |
| `relation-matrix-neighborhood` is scale-ready but not visually accepted | Its remaining gate blockers are human screenshot/readability blockers, not count blockers. The reusable matrix grammar now exposes stronger focus defaults and explicit shader-capacity diagnostics, but human review still has to accept the final screenshot. |
| `process-curve-external-hero` is scale-ready but not visually accepted | The checked-in native evidence now contains 576 real source windows, 161 query records, a 576-by-576 dense source relation, query/winner evidence and graph evidence. Its remaining blocker is visual composition acceptance, not source scale. |

## Preview Tickets

### `condition-monitoring-hero`

| Field | Plan |
| --- | --- |
| Current counts | `recordCount: 528`, `relationCount: 2`, `recordTypeCount: 1` |
| Native/synthetic status | Native `metric.visual.v1`; `provenance.native_export: true`; `provenance.synthetic` absent/false. Asset: `docs/examples/assets/condition-monitoring/metric.visual.json`. |
| Exact blocker text from gate | `visual-composition-not-human-accepted` |
| Minimum evidence target from brief | At least 500 records and at least 1 relation. Required evidence: windowed process records, native metric relation or neighborhood graph, density or entropy property, anomaly or regime property, original time-series preview payload. |
| C++ exporter file to change | `examples/engine/condition_monitoring_visual_export.cpp` |
| Dataset or deterministic native fixture source | Grow the deterministic process-window source currently in `examples/engine/condition_monitoring_visual_export.cpp::build_observations()`, mirroring the native `examples/engine/condition_monitoring.cpp` condition-monitoring generator. The target fixture must emit at least 500 ordered process windows across healthy baseline, slow drift, abrupt fault, recovery and recurring signature regimes. |
| Native C++ metric computations | Keep TWED process-window distances, full or explicitly declared sparse metric relation values, transition graph edges, nearest healthy/catalog scores, local density or entropy, DBSCAN/outlier flags, anomaly/severity scores, landmark coordinates and metric-law diagnostics in C++. JavaScript may only validate, index, interpolate exported states and render. |
| Required exported properties, relations, coordinates, timelines and previews | Relations: keep/grow `condition-monitoring-twed` and `condition-monitoring-transition` over the full record set. Properties: keep/grow `nearest-healthy-distance`, `condition-severity`, `metric-anomaly-severity`, `local-density` or native entropy, `truth-regime`, `run-phase`, `nearest-catalog-regime`, `diagnosis-state`, DBSCAN/outlier flag. Coordinates: grow `landmark-3d` and `process-state-trajectory-3d` to every record. Timelines: add a native `condition-run` or `condition-evolution` timeline with sampled healthy/drift/fault/recovery state references and property ids. Previews: every record keeps `payload.kind: time_series`, sample rate, truth regime and run phase. |
| Expected visual grammar | Primary `PropertyFieldView` plus `TrajectoryPathView`; supporting `MetricSpaceView`, `GroundProjectionLayer`, `HeatFieldLayer`, `CurveRibbonLayer` and time-series record preview. |
| Acceptance commands | `cmake --build build/core --target engine_condition_monitoring_visual_export -- -j4`; `ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure`; `node visual/tools/check-visual-document.mjs docs/examples/assets/condition-monitoring/metric.visual.json`; `node visual/tools/check-native-hero-evidence-scale.mjs`; `node visual/tools/check-public-gallery-evidence.mjs`; screenshot review via the public visual regression/review gate before any hero acceptance. |
| Risk if scaled naively | A longer list of nearly identical windows can still render as a thin point path with no early-warning field. Dense all-pairs TWED over 500+ windows is acceptable but must be computed once in C++ and exported intentionally; browser-side scoring or query padding would not prove the condition-monitoring claim. |

### `mixed-record-hero`

| Field | Plan |
| --- | --- |
| Current counts | `recordCount: 2,000`, `relationCount: 1`, `recordTypeCount: 4` |
| Native/synthetic status | Native `metric.visual.v1`; `provenance.native_export: true`; `provenance.synthetic` absent/false. Asset: `docs/examples/assets/mixed-records/metric.visual.json`. |
| Exact blocker text from gate | `visual-composition-not-human-accepted` |
| Minimum evidence target from brief | At least 2,000 records, at least 1 relation and at least 4 record types. Required evidence: typed records, type-specific preview payloads, native metric relation, cross-type nearest-neighbor edges, family or type labels. |
| C++ exporter file to change | `examples/engine/mixed_finite_records_visual_export.cpp` |
| Dataset or deterministic native fixture source | Use the deterministic native source in `examples/engine/mixed_finite_records_fixture.hpp` and metric contract in `examples/engine/mixed_finite_records.hpp`. The implementation ticket is to expand the fixture to at least 2,000 records across four honest public record types: text code records, histogram/spectrum records, process-curve records and numeric-vitals records. Each public record type must carry a type-specific payload; do not satisfy the gate by only relabeling the current `mixed_structured_record` composed payload. |
| Native C++ metric computations | Keep Edit distance for text, Wasserstein/EMD for spectra, TWED for process curves, standardized Euclidean for vitals, the positive-weight composite metric, per-field contribution vectors, nearest-neighbor search, cross-type edge selection, cluster/representative assignment and metric-law diagnostics in C++. No JavaScript or Python computation may create distances, contributions, clusters or embeddings. |
| Required exported properties, relations, coordinates, timelines and previews | Relations: grow `mixed-finite-records-composite-metric` to the 2,000-record domain and keep/grow the kNN graph. Add pair or edge metadata for dominant contribution channel where cross-type edges are shown. Properties: keep/grow `family`, `severity`, `type`, `channel`, `nearest_distance`, `nearest_contributions`, `cluster`, `representative`; add a record-type property matching the four exported `record_type` values. Coordinates: grow `mixed-finite-records-mds-3d` and `mixed-finite-records-family-severity-3d` or their native equivalents. Timelines: none required for this hero. Previews: text records expose source string, spectrum records expose histogram/distribution, process-curve records expose time series, numeric-vitals records expose vector/table values. |
| Expected visual grammar | Primary `MixedRecordView` typed glyph scene plus `RelationEdgeLayer` cross-type nearest edges and typed preview; relation matrix is supporting context. |
| Acceptance commands | `cmake --build build/core --target engine_mixed_finite_records_visual_export -- -j4`; `ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure`; `node visual/tools/check-visual-document.mjs docs/examples/assets/mixed-records/metric.visual.json`; `node visual/tools/check-native-hero-evidence-scale.mjs`; `node visual/tools/check-public-gallery-evidence.mjs`; screenshot review through the public regression/review gate. |
| Risk if scaled naively | The previous blocker was multiplying composed records without creating honest public record types. The current exporter reports 2,000 records across four record types; future changes must preserve type-specific payloads and keep cross-type graph evidence sparse and inspectable. |

### `cross-space-dependency-hero`

| Field | Plan |
| --- | --- |
| Current counts | `recordCount: 512`, `relationCount: 3`, `recordTypeCount: 1` |
| Native/synthetic status | Native `metric.visual.v1`; `provenance.native_export: true`; `provenance.synthetic` absent/false. Asset: `docs/examples/assets/cross-space-dependency/metric.visual.json`. |
| Exact blocker text from gate | `visual-composition-not-human-accepted` |
| Minimum evidence target from brief | At least 500 records and at least 2 relations. Required evidence: two native metric spaces, paired records, cross-space dependence diagnostics, linked selection metadata, bridge or edge evidence. |
| C++ exporter file to change | `examples/engine/cross_space_dependency_visual_export.cpp` |
| Dataset or deterministic native fixture source | Use `examples/engine/cross_space_dependency.hpp::generate_dataset(count, Coupling::coupled, seed)` as the deterministic native paired-observation source and raise the export count to at least 500 paired observations. Keep the source semantics: event logs on the left, process curves on the right, one shared latent regime only for diagnostics. |
| Native C++ metric computations | Keep event-log Edit distance, process-curve TWED distance, native distance matrices, MGC statistic through `mtrc::compare`/`MGC_direct`, permutation-test significance, local dependence contribution, local density and distance-profile alignment in C++. JavaScript must not recompute MGC, correlations, distance matrices or local contributions. |
| Required exported properties, relations, coordinates, timelines and previews | Relations: grow `event-log-edit-distance` and `process-curve-twed-distance` to 500 paired observations. Add explicit bridge/edge evidence for high-contribution paired observations or paired-neighborhood references if the paired-space view needs visible bridge geometry. Properties: keep/grow `generator-latent-severity`, `local-distance-profile-alignment`, `local-dependence-contribution`, `event-log-local-density`, `process-curve-local-density`, `pair-distance-profile-contribution`; keep global MGC statistic and p-value in diagnostics. Coordinates: grow `event-log-landmark-3d` and `process-curve-landmark-3d`. Timelines: none required. Previews: each paired record keeps composed event-log and process-curve payloads plus local contribution diagnostics. |
| Expected visual grammar | Primary paired-space scene plus dependence bridges and linked brushing; supporting relation matrices for the two native metric spaces. |
| Acceptance commands | `cmake --build build/core --target engine_cross_space_dependency_visual_export -- -j4`; `ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure`; `node visual/tools/check-visual-document.mjs docs/examples/assets/cross-space-dependency/metric.visual.json`; `node visual/tools/check-native-hero-evidence-scale.mjs`; `node visual/tools/check-public-gallery-evidence.mjs`; browser screenshot review for paired-space composition and linked selection. |
| Risk if scaled naively | Raising `count` without controlling permutation cost can make export time explode. Reducing the evidence to one scalar MGC statistic would not prove local dependence visually; the exporter must preserve local contribution and bridge evidence. |

### `relation-matrix-neighborhood`

| Field | Plan |
| --- | --- |
| Current counts | `recordCount: 130`, `relationCount: 1`, `recordTypeCount: 1` |
| Native/synthetic status | Native `metric.visual.v1`; `provenance.native_export: true`; `provenance.synthetic` absent/false. Asset: `docs/examples/assets/relation-matrix/metric.visual.json`. Native scale is ready, but hero acceptance is not. |
| Exact blocker text from gate | `visual-composition-not-human-accepted`; `matrix-readability-not-human-accepted` |
| Minimum evidence target from brief | At least 128 records and at least 1 relation. Required evidence: native dense relation, ordered matrix rows and columns, native neighborhood graph, pair preview evidence, block or tile readability metadata. |
| C++ exporter file to change | `examples/engine/relation_matrix_visual_export.cpp` |
| Dataset or deterministic native fixture source | Keep the deterministic process-curve fixture expansion in `examples/engine/relation_matrix_visual_export.cpp` (`family_templates`, `variant_curve`, `make_records`) sourced from the `examples/engine/process_curves_space.cpp` shape. Count growth is not required for the scale gate. |
| Native C++ metric computations | Keep aligned process-curve metric values, complete symmetric dense matrix, kNN graph, block ordering, curve summary properties and metric-law diagnostics in C++. JavaScript may build matrix textures/LOD from exported values but must not compute metric values or neighborhoods. |
| Required exported properties, relations, coordinates, timelines and previews | Relations: keep `process-curve-aligned-metric` as a native symmetric dense matrix over all 130 records. Graphs: keep `process-curve-knn` with native k-nearest edges. Properties: keep `process-family`, `matrix-block-order`, `matrix-order-index`, `within-block-order`, `curve-mean`, `curve-peak`, `curve-total-variation`. Coordinates: keep `process-curve-block-layout-3d`. Timelines: none required. Previews: ensure record previews expose original time-series snippets and pair previews expose pair distance, row/column ids, block membership and nearest-neighbor rank where available. Add explicit pair-property evidence if the current pair preview cannot explain selected cells without page-local inference. |
| Expected visual grammar | Primary relation matrix plus sparse neighborhood graph and pair preview; point cloud is optional context. |
| Acceptance commands | `cmake --build build/core --target engine_relation_matrix_visual_export -- -j4`; `ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure`; `node visual/tools/check-visual-document.mjs docs/examples/assets/relation-matrix/metric.visual.json`; `node visual/tools/check-native-hero-evidence-scale.mjs`; `node visual/tools/check-public-gallery-evidence.mjs`; screenshot review must clear both matrix readability and neighborhood composition. |
| Risk if scaled naively | Increasing record count is not the blocker and may worsen aliasing. The risk is producing a dense matrix that passes schema and scale gates but remains unreadable because block/tile metadata, row/column focus and pair preview evidence are insufficient. |

### `dynamics-noise-hero`

| Field | Plan |
| --- | --- |
| Current counts | `recordCount: 512`, `relationCount: 2`, `recordTypeCount: 1` |
| Native/synthetic status | Native `metric.visual.v1`; `provenance.native_export: true`; `provenance.synthetic` absent/false. Asset: `docs/examples/assets/dynamics-noise/metric.visual.json`. |
| Exact blocker text from gate | `visual-composition-not-human-accepted` |
| Minimum evidence target from brief | At least 500 records and at least 1 relation. Required evidence: timeline states, native transition or dynamics relation, propagation field, trajectory records, state-history preview data. |
| C++ exporter file to change | `examples/engine/finite_metric_dynamics_visual_export.cpp` |
| Dataset or deterministic native fixture source | Grow `examples/engine/finite_metric_dynamics_fixture.hpp` from the current 28-node S-curve to at least 500 deterministic nodes. The fixture should remain closed-form and reproducible, for example a higher-resolution S-curve or multi-segment low-dimensional manifold with deterministic noise states. |
| Native C++ metric computations | Keep Euclidean fixture metric, geometry-derived transition matrix, transition graph, forward diffusion, reverse reconstruction, random-walk visit counts, stationary distribution, Dirichlet energy, MSE-to-clean, best reconstruction error and timeline coordinates in C++. JavaScript may scrub/interpolate exported coordinate states only. |
| Required exported properties, relations, coordinates, timelines and previews | Relations: grow `finite-euclidean-metric` and `geometry-derived-transition` to all records. Graphs: grow `transition-graph`. Coordinates: grow all forward and reverse coordinate states (`coord-forward-*`, `coord-reverse-*`, noisy/random-walk/state coordinates if present) to at least 500 positions per state. Timelines: keep/grow `forward-diffusion` and `reverse-reconstruction`. Properties: keep/grow `stationary-distribution`, `transition-degree`, `walk-empirical-distribution`, `walk-visit-count`, `forward-terminal-error`, `best-reconstruction-error`, `terminal-reconstruction-error`, `reconstruction-improvement`, `forward-dirichlet-energy`, `reverse-dirichlet-energy`, `forward-mse-to-clean`, `reverse-mse-to-clean`. Previews: records must expose vector payloads plus state-history values for selected records. |
| Expected visual grammar | Primary `DynamicsView`/`TrajectoryPathView` with propagation field and state-history preview; supporting metric-space and relation-matrix views. |
| Acceptance commands | `cmake --build build/core --target engine_finite_metric_dynamics_visual_export -- -j4`; `ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure`; `node visual/tools/check-visual-document.mjs docs/examples/assets/dynamics-noise/metric.visual.json`; `node visual/tools/check-native-hero-evidence-scale.mjs`; `node visual/tools/check-public-gallery-evidence.mjs`; browser screenshot/interaction review for timeline controls and propagation field. |
| Risk if scaled naively | A 500-node dense metric plus dozens of coordinate states can bloat JSON and GPU buffers if every intermediate state is exported indiscriminately. Downsampling timeline states in C++ is acceptable only if the retained states still prove forward diffusion, reverse flow and state-history inspection. |

### `mapping-dimensionality-hero`

| Field | Plan |
| --- | --- |
| Current counts | `recordCount: 1,000`, `relationCount: 2`, `recordTypeCount: 1` |
| Native/synthetic status | Native `metric.visual.v1`; `provenance.native_export: true`; `provenance.synthetic` absent/false. Asset: `docs/examples/assets/mapping-dimensionality/metric.visual.json`. |
| Exact blocker text from gate | `visual-composition-not-human-accepted` |
| Minimum evidence target from brief | At least 1,000 records and at least 1 relation. Required evidence: source coordinate state, target coordinate state, mapping diagnostics, residual or preservation-error property, record preview payloads. |
| C++ exporter file to change | `examples/engine/mapping_dimensionality_visual_export.cpp` |
| Dataset or deterministic native fixture source | The current embedded `process_records()` fixture is too small. Use a deterministic native process-curve fixture with at least 1,000 records, preferably the family/variant generator pattern already present in `examples/engine/relation_matrix_visual_export.cpp` (`family_templates`, `variant_curve`) or a licensed real process-curve corpus if expanded UCR windows become available. The source must include original time-series snippets and family labels. |
| Native C++ metric computations | Keep aligned process-curve metric values, source-space metric-law diagnostics, diffusion-coordinate target construction, parametric diffusion coordinate mapping, latent Euclidean relation, local mapping distortion, target residuals, kNN graph and query diagnostics in C++. JavaScript must not compute embeddings, residuals, preservation errors or nearest neighbors. |
| Required exported properties, relations, coordinates, timelines and previews | Relations: grow `process-curve-aligned-metric` and `process-curve-latent-euclidean-metric`. Graphs: grow `process-curve-source-knn`. Coordinates: grow `source-coordinate-layout-3d`, `diffusion-coordinate-target-2d` and `parametric-coordinate-latent-2d`. Timelines: add a native coordinate-morph timeline such as source -> diffusion target -> parametric latent so `MappingView` can animate between exported states. Properties: keep/grow `process-family`, `curve-length`, `curve-mean`, `curve-total-variation`, `source-row-mean-distance`, `local-mapping-distortion`, `diffusion-coordinate-target-residual`; add neighborhood preservation or rank-error evidence if residual vectors need pair context. Previews: every process-curve record keeps original time-series payload and mapping diagnostics. |
| Expected visual grammar | Primary `MappingView` coordinate morph plus residual/error vectors or projection; point cloud and relation matrix are supporting context only. |
| Acceptance commands | `cmake --build build/core --target engine_mapping_dimensionality_visual_export -- -j4`; `ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure`; `node visual/tools/check-visual-document.mjs docs/examples/assets/mapping-dimensionality/metric.visual.json`; `node visual/tools/check-native-hero-evidence-scale.mjs`; `node visual/tools/check-public-gallery-evidence.mjs`; browser screenshot review must show morph and residual/error evidence, not only two point clouds. |
| Risk if scaled naively | Scaling the current 15 toy curves by duplicating variants can produce an impressive but uninformative cloud. Exporting only source and latent coordinates without native residual/preservation properties would regress to a generic point-cloud hero and fail the visual brief. |

### `process-curve-external-hero`

| Field | Plan |
| --- | --- |
| Current counts | `recordCount: 737`, `relationCount: 1`, `recordTypeCount: 2`. The current metric relation and coordinate state cover 576 real source records; the other 161 records are native contrast-query previews. |
| Native/synthetic status | Native `metric.visual.v1`; `provenance.native_export: true`; `provenance.synthetic` absent/false. Asset: `docs/examples/assets/process-curve-external/metric.visual.json`. |
| Exact blocker text from gate | `visual-composition-not-human-accepted` |
| Minimum evidence target from brief | At least 500 records and at least 1 relation. Required evidence: real UCR process-curve records, native aligned metric relation, metric-space winner evidence, baseline mismatch evidence, original time-series preview payloads. |
| C++ exporter file to change | `examples/engine/process_curve_external_visual_export.cpp` |
| Dataset or deterministic native fixture source | Integrated. The exporter now uses expanded real UCR-derived CSV assets already named by the exporter: `examples/engine/assets/process_curve_power_demand_gallery.csv` and `examples/engine/assets/process_curve_internal_bleeding_gallery.csv`, sourced from `UCR_Time_Series_Anomaly_Detection_2021`. |
| Native C++ metric computations | Keep aligned curve metric distances, pointwise padded Euclidean baseline distances, nearest-neighbor winner selection, metric margin, correctness/mismatch flags, dense source relation, kNN graph, landmark coordinates and summary diagnostics in C++. JavaScript must not compute winners, margins, baseline mismatches or aligned distances. |
| Required exported properties, relations, coordinates, timelines and previews | Integrated for scale. Relation `process-curve-external-aligned-metric` covers 576 real source windows. Graph `process-curve-external-knn` covers 2,304 native edges. Coordinate state `process-curve-external-landmark-3d` covers 576 source records. Properties are preserved/grown: `process-role`, `process-domain`, `source-curve-mean`, `query-expected-role`, `query-metric-winner-role`, `query-vector-baseline-winner-role`, `query-metric-distance`, `query-vector-baseline-distance`, `query-metric-margin`, `query-metric-correct`, `query-vector-baseline-mismatch`. Previews preserve original source and query time-series snippets, UCR file/source window indices and expected/winner labels. |
| Expected visual grammar | Primary process trajectory plus relation matrix and curve preview; supporting `RelationEdgeLayer`, `RelationMatrixLayer`, `CurveTubeMeshLayer`/curve preview and query-winner emphasis. |
| Acceptance commands | `cmake --build build/core --target engine_process_curve_external_visual_export -- -j4`; `ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure`; `node visual/tools/check-visual-document.mjs docs/examples/assets/process-curve-external/metric.visual.json`; `node visual/tools/check-native-hero-evidence-scale.mjs`; `node visual/tools/check-public-gallery-evidence.mjs` after public gallery wiring includes this preview; browser screenshot review for query/winner/baseline mismatch composition. |
| Risk if scaled naively | Closed for the record-count gate: the gate counts source relation record ids, not total records, and now sees 576 real source windows. Remaining risk is visual composition: the page can still fail if query/winner/baseline evidence is not legible in the process-curve grammar. |

## Queue Update

No queue file update was made. `docs/visual/native-exporter-work-queue.md`
already names the seven disjoint exporter slots covered by this report.
