# Dynamics And Noise Composition Acceptance

## Purpose

Move the native dynamics/noise preview closer to accepted-hero quality by making
state evolution, propagation fields and reverse-flow evidence readable through
`DynamicsView`.

This task is about finite metric dynamics. It must not present dynamics as a
static point cloud, invent timeline states, compute dynamics in JavaScript or
draw page-local animation helpers.

## Owner Scope

Primary write scope:

- `visual/src/views/DynamicsView.js`
- `visual/examples/dynamics-noise-hero/index.html`
- optional checker: `visual/tools/check-dynamics-composition.mjs`
- report: `docs/visual/reports/dynamics-noise-composition-acceptance.md`

Allowed supporting files only if necessary:

- `visual/src/views/TrajectoryPathView.js`
- `visual/src/views/PropertyFieldView.js`
- `visual/src/layers/CurveRibbonLayer.js`
- `visual/src/layers/HeatFieldLayer.js`

Do not edit:

- `visual/examples/grae10-metric-engine/**`
- `visual/regression-baselines/grae10-metric-engine.sha256`
- native C++ exporters
- other public hero pages
- `visual/hero-acceptance.manifest.json`

## Current Evidence

Use only:

```text
docs/examples/assets/dynamics-noise/metric.visual.json
```

The native evidence currently has 512 records, exported trajectory/path
evidence and exported timeline states. The open blocker is visual composition.

Current screenshot reference:

```text
output/visual/check-visual-regression-public-examples/dynamics-noise-hero.canvas.png
```

Current issue: the visual must explain finite metric dynamics as state
evolution and reverse-flow structure. It must not look like a generic point
cloud with a field texture.

## Required Outcome

Improve `DynamicsView` and/or reusable layer options so the preview shows:

- trajectory/path evidence as the primary dynamics object;
- propagation or uncertainty field as readable support evidence;
- timeline state history with a clear current state, past/future context and
  non-debug playback metadata;
- lifted or grounded structure that makes reverse-flow/noise interpretation
  visible;
- hover/selection previews resolving exported timeline/record payload through
  the shared preview path;
- no JavaScript dynamics computation and no page-local renderer.

Prefer engine-level options such as trajectory sampling, path width/color
encoding, timeline state fading, field normalization, motion timing and
selection metadata.

## Acceptance

Run:

```bash
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-dynamics-motion-grammar.mjs
node visual/tools/check-dynamics-timeline-control.mjs
node visual/tools/check-timeline-motion-contract.mjs
node visual/tools/check-trajectory-path-view.mjs
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-views.mjs
node visual/tools/check-hero-visual-briefs.mjs
METRIC_VISUAL_EXAMPLES=dynamics-noise-hero node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-hero-screenshot-review.mjs
```

If a custom checker is added, it must prove at least:

- selected view kind is `dynamics`;
- descriptors include `CurveRibbonLayer`, `HeatFieldLayer` and
  `InstancedPointLayer`;
- timeline and trajectory evidence come from native exported fields;
- timeline controls are metadata-driven, not page debug controls;
- no synthetic evidence or page-local render helper is used.

Do not mark the hero accepted. Leave `visual-composition-not-human-accepted`
until a separate human review clears it.
