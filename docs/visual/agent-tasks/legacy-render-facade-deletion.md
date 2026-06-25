# Agent Task: Legacy Render Facade Deletion

## Owner Scope

This task owns only:

- `visual/src/metric-visual.js`
- `visual/examples/process-curve-condition-monitoring/index.html`
- optionally one new checker under `visual/tools/`
- `docs/visual/reports/legacy-render-facade-deletion.md`

Do not edit GRAE10, native C++ exporters, generated evidence assets, project
site pages, unrelated view classes, or other example pages. Other workers may
edit different visual-engine files in parallel. Do not revert their changes.

## Objective

Remove the old `metric.evidence.v1` browser facade from the public METRIC
Visual surface. The library must not export a second WebGL scene renderer or
old 2D canvas panel stack next to `MetricVisualRuntime`.

The only valid public path is:

```text
metric.visual.v1 evidence -> semantic command/view -> layer descriptors -> MetricVisualRuntime
```

## Required Code Shape

1. Convert `visual/examples/process-curve-condition-monitoring/index.html`
   away from:
   - `MetricVisualRuntime` direct setup
   - `createLayerFromDescriptor`
   - `createMiniaturePhotographicStyle`
   - `createProcessCurveMiniatureSceneBundle`
   - `MetricHeatmap`
   - `MetricQueryInspector`
   - `MetricRecordGallery`
   - `MetricSelection`
   - `loadMetricEvidence`
   - `../../../docs/examples/assets/process-curve-external/evidence.json`
2. The page must load native `metric.visual.v1` evidence and call the public
   command API, preferably `createMetricVisual(...).showProcessCurves(...)`.
3. Delete the legacy exports from `visual/src/metric-visual.js` after the page
   no longer imports them:
   - `loadMetricEvidence`
   - `assertMetricEvidence`
   - `MetricSelection`
   - `MetricScene3D`
   - `MetricRecordGallery`
   - `MetricHeatmap`
   - `MetricQueryInspector`
   - `MetricProcessCurveApp`
4. Delete helper functions that only served those legacy classes, including raw
   WebGL shader helpers and old 2D canvas helpers.
5. Keep `adaptMetricEvidenceV1()` in `visual/src/data/evidence-adapter.js`.
   That adapter is compatibility data ingestion, not a render facade.
6. Do not replace the deleted facade with another page-local renderer or SVG/2D
   fallback.

## Forbidden

- Do not touch `visual/examples/grae10-metric-engine/`.
- Do not compute metric values, winners, distances, entropy or embeddings in
  JavaScript.
- Do not use old `metric.evidence.v1` as a public example input after this
  change.
- Do not promote `process-curve-condition-monitoring` or any preview to
  accepted hero status.

## Acceptance

Add or update a checker that fails when public code still references the legacy
facade. It must cover at least:

```bash
rg -n "MetricScene3D|MetricRecordGallery|MetricHeatmap|MetricQueryInspector|MetricProcessCurveApp|MetricSelection|loadMetricEvidence|assertMetricEvidence" visual/src/metric-visual.js visual/examples/process-curve-condition-monitoring/index.html
rg -n "metric\\.evidence\\.v1|evidence\\.json" visual/examples/process-curve-condition-monitoring/index.html
```

Run and report:

```bash
node --check visual/src/metric-visual.js
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-runtime-picking-preview.mjs
node visual/tools/check-relation-matrix-picker.mjs
METRIC_VISUAL_EXAMPLES=process-curve-condition-monitoring node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-grae10-golden.mjs
```

If `process-curve-condition-monitoring` is not part of the public regression
set, run a targeted browser smoke or add a narrow checker for that page.

## Report

Write `docs/visual/reports/legacy-render-facade-deletion.md` with:

- deleted legacy symbols
- new process-curve page path through the public command API
- validation commands and results
- any remaining noncanonical render paths intentionally left for another task
