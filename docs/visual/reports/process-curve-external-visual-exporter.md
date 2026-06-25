# Process-Curve External Visual Exporter Report

## Source Files Used

- `examples/engine/assets/process_curve_power_demand_gallery.csv`
- `examples/engine/assets/process_curve_internal_bleeding_gallery.csv`
- Existing process-curve C++ evidence pattern from `examples/engine/process_curve_external_gallery.cpp`

No synthetic fixture data, JavaScript metric computation, page-local Canvas renderer, page-local SVG renderer, or GRAE10 file was used.

## Generated Evidence

- Source records: 48 original 36-sample process-curve snippets
- Query records: 16 downsampled 18-sample process-curve query snippets
- Relations: 1 dense aligned process-curve metric relation
- Relation pair values: 2304
- Coordinates: 48 C++ metric-derived landmark positions
- Graph edges: 192 C++ selected k-nearest edges
- Properties: 11, including query expected role, metric-space winner role/distance/correctness, padded-vector baseline winner role/distance/mismatch, and metric margin

## Native Executable And Output

- C++ executable: `build/core/examples/engine/engine_process_curve_external_visual_export`
- C++ source: `examples/engine/process_curve_external_visual_export.cpp`
- Public output: `docs/examples/assets/process-curve-external/metric.visual.json`
- CMake target/test: `engine_process_curve_external_visual_export`

## Visual Engine View

- Example page: `visual/examples/process-curve-external-hero/index.html`
- Visual Engine API: `createMetricVisual(...)`
- Command/view: `showProcessCurves(...)`
- Declared document view: `process-curve-external-relation-neighborhood-view`
- Relation: `process-curve-external-aligned-metric`
- Coordinate state: `process-curve-external-landmark-3d`
- Graph: `process-curve-external-knn`

## Commands Run

- `cmake --build build --target engine_process_curve_external_visual_export -j 6`
  Result: failed because `build` is not a CMake build directory (`build/core` is the configured core build).
- `cmake --build build/core --target engine_process_curve_external_visual_export -j 6`
  Result: first attempt failed on a local C++ pointer-to-member signature; fixed in exporter source.
- `cmake --build build/core --target engine_process_curve_external_visual_export -j 6`
  Result: passed.
- `build/core/examples/engine/engine_process_curve_external_visual_export --export-json docs/examples/assets/process-curve-external/metric.visual.json`
  Result: passed; wrote 48 source records, 16 query records, 2304 relation pairs.
- `node visual/tools/check-visual-document.mjs docs/examples/assets/process-curve-external/metric.visual.json`
  Result: passed.
- `node visual/tools/check-process-curve-external-visual.mjs`
  Result: passed.
- `node visual/tools/check-public-gallery-evidence.mjs`
  Result: passed.
- `node visual/tools/check-hero-grammar-contract.mjs`
  Result: passed.
- `node visual/tools/check-single-render-pipeline.mjs`
  Result: passed.
- `node visual/tools/check-grae10-golden.mjs`
  Result: passed.
- `ctest --test-dir build/core -R 'example_engine_process_curve_external_visual_(export|validate)' --output-on-failure`
  Result: passed, 2/2 tests.

## Status

Ready for screenshot review as a native Visual Engine preview. It is not marked as a final public hero in this task.

## Parent Integration Review

- Removed local absolute CSV paths from exported provenance; the committed
  public asset now records stable repo-relative source asset identifiers.
- Rebuilt `engine_process_curve_external_visual_export` and regenerated
  `docs/examples/assets/process-curve-external/metric.visual.json`.
- Additional browser smoke:
  `METRIC_VISUAL_EXAMPLES=process-curve-external-hero METRIC_VISUAL_OUT=/tmp/metric-process-curve-external-visual node visual/tools/check-visual-examples.mjs`
  passed with one canvas, no console errors and no page errors.
- Screenshot review status remains preview-only. The current relation
  matrix/neighborhood composition is evidence-bearing, but it is not a polished
  hero screenshot and is not added to the public accepted-hero set.

## Parent Integration Review: Command Grammar

- Added `showProcessCurves()` as a semantic Visual Engine command backed by
  `ProcessCurveSceneView`.
- Updated `visual/examples/process-curve-external-hero/index.html` to call
  `createMetricVisual(...)` plus `visual.showProcessCurves(...)`; the page no
  longer uses the generic relation-matrix command as its primary process-curve
  grammar.
- Updated `ProcessCurveSceneView` to accept native process-curve exports with a
  single 3D coordinate state and `time_series.series` payloads.
- Checks run:
  - `node visual/tools/check-process-curve-external-visual.mjs`: passed.
  - `node visual/tools/check-visual-command-api.mjs`: passed, including
    required grammar primitives for `showProcessCurves`.
  - `METRIC_VISUAL_EXAMPLES=process-curve-external-hero METRIC_VISUAL_OUT=/tmp/metric-process-curve-external-visual-command node visual/tools/check-visual-examples.mjs`: passed.
  - `node visual/tools/check-single-render-pipeline.mjs`: passed.
  - `node visual/tools/check-grae10-golden.mjs`: passed.
  - `node visual/tools/check-hero-grammar-contract.mjs`: passed.
  - `node visual/tools/check-public-gallery-evidence.mjs`: passed.
  - `node visual/tools/check-hero-screenshot-review.mjs`: passed.
- Status remains preview-only. This is now a command-backed process-curve
  grammar candidate, not an accepted public hero.

## Public Regression Integration

- `process-curve-external-hero` is now an explicit native public-preview row in
  `visual/tools/check-visual-regression-public-examples.mjs`; it does not rely
  on the project-page link set to be covered by the browser gate.
- The public browser regression now covers eight pages total: protected GRAE10
  plus seven native preview pages.
- The process-curve page passed the browser gate with
  `selectedViewKind: process-curves`, native `metric.visual.v1` evidence,
  `HeatFieldLayer`, `GroundProjectionLayer`, `CurveTubeMeshLayer`,
  `InstancedBoxLayer`, `InstancedPointLayer`, `RelationEdgeLayer` and
  `RelationMatrixLayer`.
- The gate now verifies the process-curve `CurveTubeMeshLayer` GPU-picking path
  with a layer-isolated curve probe, because the rendered page may legitimately
  contain overlapping heat-field and projection evidence over the same screen
  pixels.
- Screenshot review remains preview-only: GRAE10 is still the only accepted
  hero.
