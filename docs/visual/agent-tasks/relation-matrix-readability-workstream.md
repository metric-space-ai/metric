# Agent Task: Relation Matrix Readability Workstream

## Purpose

Improve the relation-matrix and neighborhood-graph grammar until pairwise
evidence is readable as pairwise evidence. This work must strengthen the
reusable engine/layer/view path, not a page-local screenshot trick.

## Owner Scope

One worker owns only:

```text
visual/src/relational/*
visual/src/views/RelationMatrixView.js
visual/src/views/NeighborhoodGraphView.js
visual/src/layers/RelationMatrixLayer.js
visual/tools/check-relation-*.mjs
docs/visual/reports/relation-matrix-readability-workstream.md
```

Do not edit command API, glyph/field grammar, mapping/dynamics grammar, native
C++ exporters, project page, GRAE10, or unrelated examples. Other workers may
edit interaction or regression tooling in parallel.

## Required Behavior

- Matrix readability must be an engine contract:
  - block ordering and block boundaries from exported evidence
  - tile and LOD metadata for dense matrices
  - row, column and selected-cell focus as distinct states
  - missing/non-present cells handled without visual noise
  - no HTML table, SVG fallback or page-local Canvas 2D matrix renderer
- Neighborhood graph must remain tied to the same exported relation and record
  identities as the matrix.
- Pair selection must expose the selected pair to the shared runtime preview.
- Diagnostics must report:
  - matrix dimensions
  - block count
  - tile count or tile summary source
  - missing-value count
  - selected row/column/cell state
  - graph edge count and relation id
- Add a stress fixture or stress check for a larger dense matrix if the current
  native fixture is too small to reveal aliasing.

## Stop Rules

- Stop if readability requires custom DOM, SVG, or per-page Canvas 2D drawing.
- Stop if the matrix and graph use independent pair sources or independent
  selection state.
- Stop if JavaScript computes relation values that are absent from evidence.
- Do not touch the protected GRAE10 visual reference.

## Acceptance

Run:

```bash
node visual/tools/check-relation-matrix-readability.mjs
node visual/tools/check-relation-matrix-picker.mjs
node visual/tools/check-views.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
```

If a browser-facing behavior changes, also run:

```bash
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-visual-performance-large-scenes.mjs
```

## Report

Write `docs/visual/reports/relation-matrix-readability-workstream.md` with:

- files changed
- exact readability issue addressed
- selected-pair proof used in checks
- performance impact
- whether screenshot review still keeps this as public-preview-only
