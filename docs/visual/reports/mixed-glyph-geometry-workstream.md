# Mixed Glyph Geometry Workstream

Status date: 2026-06-25

## Scope

Implemented the next mixed-record typed glyph geometry slice in the reusable
visual engine. The work stays inside the evidence -> semantic view -> descriptor
-> `InstancedGlyphLayer` runtime path. It does not compute METRIC algorithms,
does not add page-local rendering, and does not create fake application data.

## Implemented

- Added `metric.visual.record_glyph_render_semantics.v1` glyph render semantics
  in `visual/src/glyphs/index.js`.
- Added type-specific render geometry codes:
  `text-card`, `time-series-ribbon`, `histogram-panel`, `image-tile`,
  `vector-diamond` and `composed-dashboard`.
- Added type-specific material codes:
  `paper-ink`, `signal-glass`, `histogram-ceramic`, `image-luma-tile`,
  `vector-metal` and `composed-instrument`.
- `createRecordGlyphGrammar()` now emits `glyphGeometry` and `glyphMaterial`
  vec4 channels for every record instance, alongside the existing payload
  feature channels.
- `InstancedGlyphLayer` uploads and binds those channels as shader attributes:
  `aGlyphGeometry` and `aGlyphMaterial`.
- The glyph fragment shader now uses render geometry/material attributes for
  distinct silhouettes, marks, relief and material response instead of relying
  only on legacy `glyphType` branches.
- The glyph picking shader consumes `glyphGeometry`, so record picking masks
  match the rendered family silhouette.

## Evidence

- Native mixed-record export:
  `docs/examples/assets/mixed-records/metric.visual.json`
- Public mixed-record page still calls `showMixedRecords()` and installs one
  runtime descriptor set through `MixedRecordView`.
- The native export contains composed records; those render as composed
  dashboard/instrument instances while preserving typed component slots for
  text, time-series, histogram/image and vector payload evidence.
- Direct heterogeneous payload fixtures prove top-level text, time-series,
  histogram, image, vector and composed records map to distinct geometry and
  material shader attributes. Histogram and image still share the legacy
  `glyphType` code but now diverge through `glyphGeometry` and `glyphMaterial`.

## Verification

- `node --check visual/src/glyphs/index.js`
- `node --check visual/src/layers/InstancedGlyphLayer.js`
- `node --check visual/tools/check-glyph-record-grammar.mjs`
- `node --check visual/tools/check-mixed-glyph-geometry.mjs`
- `node visual/tools/check-glyph-record-grammar.mjs`
- `node visual/tools/check-mixed-glyph-geometry.mjs`
- `node visual/tools/check-visual-command-api.mjs`
- `node visual/tools/check-single-render-pipeline.mjs`
- `node visual/tools/check-views.mjs`
- `METRIC_VISUAL_EXAMPLES=mixed-record-hero node visual/tools/check-visual-examples.mjs`
- `METRIC_VISUAL_EXAMPLES=mixed-record-hero METRIC_VISUAL_FRAME_SAMPLES=8 node visual/tools/check-visual-regression-public-examples.mjs`

## Remaining Gaps

- The native mixed-record preview is still a small native export foundation
  with 20 composed records, not the public-scale mixed hero target.
- This slice improves typed glyph geometry inside the existing instanced
  billboard layer; it does not introduce separate mesh instancing per glyph
  family.
- Full all-grammar large-scene performance was not rerun because the available
  performance gate does not expose a single mixed-glyph row filter.
- Screenshot review is still required before promoting mixed records beyond
  public-preview status.
