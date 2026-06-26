# Dynamics Noise Composition Acceptance

Date: 2026-06-26

## Changed Files

- `visual/src/views/DynamicsView.js`
- `visual/src/views/TrajectoryPathView.js`
- `visual/examples/dynamics-noise-hero/index.html`
- `visual/tools/check-dynamics-composition.mjs`
- `docs/visual/reports/dynamics-noise-composition-acceptance.md`

## Scope Result

The dynamics/noise preview still uses only:

```text
docs/examples/assets/dynamics-noise/metric.visual.json
```

No GRAE10 files, hero acceptance manifests, native exporters, synthetic
evidence, or other hero pages were edited.

`DynamicsView` now owns the composition improvements:

- deterministic trajectory display thinning with native candidate counts
  preserved in metadata;
- exported record-property color and width encoding for trajectory paths;
- lifted field metadata that ties the record property field to exported
  timeline-step state;
- timeline state-history context point layers for past/future exported states;
- current-state point metadata marked separately from context states.

`TrajectoryPathView` now applies exported record-property color/width channels
to paths generated from exported timeline states. The dynamics page only passes
engine options to `showDynamics()` and reads timeline state from descriptor
metadata for the readout.

## Descriptor Evidence

The dynamics composition checker verifies the page and view emit:

- `CurveRibbonLayer`
- `HeatFieldLayer`
- `InstancedPointLayer`

The trajectory descriptor reports `512` native timeline-state path candidates
and `192` displayed paths via deterministic record stride. The trajectory path
encoding uses exported record properties:

- color: `reconstruction-improvement`
- width: `best-reconstruction-error`

The field descriptor uses exported `best-reconstruction-error` record values
and exported `reverse-mse-to-clean` timeline-step samples. The timeline control
is `metric.visual.timeline_control.v1` with state, playback and reset controls.

## Acceptance Checks

Passed:

- `node --check visual/src/views/DynamicsView.js`
- `node --check visual/src/views/TrajectoryPathView.js`
- `node --check visual/tools/check-dynamics-composition.mjs`
- `node visual/tools/check-dynamics-composition.mjs`
  - `ok: true`
  - `13` checks, `0` failures
- `node visual/tools/check-grae10-golden.mjs`
  - GRAE10 hash:
    `464f6a90c36c1e9c6b4ec90068500dc226740d65b251918aca567f99d64d3d5e`
- `node visual/tools/check-dynamics-motion-grammar.mjs`
  - `ok: true`
  - `23` checks, `0` failures
- `node visual/tools/check-dynamics-timeline-control.mjs`
  - `ok: true`
  - `14` checks, `0` failures
- `node visual/tools/check-timeline-motion-contract.mjs`
  - `ok: true`
  - `22` checks, `0` failures
- `node visual/tools/check-trajectory-path-view.mjs`
  - `ok: true`
  - `16` checks, `0` failures
- `node visual/tools/check-visual-command-api.mjs`
  - `ok: true`
- `node visual/tools/check-views.mjs`
  - `ok: true`
  - `80` checks, `0` failures
- `METRIC_VISUAL_EXAMPLES=dynamics-noise-hero node visual/tools/check-visual-regression-public-examples.mjs`
  - `ok: true`
  - `1` example, `0` failures
  - `dynamics-noise-hero` remains `heroAccepted: false`
- `node visual/tools/check-visual-regression-public-examples.mjs`
  - `ok: true`
  - `8` examples, `0` failures
- `node visual/tools/check-hero-visual-briefs.mjs`
  - `ok: true`
  - `dynamics-noise-hero` remains `review-pending`
  - blocker remains `visual-composition-not-human-accepted`
- `node visual/tools/check-hero-screenshot-review.mjs`
  - `ok: true`
  - accepted hero remains only `grae10-metric-engine`
  - `dynamics-noise-hero` remains `review-pending`
- `git diff --check -- visual/src/views/DynamicsView.js visual/src/views/TrajectoryPathView.js visual/examples/dynamics-noise-hero/index.html visual/tools/check-dynamics-composition.mjs docs/visual/reports/dynamics-noise-composition-acceptance.md`

One early `node visual/tools/check-hero-visual-briefs.mjs` run failed because
the existing regression artifact only contained a different scoped preview. A
full `node visual/tools/check-visual-regression-public-examples.mjs` run
refreshed the artifact, after which the brief gate passed.

## Remaining Status

The preview is not hero-accepted. The remaining blocker is intentionally:

```text
visual-composition-not-human-accepted
```

Status:

- loads: yes
- renders: yes
- interaction and GPU picking: yes
- accepted by automated gates: no, preview only
- manually accepted: no
- final preview status: review-pending
