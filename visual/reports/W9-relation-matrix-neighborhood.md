# W9 Relation Matrix Visual Acceptance

Status date: 2026-06-26

## Status

Review-pending screenshot candidate. This is not manually accepted as a public
hero.

## Scope

- Example: `visual/examples/relation-matrix-neighborhood/index.html`
- Native evidence: `docs/examples/assets/relation-matrix/metric.visual.json`
- Screenshot: `visual/output/W9-relation-matrix-neighborhood.png`
- Browser capture URL:
  `http://127.0.0.1:55754/visual/examples/relation-matrix-neighborhood/index.html?verify=1&w9=selected-native-edge`
- Engine modules touched: none

The page uses `createMetricVisual()` and
`showRelationMatrixNeighborhood()`. It loads only
`docs/examples/assets/relation-matrix/metric.visual.json`; pair relations,
block ranges and neighborhood graph values are taken from exported evidence,
not recomputed in the example JavaScript.

## Native Evidence

- Native records: 130 process-curve records
- Dense relation: `process-curve-aligned-metric`
- Matrix size: 130 x 130
- Dense values: 8,385 upper-triangle values mirrored by the matrix layer
- Blocks: 5 exported block ranges
- Block labels: normal reference, flat hold, late ramp, spike, early ramp
- Graph: `process-curve-knn`
- Graph edges: 650 exported directed edges
- Coordinate state: `process-curve-block-layout-3d`
- Metric operator: `AlignedCurveDistance`

## Visual Grammar

- Primary semantic view: `RelationMatrixView`
- Primary primitive: `RelationMatrixLayer`
- Supporting semantic view: `NeighborhoodGraphView`
- Supporting primitives: `InstancedPointLayer`, `RelationEdgeLayer`
- Interaction: matrix row, column and cell focus with pair preview through the
  shared runtime selection path

## Browser Evidence

The screenshot was captured after selecting the matrix cell for native graph
edge evidence `pc-002 -> pc-105`.

- Selected matrix cell: row 2, column 105
- Selected pair: `pc-002` <-> `pc-105`
- Preview title: normal reference / variant 2 <-> early ramp / variant 1
- Pair value: `0.3429`
- Pick source: `relation-matrix-picking`
- Native pair evidence: present
- Linked graph edges in runtime selection presentation:
  `pc-002->pc-105:10`, `pc-105->pc-002:525`
- Linked selection counts: 1 relation-matrix cell, 2 graph edges

This screenshot can be reviewed as a relation-matrix/neighborhood view: the
matrix blocks remain legible, the selected row and column cross block
boundaries, and the pair preview exposes the native pair identity and value.

## Validation

Commands run:

```bash
node visual/tools/check-visual-document.mjs docs/examples/assets/relation-matrix/metric.visual.json
node visual/tools/check-relation-matrix-readability.mjs
node visual/tools/check-relation-matrix-picker.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-grae10-golden.mjs
```

Result:

- visual document check: pass
- relation-matrix readability check: pass
- relation-matrix picker check: pass
- hero grammar contract: pass
- public gallery evidence gate: pass
- public examples visual regression: pass, 8 total, 0 failed
- GRAE10 golden reference: pass

## Remaining Blockers

- Manual screenshot review has not accepted this as a public hero.

Status:

- loads: yes
- renders: yes
- interactive: yes
- visually accepted: no
- complete: review-pending
