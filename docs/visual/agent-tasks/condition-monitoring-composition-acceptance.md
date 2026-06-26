# Condition Monitoring Composition Acceptance

## Purpose

Move the native condition-monitoring preview closer to accepted-hero quality by
improving the reusable engine grammar for anomaly/density fields, trajectory
paths, labels, previews and camera composition.

This task is not allowed to fake additional data or hand-build a one-off page.
The page is only an acceptance fixture for the engine view.

## Owner Scope

Primary write scope:

- `visual/src/views/ProcessCurveSceneView.js`
- `visual/examples/condition-monitoring-hero/index.html`
- optional checker: `visual/tools/check-condition-monitoring-composition.mjs`
- report: `docs/visual/reports/condition-monitoring-composition-acceptance.md`

Allowed supporting files only if necessary:

- `visual/src/views/TrajectoryPathView.js`
- `visual/src/views/PropertyFieldView.js`
- `visual/src/layers/HeatFieldLayer.js`
- `visual/src/layers/GroundProjectionLayer.js`

Do not edit:

- `visual/examples/grae10-metric-engine/**`
- `visual/regression-baselines/grae10-metric-engine.sha256`
- native C++ exporters
- other public hero pages
- `visual/hero-acceptance.manifest.json`

## Current Evidence

Use only:

```text
docs/examples/assets/condition-monitoring/metric.visual.json
```

The native evidence currently has 528 records and passes scale. The open blocker
is visual composition, not record count.

Current screenshot reference:

```text
output/visual/check-visual-regression-public-examples/condition-monitoring-hero.canvas.png
```

Current issue: the visual is stylish, but the density/anomaly field and process
trajectory do not yet read strongly enough as a condition-monitoring proof. The
scene can look like decorative path art rather than a finite-metric-space
analysis.

## Required Outcome

Improve the engine/view behavior so the screenshot shows:

- a bounded, grounded miniature scene with clear perspective;
- anomaly severity visible as the dominant scalar field;
- density/entropy support visible without becoming background noise;
- trajectory/path evidence that reads as process evolution, not random lines;
- regime labels that attach to scene structure and do not occlude the evidence;
- hover/preview behavior still resolving original time-series payloads;
- no page-local rendering helper, no synthetic padding, no JavaScript metric
  computation.

Prefer changes in `ProcessCurveSceneView` and reusable layer options over
per-page CSS or ad hoc descriptor patches.

## Acceptance

Run:

```bash
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-views.mjs
node visual/tools/check-hero-visual-briefs.mjs
METRIC_VISUAL_EXAMPLES=condition-monitoring-hero node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-hero-screenshot-review.mjs
```

If a custom checker is added, it must prove at least:

- selected view kind is `condition-monitoring`;
- descriptors include `HeatFieldLayer`, `GroundProjectionLayer`,
  `CurveRibbonLayer` and `InstancedPointLayer`;
- field descriptor uses an exported anomaly/density property id, not an
  arbitrary scalar fallback;
- trajectory descriptor is backed by exported graph/relation/timeline evidence
  or explicitly marked as non-evidence fallback;
- screenshot and canvas screenshot exist.

Do not mark the hero accepted. Leave `visual-composition-not-human-accepted`
until a separate human review clears it.
