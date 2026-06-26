# GPU Picking And Preview Workstream

Status date: 2026-06-26

## Outcome

No runtime code changes were needed in this slice. The current engine already
wires hover/click inspection through `MetricVisualRuntime`, uses the shared
`PickingPass` and `PickingRegistry`, builds record and pair previews from
`metric.visual.v1`, and keeps deterministic CPU fallbacks for views without a
usable pick buffer hit.

No hero-page DOM/event preview logic was added. JavaScript preview code renders
or summarizes exported evidence only; it does not compute metric values,
neighbors, mappings, dynamics or derived algorithm output.

## GPU Picking Coverage

Layer primitives with engine-level `renderPicking(context)` support are:

- `InstancedPointLayer`
- `InstancedGlyphLayer`
- `RelationEdgeLayer`
- `HeatFieldLayer`
- `GroundProjectionLayer`
- `CurveRibbonLayer`
- `CurveTubeMeshLayer`

Public regression examples exposing GPU record picking:

- `condition-monitoring-hero`
- `mixed-record-hero`
- `cross-space-dependency-hero`
- `mapping-dimensionality-hero`
- `dynamics-noise-hero`
- `relation-matrix-neighborhood`
- `process-curve-external-hero`

Public regression examples exposing GPU pair/edge picking:

- `mixed-record-hero`
- `cross-space-dependency-hero`
- `relation-matrix-neighborhood`
- `process-curve-external-hero`

Public regression examples exposing GPU field/curve picking probes:

- field probes hit on `condition-monitoring-hero`, `dynamics-noise-hero` and
  `process-curve-external-hero`
- curve probes hit on `condition-monitoring-hero`, `dynamics-noise-hero` and
  `process-curve-external-hero`

The protected `grae10-metric-engine` page is not a `MetricVisualRuntime`
inspection surface in the regression harness and reports no GPU-pickable
runtime.

## Fallback Coverage

Runtime inspection still reports deterministic fallback sources through:

- `relation-matrix-picking` for `RelationMatrixLayer` semantic cell picking
- `graph-picking` for relation-edge segment picking when a GPU edge hit is not
  available
- `cpu-fallback` for projected record-point picking

The public regression examples with runtime CPU fallback available are:

- `condition-monitoring-hero`
- `mixed-record-hero`
- `cross-space-dependency-hero`
- `mapping-dimensionality-hero`
- `dynamics-noise-hero`
- `relation-matrix-neighborhood`
- `process-curve-external-hero`

`relation-matrix-neighborhood` and `process-curve-external-hero` additionally
report `relation-matrix-picking` and `graph-picking` in
`inspection.availableSources`.

## Runtime State Keys

Selected record state is exposed by:

- `selectedRecordId`
- `selectedRecord`
- `selectedRecordPreview`

Selected pair state is exposed by:

- `selectedPair`
- `selectedPairPreview`

Shared selection/diagnostic state is exposed by:

- `selectionPreview`
- `selectionPresentation`
- `selectionFeatures`
- `selectionSource`
- `selectionPickSource`
- `inspection.source`
- `inspection.hover`
- `inspection.selection`
- `inspection.lastResult`
- `inspection.availableSources`

The runtime also reports source availability under:

- `inspection.gpuPicking`
- `inspection.relationMatrixPicking`
- `inspection.graphPicking`
- `inspection.cpuFallback`

## Checks Run

```bash
node visual/tools/check-runtime-picking-preview.mjs
node visual/tools/check-relation-matrix-picker.mjs
node visual/tools/check-views.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-visual-regression-public-examples.mjs
```

All commands passed. The browser regression wrote screenshots and
`results.json` under `output/visual/check-visual-regression-public-examples/`.
