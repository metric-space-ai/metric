# Relation Matrix Readability Workstream

Date: 2026-06-25

## Files Changed

- `visual/src/relational/diagnostics.js`
- `visual/src/relational/descriptors.js`
- `visual/src/relational/index.js`
- `visual/src/relational/RelationMatrixLayer.js`
- `visual/tools/check-relation-matrix-readability.mjs`
- `visual/tools/check-relation-matrix-picker.mjs`
- `docs/visual/reports/relation-matrix-readability-workstream.md`

No command API, glyph/field grammar, mapping/dynamics grammar, native C++
exporter, project page, GRAE10, or unrelated example files were changed.

## Readability Issue Addressed

The matrix rendering path already used the reusable WebGL relation layer, but
diagnostics did not expose the full readability contract in one durable payload.
This pass adds `metric.visual.relation_matrix_readability_diagnostics.v1` so
descriptors and runtime layer diagnostics report:

- matrix dimensions: `130 x 130`
- block count: `5`, with `4` shader block boundaries
- tile count: `25`, tile size `32`
- tile summary source: `exported-relation-texture-downsample`
- missing-value count: `0`
- selected row, column, and cell state: `none`, `row`, `column`, or `cell`

Neighborhood graph descriptors now also repeat `relationId`, `edgeCount`, and
`graphEdgeCount` in diagnostics, while keeping the graph tied to the same
relation and record ids as the matrix.

## Selected-Pair Proof

`check-relation-matrix-picker.mjs` selects the native matrix cell at row
`pc-000`, column `pc-001` for relation `process-curve-aligned-metric`. The
selected pair carries value `2.34478`, `present: true`, pair key
`process-curve-aligned-metric\0pc-000\0pc-001`, and native pair evidence.

The browser regression output for `relation-matrix-neighborhood` also confirmed
a shared runtime selected pair from graph picking: `pc-025 -> pc-076`, relation
`process-curve-aligned-metric`, with one matching relation-matrix cell and two
matching graph edges in the selection presentation.

## Checks Run

Passed:

```bash
node --check visual/src/relational/diagnostics.js
node --check visual/src/relational/descriptors.js
node --check visual/src/relational/RelationMatrixLayer.js
node --check visual/tools/check-relation-matrix-readability.mjs
node --check visual/tools/check-relation-matrix-picker.mjs
node visual/tools/check-relation-matrix-readability.mjs
node visual/tools/check-relation-matrix-picker.mjs
node visual/tools/check-views.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-visual-performance-large-scenes.mjs
```

Optional browser regression:

```bash
node visual/tools/check-visual-regression-public-examples.mjs
```

Overall result: failed `1 / 7` because `condition-monitoring-hero` reported
three page errors. The `relation-matrix-neighborhood` row passed with no issues,
classification `public-preview-only`, relation-matrix picking available, graph
picking available, `650` graph edges, and one relation-matrix layer.

## Performance Impact

No shader loop, texture upload path, or layer count was added. Runtime cost is a
small diagnostic object created from existing matrix, readability, tile-summary,
and selection state.

`check-visual-performance-large-scenes.mjs` passed. The relation-matrix
neighborhood row reported median frame time `8.3 ms`, p95 `8.4 ms`, `120.5 fps`,
`48,584` GPU buffer bytes, `951` draw calls, and no issues. The generic large
scene rows also passed at `1,000`, `10,000`, and `60,000` records with median
frame time `8.3 ms`.

## Remaining Gaps

- Screenshot review still keeps the relation matrix and neighborhood graph as
  `public-preview-only`; this is not hero acceptance.
- The renderer still uses one source matrix texture plus one derived
  tile-summary texture. It is not a streamed out-of-core tile renderer.
- The selected matrix layer diagnostics report current layer selection state;
  graph edge count remains on the graph descriptor diagnostics rather than a
  single combined cross-layer diagnostic object.
