# Agent Task: Cross-Space Dependency Visual Acceptance

## Owner

One worker owns only:

- `visual/examples/cross-space-dependency-hero/index.html`
- `visual/reports/W7-cross-space-dependency-hero.md`
- `visual/output/W7-cross-space-dependency-hero.png`
- cross-space checks under `visual/tools/check-cross-space-*.mjs`
- only if strictly required by this example:
  `visual/src/views/CrossSpaceView.js` and cross-space interaction helpers in
  `visual/src/interaction/`

Do not edit GRAE10, native exporters, command API, project page, condition,
mixed-record, mapping, dynamics or relation-matrix files. Other agents may be
working in parallel; do not revert their changes.

## Goal

Make cross-space dependence visible as two linked finite metric spaces with
exported pair/dependence evidence. Do not merge both spaces into one generic
point-cloud scene.

## Visual Claim

Dependence can be inspected between metric spaces without forcing both spaces
into one shared vector coordinate system.

## Input Evidence

Use only:

```text
docs/examples/assets/cross-space-dependency/metric.visual.json
```

Do not compute dependence, local contribution or paired neighborhoods in
JavaScript.

## Required Engine Grammar

Primary:

- `CrossSpaceView`
- paired-space layout
- linked selection between spaces
- dependence bridge, contribution field or paired-neighborhood relation layer

Supporting:

- point clouds only as per-space spatial context
- pair preview through the shared preview system

## Acceptance

- Two metric spaces are visually distinct.
- Selecting or hovering in one space visibly affects the paired records in the
  other space.
- Local dependence contribution is visible as evidence, not decoration.
- The global statistic is shown as exported evidence.
- The screenshot does not look like a single generic point cloud.
- The report states whether this is still review-pending or manually accepted.

## Validation

Run:

```bash
node visual/tools/check-visual-document.mjs docs/examples/assets/cross-space-dependency/metric.visual.json
node visual/tools/check-cross-space-linked-selection.mjs
node visual/tools/check-linked-selection-presentation.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-grae10-golden.mjs
```

The report must include the local URL used, screenshot path, native evidence
path, paired-record count, selected-pair behavior, and any remaining blocker.
