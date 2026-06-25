# Relation Matrix Readability Workstream

Date: 2026-06-25

## Scope

This slice keeps relation matrices on the engine WebGL path. It does not add a
table, SVG renderer, DOM fallback, page-local canvas path, or JavaScript-derived
relation values.

Changed contract:

- `RelationMatrixLayer` consumes descriptor readability metadata for dense-cell
  LOD smoothing, alpha-weighted GPU tile-summary LOD textures, logical tile
  boundaries, and separate row/column/cell focus.
- `matrix-readability.js` derives a reusable readability profile from exported
  dense matrix evidence: block boundaries, tile summaries, LOD thresholds, and
  picker/fallback semantics.
- relation matrix descriptors now carry the semantic picker contract, native
  pair identity preservation, selected feature semantics, and no DOM/SVG
  fallback flags.
- headless checks cover block boundaries, dense-cell smoothing/LOD metadata,
  GPU tile-summary texture creation, selected row/column/cell semantics, native
  pair identity, and the absence of a DOM/SVG fallback in the relation-matrix
  engine modules.

## Acceptance Commands

Passed:

```bash
node --check visual/src/relational/matrix-readability.js
node --check visual/src/relational/RelationMatrixLayer.js
node --check visual/src/relational/descriptors.js
node --check visual/tools/check-relation-matrix-readability.mjs
node --check visual/tools/check-relation-matrix-picker.mjs
node visual/tools/check-relation-matrix-picker.mjs
node visual/tools/check-relation-matrix-readability.mjs
node visual/tools/check-views.mjs
node visual/tools/check-runtime-picking-preview.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-visual-performance-large-scenes.mjs
```

## Mechanics Proven

- Native block ranges from the relation-matrix evidence resolve to explicit
  block boundaries at indices `26`, `52`, `78`, and `104`.
- The 130-record native matrix exposes a 32-cell logical tile grid with full
  tile summary coverage.
- A 384-record dense matrix exposes a 64-cell logical tile grid and weighted
  3x3 dense-cell LOD metadata.
- `RelationMatrixLayer` creates a 5 x 5 alpha-weighted tile-summary texture
  for the native 130-record relation matrix from the exported RGBA matrix
  texture.
- The fragment shader exposes LOD smoothing, tile-summary texture, logical tile
  boundary, and separate row/column/cell selection uniforms.
- The semantic matrix picker still returns native pair evidence and stable
  relation/row/column pair identity.
- `RelationMatrixLayer.setSelection()` accepts pair, row-only, column-only,
  record, and explicit row/column index selection shapes.

## Remaining Gaps

- This is still not hero acceptance. Screenshot review is still required.
- The matrix still uses one source WebGL texture plus one derived tile-summary
  LOD texture. It is not yet a streamed multi-texture tile renderer for very
  large out-of-core matrices.
- Matrix picking remains the deterministic semantic picker. GPU/tiled matrix
  picking should be added only if the semantic picker becomes insufficient at
  larger matrix scales.
- Public relation-matrix pages still need final composition and screenshot
  baselines before the matrix/neighborhood view can be promoted from preview to
  accepted hero.
