# Linked Selection Presentation Workstream

Status date: 2026-06-25

## Implemented

- Runtime selection now carries `selectionPresentation` alongside the existing
  selected record, selected pair and preview fields.
- The presentation contract is emitted from `visual/src/selection/index.js` as
  `metric.visual.linked_selection_presentation.v1`.
- Record and pair selections are resolved against current runtime descriptors,
  so the state exposes related metric-space record features, relation-matrix
  row/column/cell features, relation graph edges and paired-space bridge
  features without page-local selection state or synthetic data.
- Runtime hover/click inspection details include the same presentation payload,
  preserving the existing `PickResult` and preview contracts.
- `MetricVisualSurface` pair preview clicks now route through
  `MetricVisualRuntime.inspectAt()` and no longer write a selected-pair DOM
  dataset marker.

## Checks

Focused gates:

```bash
node --check visual/src/selection/index.js
node --check visual/src/runtime/runtime.js
node --check visual/src/metric-visual.js
node --check visual/tools/check-linked-selection-presentation.mjs
node --check visual/tools/check-runtime-picking-preview.mjs
node visual/tools/check-runtime-picking-preview.mjs
node visual/tools/check-linked-selection-presentation.mjs
node visual/tools/check-relation-matrix-picker.mjs
node visual/tools/check-views.mjs
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-single-render-pipeline.mjs
METRIC_VISUAL_OUT=/tmp/metric-linked-selection-regression node visual/tools/check-visual-regression-public-examples.mjs
```

The linked-selection gate uses native cross-space exported evidence from
`docs/examples/assets/cross-space-dependency/metric.visual.json`.
All commands above passed.

## Remaining Gaps

- `RelationEdgeLayer` still renders static edge colors. The new runtime state
  carries the graph-edge and paired-space bridge features needed for edge
  highlighting, but shader-level edge emphasis is a later layer implementation
  slice.
- Public previews remain preview-only until screenshot review accepts each hero
  grammar.
