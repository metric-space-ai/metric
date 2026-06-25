# Linked Edge Emphasis Workstream

Status date: 2026-06-25

## Implemented

- `RelationEdgeLayer` now accepts runtime selection state through
  `setSelection(selection)`.
- Edge emphasis is resolved from existing linked-selection presentation payloads,
  selected pair identity, edge ids, source/target record ids and row/column ids.
- The normal edge shader consumes a per-vertex `aEdgeEmphasis` attribute to boost
  selected or related edges while dimming non-selected edges during an active
  selection.
- The picking pass remains on the existing position and pick-color buffers, so
  picking masks are not affected by visual emphasis.

## Checks

Focused gates:

```bash
node --check visual/src/layers/RelationEdgeLayer.js
node --check visual/tools/check-linked-edge-emphasis.mjs
node visual/tools/check-linked-edge-emphasis.mjs
node visual/tools/check-linked-selection-presentation.mjs
node visual/tools/check-runtime-picking-preview.mjs
node visual/tools/check-single-render-pipeline.mjs
```

## Remaining Gaps

- The emphasis is color and alpha based. Browser line-width limits still prevent
  reliable per-edge GPU line thickening without moving to mesh-backed edges.
