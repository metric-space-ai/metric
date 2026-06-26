# Condition Monitoring Composition Acceptance

Date: 2026-06-26

## Changed Files

- `visual/src/views/ProcessCurveSceneView.js`
- `visual/src/views/TrajectoryPathView.js`
- `visual/examples/condition-monitoring-hero/index.html`
- `visual/tools/check-condition-monitoring-composition.mjs`
- `docs/visual/reports/condition-monitoring-composition-acceptance.md`

## Scope Result

Condition monitoring now stays on the reusable `ProcessCurveSceneView` grammar
and resolves these native evidence channels from
`docs/examples/assets/condition-monitoring/metric.visual.json`:

- dominant anomaly field: `metric-anomaly-severity`
- density support projection: `local-density`
- regime labels: `truth-regime`
- transition path: `process-window-trajectory` backed by
  `condition-monitoring-transition`

No synthetic records, synthetic padding, page-local renderer, JavaScript metric
computation, GRAE10 files, native C++ exporters, public hero manifests, or
other hero pages were changed.

## Implementation Notes

- `ProcessCurveSceneView` now separates primary scalar field selection from
  support scalar projection selection. Fuzzy refs such as `anomaly` can resolve
  to exported scalar properties, while `groundField: "local-density"` remains a
  separate support layer.
- `GroundProjectionLayer` is emitted through `GroundProjectionView` with
  `local-density` metadata and a quieter density-support material.
- `PropertyFieldView` is still the source of the anomaly field. The role is
  marked `dominant-anomaly-field` only when the exported property semantic is
  actually anomaly-like.
- `TrajectoryPathView` now accepts exported per-record property maps for path
  color and width. The condition-monitoring path keeps the exported graph
  trajectory and encodes `metric-anomaly-severity` as visual path evidence.
- Regime labels now anchor to group structure bounds and scene center rather
  than only to a raw centroid.
- The hero fixture only adjusts command options for `showConditionMonitoring`;
  it does not create descriptors or render layers locally.

## Checker Evidence

Added `visual/tools/check-condition-monitoring-composition.mjs`. It verifies:

- selected view kind is `condition-monitoring`
- descriptors include `HeatFieldLayer`, `GroundProjectionLayer`,
  `CurveRibbonLayer`, and `InstancedPointLayer`
- anomaly field uses exported `metric-anomaly-severity`
- density support uses exported `local-density`
- trajectory is backed by exported graph or transition evidence and is not a
  fallback
- screenshot and canvas screenshot files exist

## Validation

- `node --check visual/src/views/ProcessCurveSceneView.js`: passed.
- `node --check visual/src/views/TrajectoryPathView.js`: passed.
- `node --check visual/tools/check-condition-monitoring-composition.mjs`:
  passed.
- `node visual/tools/check-grae10-golden.mjs`: passed,
  `464f6a90c36c1e9c6b4ec90068500dc226740d65b251918aca567f99d64d3d5e`.
- `node visual/tools/check-visual-command-api.mjs`: passed.
- `node visual/tools/check-views.mjs`: passed, `total: 80`, `failed: 0`.
- `node visual/tools/check-hero-visual-briefs.mjs`: passed.
- `METRIC_VISUAL_EXAMPLES=condition-monitoring-hero node visual/tools/check-visual-regression-public-examples.mjs`:
  passed, `total: 1`, `failed: 0`, `heroAccepted: false`.
- `node visual/tools/check-visual-regression-public-examples.mjs`: passed,
  `total: 8`, `failed: 0`. This full run was needed before screenshot review
  because `check-hero-screenshot-review.mjs` requires protected GRAE10 in the
  regression report.
- `node visual/tools/check-hero-screenshot-review.mjs`: passed after the full
  regression report, with `grae10-metric-engine` accepted and
  `condition-monitoring-hero` still `review-pending`.
- `node visual/tools/check-condition-monitoring-composition.mjs`: passed.
- `node visual/tools/check-condition-monitoring-visual-acceptance.mjs`: passed.
- `node visual/tools/check-trajectory-path-view.mjs`: passed.

## Remaining Status

`condition-monitoring-hero` is not marked accepted. The visual brief still
keeps `visual-composition-not-human-accepted` until separate human screenshot
review clears it.
