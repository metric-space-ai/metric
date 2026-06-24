# Agent Task: Relation Matrix And Graph Grammar Workstream

## Owner

One worker owns only:

- `visual/src/relational/*`
- `visual/src/views/RelationMatrixView.js`
- `visual/src/views/NeighborhoodGraphView.js`
- relation-specific checks under `visual/tools/check-relation-*.mjs`
- `visual/examples/relation-matrix-neighborhood/index.html` only as a thin
  acceptance fixture
- relation-specific progress notes in `docs/visual/metric-visual-progress.md`

Do not edit `visual/src/metric-visual.js`, generic runtime internals, C++
exporters, project page, or unrelated examples. Other workers may edit command
API, glyphs, mapping/dynamics or regression tools in parallel; do not revert
their changes.

## Goal

Make relation evidence readable as relation evidence, not as another point
cloud. The grammar must show a dense relation matrix, sparse neighborhood graph,
pair preview, and linked row/column/cell selection through one shared runtime
selection model.

## Required Behavior

- Relation matrix:
  - supports block/tile ordering supplied by native evidence
  - keeps cells readable at the page scale used by the public example
  - exposes selected row, selected column and selected cell as visible grammar
    features
  - supports missing/non-present cells without visual noise
- Neighborhood graph:
  - uses the same relation and record IDs as the matrix
  - responds to selected record or selected pair where applicable
  - does not compute graph edges in JavaScript when native graph evidence is
    present
- Pair preview:
  - is engine-level, not page DOM
  - shows row record, column record, relation name/id, value and optional pair
    properties
- Diagnostics:
  - relation matrix dimensions
  - graph edge count
  - selected pair identity
  - missing-value count
  - metric-law diagnostic references when exported

## Hard Stop Rules

- Stop if a matrix view needs an HTML table, SVG, or per-page Canvas 2D drawing
  to be readable. Fix the relation grammar/layer instead.
- Stop if matrix and graph use different pair sources or independent selection
  state.
- Do not modify GRAE10.

## Acceptance

Run:

```bash
node visual/tools/check-relation-matrix-picker.mjs
node visual/tools/check-views.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
```

Browser-verify:

- hover over a matrix cell shows pair preview
- click a matrix cell updates runtime selected pair
- the selected row/column/cell is visible in the matrix layer
- the graph remains visible and tied to the same native relation

Report the exact selected pair used for browser verification.
