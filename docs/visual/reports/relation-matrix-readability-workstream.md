# Relation Matrix Readability Workstream

Date: 2026-06-25

## Files Changed

- `visual/src/relational/matrix-readability.js`
- `visual/src/relational/diagnostics.js`
- `visual/src/relational/descriptors.js`
- `visual/src/relational/index.js`
- `visual/src/relational/RelationMatrixLayer.js`
- `visual/tools/check-relation-matrix-readability.mjs`
- `visual/tools/check-relation-matrix-picker.mjs`
- `visual/tools/check-visual-regression-public-examples.mjs`
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
- block labels: `normal reference`, `flat hold`, `late ramp`, `spike`,
  `early ramp`
- block coverage: `full`
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

The same browser output now includes a compact `relationMatrixReadability`
state for the relation matrix descriptor. The public relation-matrix preview
fails if it loses named block labels, full block coverage, tile-count metadata
or the `exported-relation-texture-downsample` LOD source.

## Checks Run

Passed:

```bash
node --check visual/src/relational/diagnostics.js
node --check visual/src/relational/matrix-readability.js
node --check visual/src/relational/descriptors.js
node --check visual/src/relational/RelationMatrixLayer.js
node --check visual/tools/check-relation-matrix-readability.mjs
node --check visual/tools/check-relation-matrix-picker.mjs
node --check visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-relation-matrix-readability.mjs
node visual/tools/check-relation-matrix-picker.mjs
node visual/tools/check-views.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
METRIC_VISUAL_EXAMPLES=relation-matrix-neighborhood node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-visual-performance-large-scenes.mjs
node visual/tools/check-hero-visual-briefs.mjs
node visual/tools/check-hero-screenshot-review.mjs
```

The isolated public browser regression for `relation-matrix-neighborhood`
passed with classification `public-preview-only`, relation-matrix picking
available, graph picking available, `650` graph edges, one relation-matrix
layer and named block-readability diagnostics in browser state.

## Performance Impact

No shader loop, texture upload path, or layer count was added. Runtime cost is a
small diagnostic object created from existing matrix, readability, tile-summary,
and selection state.

No new browser layer, postprocess pass, DOM overlay, SVG fallback or Canvas 2D
fallback was added.

`check-visual-performance-large-scenes.mjs` passed after this change.

## Remaining Gaps

- Screenshot review still keeps the relation matrix and neighborhood graph as
  `public-preview-only`; this is not hero acceptance.
- The renderer still uses one source matrix texture plus one derived
  tile-summary texture. It is not a streamed out-of-core tile renderer.
- The selected matrix layer diagnostics report current layer selection state;
  graph edge count remains on the graph descriptor diagnostics rather than a
  single combined cross-layer diagnostic object.
