# W4 Condition Monitoring Visual Acceptance

Status date: 2026-06-26

## Status

Review-pending acceptance candidate. There is no explicit manual acceptance
evidence, so this is not accepted as a public hero.

## Scope

- Example: `visual/examples/condition-monitoring-hero/index.html`
- Native evidence: `docs/examples/assets/condition-monitoring/metric.visual.json`
- Screenshot: `visual/output/W4-condition-monitoring-hero.png`
- Engine module touched: `visual/src/views/ProcessCurveSceneView.js`
- Condition checks: `visual/tools/check-condition-monitoring-visual-acceptance.mjs`, `visual/tools/check-condition-monitoring-browser.mjs`

The page uses `createMetricVisual()` and `showConditionMonitoring()`. It does
not load page-local `evidence.json`, does not compute anomaly, density, regime
or time-series evidence values in page JavaScript, and remains review-pending.

## Evidence Summary

- Local URL used: `http://127.0.0.1:8789/visual/examples/condition-monitoring-hero/index.html?verify=1`
- Native evidence path: `docs/examples/assets/condition-monitoring/metric.visual.json`
- Screenshot path: `visual/output/W4-condition-monitoring-hero.png`
- Native records: 528 process-window records
- Native relations: `condition-monitoring-twed`, `condition-monitoring-transition`
- Native graph: `process-window-trajectory`
- Target coordinate: `process-state-trajectory-3d`
- Field property: `metric-anomaly-severity`
- Label property: `truth-regime`
- Regimes present: `drift`, `fault`, `normal`, `recovery`, `signature`
- Visible regime labels: `healthy` (native `normal`), `drift`, `fault`, `recovery`, `signature`
- Preview payload family: exported `time_series` process-window payloads

## Visual Grammar

- Primary grammar: `PropertyFieldView` + `ProcessCurveSceneView` + shared record preview
- Primary primitives: `HeatFieldLayer`, `CurveRibbonLayer`, `BillboardLabelLayer`
- Supporting primitives: `GroundProjectionLayer`, `InstancedPointLayer`
- Field evidence: `PropertyFieldView` emits `metric-anomaly-severity` with `algorithmicComputation: false`
- Trajectory evidence: `TrajectoryPathView` emits graph-backed `process-window-trajectory`
- Time-series evidence: original process-window series are exposed through the shared preview panel
- Derived time-series geometry: disabled for this hero with `includeRecordSkyline: false`

## Browser Evidence

`METRIC_VISUAL_PORT=8789 node visual/tools/check-condition-monitoring-browser.mjs` verified:

- `metricConditionHero=ready` and `metricRecordCount=528`
- 528 preview-indexed process-window records from the process-state coordinate
- visible `healthy`, `drift`, `fault`, and `recovery` scene labels
- descriptor primitives are `HeatFieldLayer`, `GroundProjectionLayer`, `CurveRibbonLayer`, `InstancedPointLayer`, `BillboardLabelLayer`
- no derived record-skyline/time-series geometry (`derivedTimeSeriesLayerCount=0`)
- no debug UI selectors present
- hover target `window-0308` in regime `fault`
- hover preview visible with a time-series sparkline
- selection preview for `window-0308` resolves the original `time_series` payload
- screenshot written to `visual/output/W4-condition-monitoring-hero.png`

## Validation

Commands run:

```bash
node visual/tools/check-visual-document.mjs docs/examples/assets/condition-monitoring/metric.visual.json
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-condition-monitoring-visual-acceptance.mjs
METRIC_VISUAL_PORT=8789 node visual/tools/check-condition-monitoring-browser.mjs
```

Result: pass. `check-visual-regression-public-examples.mjs` reported `ok: true`
with 8 examples checked and 0 failures; `condition-monitoring-hero` remains
classified as `public-preview-only`.

## Remaining Blockers

- Manual screenshot review has not accepted this as a public hero.
- The view is intentionally an acceptance candidate only; promotion is still gated by review.
