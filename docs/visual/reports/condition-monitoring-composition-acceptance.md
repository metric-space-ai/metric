# Condition Monitoring Composition Acceptance

Date: 2026-06-26

## Changed Files

- `visual/src/views/TrajectoryPathView.js`
- `visual/src/views/PropertyFieldView.js`
- `visual/src/views/ProcessCurveSceneView.js`
- `visual/tools/check-trajectory-path-view.mjs`
- `visual/tools/check-condition-monitoring-composition.mjs`
- `docs/visual/reports/condition-monitoring-composition-acceptance.md`

No condition-monitoring page HTML, synthetic evidence, native exporter,
GRAE10 file, public hero manifest, or project-site file was changed.

## Scope Result

Condition monitoring still resolves only native evidence from:

```text
docs/examples/assets/condition-monitoring/metric.visual.json
```

The reusable view descriptors now expose
`metric.visual.layer_hierarchy.v1` metadata for the hierarchy that matters to
this preview:

- anomaly field: `support-field`, below trajectory, current state and labels
- density projection: `ground-projection`, below trajectory, current state and
  labels
- process path: `trajectory-path`, above fields/projections and below current
  state/labels
- current records: `current-state`, above fields/projections/path and below
  labels
- regime labels: `scene-labels`, above the scene evidence

The trajectory remains backed by `process-window-trajectory` and
`condition-monitoring-transition`; `metric-anomaly-severity`, `local-density`
and `truth-regime` remain exported evidence inputs. The hierarchy contract is
descriptor metadata only and reports `algorithmicComputation: false`.

## Checker Evidence

`visual/tools/check-condition-monitoring-composition.mjs` now verifies the
hierarchy contract in addition to the existing grammar checks. Current browser
regression artifacts emit canvas screenshots; the full-page screenshot file is
treated as optional when absent, while the canvas screenshot remains required.

## Validation

- `node --check visual/src/views/TrajectoryPathView.js`: passed.
- `node --check visual/src/views/PropertyFieldView.js`: passed.
- `node --check visual/src/views/ProcessCurveSceneView.js`: passed.
- `node --check visual/tools/check-condition-monitoring-composition.mjs`:
  passed.
- `node visual/tools/check-condition-monitoring-composition.mjs`: passed,
  `ok: true`, `canvasScreenshotAvailable: true`,
  `pageScreenshotAvailable: false`.
- `node visual/tools/check-trajectory-path-view.mjs`: passed, `total: 19`,
  `failed: 0`.
- `node visual/tools/check-hero-visual-briefs.mjs`: passed,
  `condition-monitoring-hero` roles include `dominant-anomaly-field`,
  `density-support-projection`, `trajectory/path`, `current-state-records` and
  `region-labels`.
- `node visual/tools/check-public-gallery-evidence.mjs`: passed,
  `ok: true`, public condition evidence points to
  `../../../docs/examples/assets/condition-monitoring/metric.visual.json`.

## Remaining Status

`condition-monitoring-hero` remains review-pending. The blocker is still:

```text
visual-composition-not-human-accepted
```
