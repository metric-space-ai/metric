# Agent Task: Relation Matrix Visual Acceptance

## Owner

One worker owns only:

- `visual/examples/relation-matrix-neighborhood/index.html`
- `visual/reports/W9-relation-matrix-neighborhood.md`
- `visual/output/W9-relation-matrix-neighborhood.png`
- relation-matrix checks under `visual/tools/check-relation-matrix-*.mjs`
- only if strictly required by this example:
  `visual/src/views/RelationMatrixView.js` and
  `visual/src/views/NeighborhoodGraphView.js`

Do not edit GRAE10, native exporters, command API, project page, condition,
mixed-record, cross-space, mapping or dynamics files. Other agents may be
working in parallel; do not revert their changes.

## Goal

Make pair relations readable as a matrix/neighborhood grammar, not as
decorative texture noise.

## Visual Claim

A finite metric space is pairwise evidence. A readable relation matrix and
local graph reveal families, boundaries, neighborhoods and outliers without
requiring a vector embedding first.

## Input Evidence

Use only:

```text
docs/examples/assets/relation-matrix/metric.visual.json
```

Do not compute pair relations, blocks or neighborhood graph values in
JavaScript.

## Required Engine Grammar

Primary:

- `RelationMatrixView`
- readable block/tile/LOD matrix
- row/column/cell focus
- pair preview through shared preview system

Supporting:

- `NeighborhoodGraphView` for selected rows/blocks
- block labels and relation diagnostics from exported evidence

## Acceptance

- Matrix cells do not alias into unreadable stripes.
- Blocks, row/column focus and selected pair are legible.
- Neighborhood graph is visibly tied to selected matrix evidence.
- Hover/selection exposes pair evidence through the shared preview system.
- The screenshot can be reviewed as a relation view, not a point-cloud view.
- The report states whether this is still review-pending or manually accepted.

## Validation

Run:

```bash
node visual/tools/check-visual-document.mjs docs/examples/assets/relation-matrix/metric.visual.json
node visual/tools/check-relation-matrix-readability.mjs
node visual/tools/check-relation-matrix-picker.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-grae10-golden.mjs
```

The report must include the local URL used, screenshot path, native evidence
path, matrix size, block count, selected cell behavior, and any remaining
blocker.
