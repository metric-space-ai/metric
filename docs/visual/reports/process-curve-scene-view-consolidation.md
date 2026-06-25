# ProcessCurveSceneView Consolidation

## Changed files

- `visual/src/views/ProcessCurveSceneView.js`
- `visual/src/views/index.js`
- `visual/src/metric-visual.js`
- `visual/tools/check-process-curve-scene-view.mjs`
- `visual/tools/check-single-render-pipeline.mjs`
- `docs/visual/reports/process-curve-scene-view-consolidation.md`

## Direct render path removed

`MetricVisualSurface.showProcessCurves()` now creates a
`ProcessCurveSceneView`, stores `this.views = [view]`, and installs
`view.toLayerDescriptors()` output. It no longer imports or calls
`createProcessCurveMiniatureLayerDescriptors` from `metric-visual.js`, and it
no longer clears `this.views`.

The view resolves the exported process-curve dataset, source coordinate, target
coordinate, label property, relation, and graph. Its metadata marks:

- `viewClass: "ProcessCurveSceneView"`
- `visualGrammar: "process-curves"`
- `algorithmicComputation: false`
- resolved dataset, coordinate, property, relation, and graph IDs

## Descriptor primitives

With graph and matrix support enabled, `ProcessCurveSceneView` emits:

- `HeatFieldLayer`
- `GroundProjectionLayer`
- `CurveTubeMeshLayer` or `CurveRibbonLayer`
- `InstancedBoxLayer`
- `InstancedPointLayer`
- `RelationEdgeLayer`
- `RelationMatrixLayer`

Graph support is omitted when `includeNeighborhood: false`; matrix support is
omitted when `includeMatrix: false`.

## Validation summary

- `node --check visual/src/views/ProcessCurveSceneView.js`: passed.
- `node --check visual/src/metric-visual.js`: passed.
- `node --check visual/tools/check-process-curve-scene-view.mjs`: passed.
- `node visual/tools/check-process-curve-scene-view.mjs`: passed; reported
  `viewKind: "process-curves"`, `descriptorCount: 8`, and all required
  primitive families.
- `node visual/tools/check-views.mjs`: passed, `total: 80`, `failed: 0`.
- `node visual/tools/check-visual-command-api.mjs`: passed; `showProcessCurves`
  reported `descriptorCount: 8`, `runtimeLayerCount: 8`, `evidenceKind:
  "native"`.
- `node visual/tools/check-single-render-pipeline.mjs`: passed.
- `node visual/tools/check-grae10-golden.mjs`: passed,
  `464f6a90c36c1e9c6b4ec90068500dc226740d65b251918aca567f99d64d3d5e`.
- `METRIC_VISUAL_EXAMPLES=process-curve-external-hero node visual/tools/check-visual-regression-public-examples.mjs`:
  passed, `total: 1`, `failed: 0`.

## Remaining preview-only blockers

- `process-curve-external-hero` remains `public-preview-only`.
- The browser regression explicitly reported `heroAccepted: false`.
- No hero acceptance manifest entry was promoted.
