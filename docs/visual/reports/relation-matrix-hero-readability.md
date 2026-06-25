# Relation Matrix Hero Readability

Date: 2026-06-25
Branch: `codex/visual-engine-evidence-exporters`

## Changed Files

- `visual/src/views/RelationMatrixView.js`
- `visual/src/relational/RelationMatrixLayer.js`
- `visual/tools/check-relation-matrix-readability.mjs`
- `visual/tools/check-relation-matrix-picker.mjs`
- `docs/visual/reports/relation-matrix-hero-readability.md`

Note: the active engine matrix layer in this tree is
`visual/src/relational/RelationMatrixLayer.js`; the older task path
`visual/src/layers/RelationMatrixLayer.js` is not present.

## Engine-Level Readability Improvements

- Strengthened the reusable `RelationMatrixView` focus defaults for selected
  row, column and cell states: stronger cell fill, clearer outline, stronger
  block boundaries, softer tile boundaries and a visible outer border.
- Added matrix focus material metadata from the view:
  `focusBackdropAlpha`, `focusBlockAlpha` and a descriptor `focusModel` that
  declares row/column/cell focus, runtime pair preview and linked graph
  emphasis.
- Extended the WebGL relation matrix shader to dim unfocused cells during
  selection and add selected row/column block-context bands. This reduces
  dense texture noise around a selected pair without adding DOM, SVG or canvas
  fallback rendering.
- Extended layer diagnostics with `selectedFocus`, `selectedPair` and
  `linkedGraph` metadata. When the selected pair is present in the native graph
  selection presentation, diagnostics now expose matching graph edge ids.
- Evidence counts and native pair identity were not changed. JavaScript still
  only encodes exported matrix values, builds render textures/LOD summaries and
  displays existing native pair/graph evidence.

## Commands Run

Passed:

```bash
node --check visual/src/views/RelationMatrixView.js
node --check visual/src/relational/RelationMatrixLayer.js
node --check visual/tools/check-relation-matrix-readability.mjs
node --check visual/tools/check-relation-matrix-picker.mjs
node visual/tools/check-relation-matrix-readability.mjs
node visual/tools/check-relation-matrix-picker.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-hero-screenshot-review.mjs
node visual/tools/check-visual-performance-large-scenes.mjs
```

Failed for unrelated preview manifest/assets outside this task scope:

```bash
node visual/tools/check-native-hero-evidence-scale.mjs
```

Failure details: `condition-monitoring-hero`, `cross-space-dependency-hero`
and `dynamics-noise-hero` now exceed their minimum record counts but still carry
`record-count-below-hero-minimum` blockers. The `relation-matrix-neighborhood`
row itself remained native-scale-ready with `recordCount: 130`,
`relationCount: 1`, and blockers limited to human visual acceptance.

## Readability And Picker Gate Results

- `check-relation-matrix-readability`: passed.
- `check-relation-matrix-picker`: passed.
- Matrix diagnostics still report `130 x 130`, `5` labeled blocks, full block
  coverage, `25` logical tiles, tile summary source
  `exported-relation-texture-downsample`, and `0` missing values.
- Picker proof still selects native pair `pc-000 -> pc-001` with value
  `2.34478`, native pair identity and pair key
  `process-curve-aligned-metric\0pc-000\0pc-001`.
- Added picker proof for a selected native graph pair, including matrix cell
  metadata plus linked graph edge diagnostics.

## Browser Gate Results

- `check-visual-regression-public-examples`: passed, `8` examples, `0` failed.
- `relation-matrix-neighborhood`: `public-preview-only`, render green,
  `3` runtime layers, primitives `InstancedPointLayer`, `RelationEdgeLayer`,
  `RelationMatrixLayer`.
- Runtime relation-matrix readability summary: `5` labeled blocks, full block
  coverage, `25` tiles, tile summary source
  `exported-relation-texture-downsample`.
- Browser interaction selected graph pair `pc-025 -> pc-076`; the runtime pair
  preview exposed pair value `0.642375`, row/column record ids, block/order
  properties, and linked graph selection presentation.
- `check-hero-screenshot-review`: passed. `relation-matrix-neighborhood`
  remains `review-pending`, not hero accepted.
- `check-visual-performance-large-scenes`: passed. Relation matrix grammar row:
  `recordCount: 130`, `descriptorCount: 3`, `runtimeLayerCount: 3`,
  median frame `8.3 ms`, `120.5 fps`, `0` issues.

## Screenshot Paths

- `/Users/michaelwelsch/Documents/metric/output/visual/check-visual-regression-public-examples/relation-matrix-neighborhood.png`
- `/Users/michaelwelsch/Documents/metric/output/visual/check-visual-regression-public-examples/relation-matrix-neighborhood.canvas.png`
- Browser report:
  `/Users/michaelwelsch/Documents/metric/output/visual/check-visual-regression-public-examples/results.json`
- Screenshot review report:
  `/Users/michaelwelsch/Documents/metric/output/visual/check-hero-screenshot-review/results.json`
- Performance report:
  `/Users/michaelwelsch/Documents/metric/output/visual/check-visual-performance-large-scenes/results.json`

## Remaining Human Screenshot Blocker

The remaining blocker is manual screenshot acceptance. Automated gates prove
native evidence, relation-matrix readability metadata, pair preview, graph
linkage, screenshots and performance, but `relation-matrix-neighborhood` still
has no human acceptance record clearing `visual-composition-not-human-accepted`
and `matrix-readability-not-human-accepted`. GRAE10 remains the only accepted
public hero.
