# Agent Task: Mixed Records Visual Acceptance

## Owner

One worker owns only:

- `visual/examples/mixed-record-hero/index.html`
- `visual/reports/W5-mixed-record-hero.md`
- `visual/output/W5-mixed-record-hero.png`
- mixed-record checks under `visual/tools/check-mixed-*.mjs`
- only if strictly required by this example:
  `visual/src/views/MixedRecordView.js`,
  `visual/src/layers/InstancedGlyphLayer.js` and `visual/src/glyphs/*`

Do not edit GRAE10, native exporters, command API, project page, condition,
cross-space, mapping, dynamics or relation-matrix files. Other agents may be
working in parallel; do not revert their changes.

## Goal

Make the mixed-record preview prove heterogeneous record handling through the
engine. This is not a multimodal-ML claim; it is a finite-metric-space evidence
view over exported typed records and exported pair relations.

## Visual Claim

Records of different original forms can be inspected in one finite metric-space
workflow once native metrics export comparable relation evidence.

## Input Evidence

Use only:

```text
docs/examples/assets/mixed-records/metric.visual.json
```

Do not add page-local synthetic evidence. Do not fabricate text, image, curve
or table payloads in JavaScript.

## Required Engine Grammar

Primary:

- `MixedRecordView`
- typed glyphs per record family
- cross-type relation edges
- typed record preview

Supporting:

- metric-space coordinates only as layout context
- labels that identify record families, not generic clusters

## Acceptance

- At least four exported record types are visibly distinct.
- Cross-type relation edges are visible and selectable.
- Hover/selection proves original record heterogeneity via preview payloads.
- The screenshot is dense enough for project-page review.
- The view does not collapse into colored dots.
- The report states whether this is still review-pending or manually accepted.

## Validation

Run:

```bash
node visual/tools/check-visual-document.mjs docs/examples/assets/mixed-records/metric.visual.json
node visual/tools/check-glyph-record-grammar.mjs
node visual/tools/check-mixed-glyph-geometry.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-grae10-golden.mjs
```

The report must include the local URL used, the screenshot path, native evidence
path, visible record types, preview payload families, and any remaining blocker.
