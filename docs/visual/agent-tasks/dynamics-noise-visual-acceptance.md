# Agent Task: Dynamics And Noise Visual Acceptance

## Owner

One worker owns only:

- `visual/examples/dynamics-noise-hero/index.html`
- `visual/reports/W8-dynamics-noise-hero.md`
- `visual/output/W8-dynamics-noise-hero-start.png`
- `visual/output/W8-dynamics-noise-hero-middle.png`
- `visual/output/W8-dynamics-noise-hero-end.png`
- dynamics checks under `visual/tools/check-dynamics-*.mjs` and
  `visual/tools/check-timeline-*.mjs`
- only if strictly required by this example:
  `visual/src/views/DynamicsView.js` and
  `visual/src/views/TrajectoryPathView.js`

Do not edit GRAE10, native exporters, command API, project page, condition,
mixed-record, cross-space, mapping or relation-matrix files. Other agents may
be working in parallel; do not revert their changes.

## Goal

Make finite dynamics/noise evidence visible as movement over exported metric
relations and timeline states.

## Visual Claim

Noise-like behavior can be interpreted as movement over a finite metric
relation structure; reverse flow or propagation reveals how states move through
that structure.

## Input Evidence

Use only:

```text
docs/examples/assets/dynamics-noise/metric.visual.json
```

Do not simulate dynamics, transition costs, uncertainty or timeline states in
JavaScript.

## Required Engine Grammar

Primary:

- `DynamicsView`
- `TrajectoryPathView`
- exported timeline state controls
- propagation field, uncertainty field or transition-cost layer

Supporting:

- record preview showing exported state history
- metric-space points only as context

## Acceptance

- Start, middle and end states are visibly different.
- Motion reads as a metric-space process, not a decorative animation.
- The ground/field changes with exported state where evidence exists.
- Hover/selection exposes record history through the shared preview system.
- The view is not a static point cloud.
- The report states whether this is still review-pending or manually accepted.

## Validation

Run:

```bash
node visual/tools/check-visual-document.mjs docs/examples/assets/dynamics-noise/metric.visual.json
node visual/tools/check-dynamics-motion-grammar.mjs
node visual/tools/check-dynamics-timeline-control.mjs
node visual/tools/check-timeline-motion-contract.mjs
node visual/tools/check-trajectory-path-view.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-grae10-golden.mjs
```

The report must include the local URL used, screenshot paths, native evidence
path, timeline state count, trajectory evidence, and any remaining blocker.
