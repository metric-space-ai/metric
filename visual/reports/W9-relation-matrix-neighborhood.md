# W9 Relation Matrix Visual Acceptance

Status date: 2026-06-25

## Status

Review-pending screenshot candidate, not manually accepted as a public hero.

## Scope

- Example: `visual/examples/relation-matrix-neighborhood/index.html`
- Native evidence: `docs/examples/assets/relation-matrix/metric.visual.json`
- Screenshot target: `visual/output/W9-relation-matrix-neighborhood.png`
- Engine modules touched: none in this pass
- Checks used: `visual/tools/check-relation-matrix-readability.mjs`,
  `visual/tools/check-relation-matrix-picker.mjs`

The page uses `createMetricVisual()` and
`showRelationMatrixNeighborhood()`. It does not compute pair relations, blocks
or neighborhood graph values in JavaScript.

## Evidence Summary

- Native records: 130 process-curve records
- Dense relation: `process-curve-aligned-metric`
- Dense values: 8,385 upper-triangle values mirrored by the matrix layer
- Blocks: 5 exported block ranges
- Graph: `process-curve-knn` with 650 exported edges
- Coordinate state: `process-curve-block-layout-3d`

## Visual Grammar

- Primary semantic view: `RelationMatrixView`
- Primary primitive: `RelationMatrixLayer`
- Supporting semantic view: `NeighborhoodGraphView`
- Supporting primitives: `InstancedPointLayer`, `RelationEdgeLayer`
- Interaction: matrix row/column/cell focus and pair preview through the shared
  runtime selection path

## Validation

Commands run:

```bash
node visual/tools/check-relation-matrix-readability.mjs
node visual/tools/check-relation-matrix-picker.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
```

Result:

- relation-matrix readability check: pass
- relation-matrix picker check: pass
- public gallery evidence gate: pass
- GRAE10 golden reference: pass

The checks verified readable matrix metadata, tile/block contracts and semantic
pair picking from native relation evidence.

## Remaining Blockers

- Manual screenshot review has not accepted this as a public hero.
- The composition still needs visual review for matrix legibility at public
  gallery size.

Status:

- loads: yes by command/example checks
- renders: yes by descriptor checks
- interactive: semantic matrix picker checks pass
- visually accepted: no
- complete: no, review-pending
