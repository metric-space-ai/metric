# Agent Task: ProcessCurveSceneView Consolidation

## Owner Scope

This task owns only the process-curve semantic-view slice:

- `visual/src/views/ProcessCurveSceneView.js`
- `visual/src/views/index.js` if an export is needed
- `visual/src/metric-visual.js` only inside `showProcessCurves()` imports and
  call wiring
- `visual/tools/check-process-curve-scene-view.mjs`
- `visual/tools/check-single-render-pipeline.mjs`
- `docs/visual/reports/process-curve-scene-view-consolidation.md`

Do not edit GRAE10, public project pages, native C++ exporters, relation
matrix/glyph/mapping/dynamics views, runtime internals, or generated outputs.
Other workers may be editing different visual-engine files. Do not revert or
format unrelated changes.

## Objective

Make `ProcessCurveSceneView` a real semantic engine view. The public command
`MetricVisualSurface.showProcessCurves()` must configure this view and call
`view.toLayerDescriptors()`. It must not call low-level process-curve descriptor
helpers directly and must not leave `this.views = []`.

This is not a hero-page task. The result is an engine grammar that can render
process-curve evidence through the same pipeline as every other view:

```text
metric.visual.v1 evidence -> ProcessCurveSceneView -> layer descriptors -> MetricVisualRuntime
```

## Required Code Shape

1. Add or complete an exported `ProcessCurveSceneView` class extending
   `BaseView`.
2. Keep the existing helper functions only as private implementation helpers
   behind the view. Their public use from `metric-visual.js` must stop.
3. `ProcessCurveSceneView.fromVisualSpace(document, options)` must resolve the
   dataset, source coordinate, target coordinate and label property.
4. `ProcessCurveSceneView.toLayerDescriptors()` must emit the current
   process-curve grammar descriptors:
   - scalar/semantic field
   - ground projection
   - record-order trajectory/track
   - skyline/record-volume glyphs
   - 2D/3D morph point layer
   - optional neighborhood graph support
   - optional relation matrix support
5. The view metadata must mark:
   - `viewClass: "ProcessCurveSceneView"`
   - `visualGrammar: "process-curves"`
   - `algorithmicComputation: false`
   - resolved dataset, coordinate, property and relation IDs
6. `showProcessCurves()` must set `this.views = [view]` and call either
   `this.setViews([view], ...)` or `this.setLayerDescriptors(view.toLayerDescriptors(), ...)`.
7. Keep all algorithmic values from exported evidence. Do not compute process
   curves, metrics, PHATE, AE, embeddings, entropy or distances in JavaScript.

## Forbidden

- Do not touch `visual/examples/grae10-metric-engine/`.
- Do not create a page-local renderer or HTML helper.
- Do not add npm, CDN, build tooling or external dependencies.
- Do not generate synthetic application evidence.
- Do not silently catch missing required process-curve inputs.
- Do not promote any preview to hero accepted.
- Do not remove the existing look/stage helpers unless the replacement is
  still inside `ProcessCurveSceneView`.

## Acceptance

Add `visual/tools/check-process-curve-scene-view.mjs` that proves:

- `ProcessCurveSceneView.fromVisualSpace()` produces one view with kind
  `process-curves`.
- `toLayerDescriptors()` emits the required primitive families:
  `HeatFieldLayer`, `GroundProjectionLayer`, `CurveRibbonLayer` or
  `CurveTubeMeshLayer`, `InstancedBoxLayer`, `InstancedPointLayer`.
- Optional graph/matrix support can be enabled and disabled through options.
- `MetricVisualSurface.showProcessCurves()` records
  `surface.views[0].kind === "process-curves"`.
- `metric-visual.js` no longer imports or calls
  `createProcessCurveMiniatureLayerDescriptors`.

Update `visual/tools/check-single-render-pipeline.mjs` so it rejects direct
`createProcessCurveMiniatureLayerDescriptors` use from `metric-visual.js`.

Run and report:

```bash
node --check visual/src/views/ProcessCurveSceneView.js
node --check visual/src/metric-visual.js
node --check visual/tools/check-process-curve-scene-view.mjs
node visual/tools/check-process-curve-scene-view.mjs
node visual/tools/check-views.mjs
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-grae10-golden.mjs
METRIC_VISUAL_EXAMPLES=process-curve-external-hero node visual/tools/check-visual-regression-public-examples.mjs
```

## Report

Write `docs/visual/reports/process-curve-scene-view-consolidation.md` with:

- changed files
- old direct render path removed
- descriptor primitive list emitted by the view
- validation command outputs summarized
- remaining preview-only blockers

