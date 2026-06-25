# Agent Task: Mapping And Dimensionality Visual Acceptance

## Owner

One worker owns only:

- `visual/examples/mapping-dimensionality-hero/index.html`
- `visual/reports/W6-mapping-dimensionality-hero.md`
- `visual/output/W6-mapping-dimensionality-hero-source.png`
- `visual/output/W6-mapping-dimensionality-hero-transition.png`
- `visual/output/W6-mapping-dimensionality-hero-target.png`
- mapping checks under `visual/tools/check-mapping-*.mjs`
- only if strictly required by this example:
  `visual/src/views/MappingView.js` and `visual/src/views/MorphView.js`

Do not edit GRAE10, native exporters, command API, project page, condition,
mixed-record, cross-space, dynamics or relation-matrix files. Other agents may
be working in parallel; do not revert their changes.

## Goal

Make mapping/dimensionality evidence visible as a metric-space transformation:
source state, quick morph, target state and residual/error evidence.

## Visual Claim

A mapping is a transformation of metric-space evidence. The visual question is
which structure is preserved, compressed or distorted.

## Input Evidence

Use only:

```text
docs/examples/assets/mapping-dimensionality/metric.visual.json
```

Do not compute mapping, residuals, coordinates or preservation scores in
JavaScript.

## Required Engine Grammar

Primary:

- `MappingView`
- source and target coordinate states for the same record IDs
- residual/error layer or projection
- hold, quick transition and hold timing

Supporting:

- record preview with exported local residual/error values
- labels for visible structure families when present in evidence

## Acceptance

- Source, transition and target frames are visually distinct.
- Residual/error evidence is visible without a side panel.
- Morph timing contains holds and a quick transition, not a constant slow loop.
- Hover/selection exposes local mapping evidence.
- The screenshot set can be reviewed as a transformation, not a static point
  cloud.
- The report states whether this is still review-pending or manually accepted.

## Validation

Run:

```bash
node visual/tools/check-visual-document.mjs docs/examples/assets/mapping-dimensionality/metric.visual.json
node visual/tools/check-mapping-motion-grammar.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-grae10-golden.mjs
```

The report must include the local URL used, three screenshot paths, native
evidence path, residual channel name, morph timing, and any remaining blocker.
