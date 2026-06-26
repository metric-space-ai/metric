# Relation Matrix Hero Readability

Date: 2026-06-26

## Scope

This pass stayed inside the reusable relation-matrix engine slice. It did not
edit public page code, GRAE10, native exporters, project site files, DOM/SVG
fallbacks, or fake data.

## Changed Files

- `visual/src/views/RelationMatrixView.js`
- `visual/src/relational/RelationMatrixLayer.js`
- `visual/src/relational/matrix-readability.js`
- `visual/src/relational/diagnostics.js`
- `visual/src/relational/descriptors.js`
- `visual/tools/check-relation-matrix-readability.mjs`
- `visual/tools/check-relation-matrix-picker.mjs`
- `docs/visual/reports/relation-matrix-hero-readability.md`

Note: the active matrix layer in this tree is
`visual/src/relational/RelationMatrixLayer.js`.

## Engine-Level Improvements

- Tightened `RelationMatrixView` and descriptor defaults for selected
  row/column/cell focus: stronger focus backdrop, block context, cell fill,
  cell outline and outer border.
- Made block boundaries read more crisply in `RelationMatrixLayer` by drawing a
  separate shader boundary core and soft halo instead of one blended boundary.
- Added selected row and column edge guides in the shader so row/column focus is
  readable as a band plus boundary, not only as a tint.
- Reduced default tile-boundary emphasis so logical tiles still diagnose LOD
  behavior without competing with exported block structure.
- Extended readability metadata and diagnostics with block-boundary geometry,
  tile-summary coverage, focus presentation semantics and the graph-first,
  screen-readable matrix overlay rule.
- Preserved native pair identity. JavaScript still encodes exported relation
  values into GPU texture/LOD data and does not compute metric values,
  neighborhoods or synthetic evidence.

## Checks

Passed:

```bash
node --check visual/src/views/RelationMatrixView.js
node --check visual/src/relational/RelationMatrixLayer.js
node --check visual/src/relational/matrix-readability.js
node --check visual/src/relational/diagnostics.js
node --check visual/src/relational/descriptors.js
node --check visual/tools/check-relation-matrix-readability.mjs
node --check visual/tools/check-relation-matrix-picker.mjs
node visual/tools/check-relation-matrix-readability.mjs
node visual/tools/check-relation-matrix-picker.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-hero-visual-briefs.mjs
METRIC_VISUAL_EXAMPLES=relation-matrix-neighborhood METRIC_VISUAL_OUT=output/visual/relation-matrix-readability-regression node visual/tools/check-visual-regression-public-examples.mjs
```

The required `check-hero-visual-briefs` gate used the existing full public
regression report at
`output/visual/check-visual-regression-public-examples/results.json`, generated
at `2026-06-26T19:08:30.788Z`, with all 8 public rows present.

Attempted but stopped:

```bash
node visual/tools/check-visual-regression-public-examples.mjs
```

The full public browser regression produced no output for roughly five minutes,
so it was stopped with `SIGINT` (`exit 130`). To verify this slice's pixel path
without replacing the full report, the targeted relation-matrix regression was
run in `output/visual/relation-matrix-readability-regression` and passed.

## Current Contract Evidence

- Readability checker: `130 x 130` dense matrix, 5 labelled blocks, full block
  coverage, 25 logical tiles, tile summary source
  `exported-relation-texture-downsample`, 0 missing values.
- Picker checker: native cell `pc-000 -> pc-001` resolves with value
  `2.34478`, pair key
  `process-curve-aligned-metric\0pc-000\0pc-001`, and native pair evidence.
- Targeted browser regression: `relation-matrix-neighborhood` passed as
  `public-preview-only` with `InstancedPointLayer`, `RelationEdgeLayer` and
  `RelationMatrixLayer`; `ScreenReadableOverlayPass` remained present.
- Brief gate: `relation-matrix-neighborhood` remains review-pending with
  blockers `visual-composition-not-human-accepted` and
  `matrix-readability-not-human-accepted`.

## Artifacts

- Current targeted canvas screenshot:
  `output/visual/relation-matrix-readability-regression/relation-matrix-neighborhood.canvas.png`
- Targeted browser report:
  `output/visual/relation-matrix-readability-regression/results.json`
- Brief report:
  `output/visual/check-hero-visual-briefs/results.json`
- Gallery evidence report:
  terminal output from `node visual/tools/check-public-gallery-evidence.mjs`

## Remaining Blocker

The remaining blocker is human screenshot acceptance. Automated gates now prove
the WebGL-only matrix readability contract, block/tile diagnostics,
row/column/cell focus semantics, native pair picking and graph/matrix render
hierarchy, but the preview is still not hero accepted. GRAE10 remains the only
accepted public hero.
