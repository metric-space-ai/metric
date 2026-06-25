# W4 Condition Monitoring Visual Acceptance

Status date: 2026-06-25

## Status

Review-pending screenshot candidate, not manually accepted as a public hero.

## Scope

- Example: `visual/examples/condition-monitoring-hero/index.html`
- Native evidence: `docs/examples/assets/condition-monitoring/metric.visual.json`
- Screenshot target: `visual/output/W4-condition-monitoring-hero.png`
- Engine modules touched: `visual/src/views/ProcessCurveSceneView.js`
- Check added: `visual/tools/check-condition-monitoring-visual-acceptance.mjs`

The page uses `createMetricVisual()`, `showConditionMonitoring()` and
`showProcessCurves()`. It does not load page-local `evidence.json`, does not
compute anomaly, density, regime or time-series values in JavaScript, and keeps
the result review-pending.

## Evidence Summary

- Native records: 528 process-window records
- Relations: `condition-monitoring-twed`,
  `condition-monitoring-transition`
- Graph: `process-window-trajectory`
- Coordinate state: `process-state-trajectory-3d`
- Field property used in the process scene: `metric-anomaly-severity`
- Label property: `truth-regime`
- Regimes present: `drift`, `fault`, `normal`, `recovery`, `signature`
- Preview payload family: exported `time_series` process-window payloads

## Visual Grammar

- Primary grammar: property field plus process trajectory
- Primary views: `PropertyFieldView`, `ProcessCurveSceneView`,
  `TrajectoryPathView`
- Primary primitives: `HeatFieldLayer`, `CurveRibbonLayer`,
  `BillboardLabelLayer`
- Supporting primitives: `GroundProjectionLayer`, `InstancedPointLayer`,
  `InstancedBoxLayer`

## Validation

Commands run:

```bash
node visual/tools/check-condition-monitoring-visual-acceptance.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
```

Result:

- condition visual acceptance check: pass
- public gallery evidence gate: pass
- GRAE10 golden reference: pass

The condition check verified 528 native records, exported anomaly field,
graph-backed trajectory, five 3D regime labels, and no page-local evidence.

## Remaining Blockers

- Manual screenshot review has not accepted this as a public hero.
- The native fixture is only slightly above the 500-window minimum, so a larger
  real process dataset would still be better before public hero promotion.

Status:

- loads: yes
- renders: yes by specialized descriptor checks
- interactive: shared record preview path available, manual browser review still
  pending
- visually accepted: no
- complete: no, review-pending
