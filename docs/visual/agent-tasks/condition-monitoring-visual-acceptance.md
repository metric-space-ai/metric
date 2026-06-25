# Agent Task: Condition Monitoring Visual Acceptance

## Owner

One worker owns only:

- `visual/examples/condition-monitoring-hero/index.html`
- `visual/reports/W4-condition-monitoring-hero.md`
- `visual/output/W4-condition-monitoring-hero.png`
- condition-specific browser checks under
  `visual/tools/check-condition-monitoring-*.mjs`
- only if strictly required by this example:
  `visual/src/views/ProcessCurveSceneView.js` and
  `visual/src/views/PropertyFieldView.js`

Do not edit GRAE10, native exporters, command API, project page, mixed-record,
cross-space, mapping, dynamics or relation-matrix files. Other agents may be
working in parallel; do not revert their changes.

## Goal

Turn the native condition-monitoring preview into a screenshot-reviewable visual
acceptance candidate for the engine, without promoting it to an accepted hero.

The point is to prove that METRIC Visual can render process-state evidence with
a property field, regime labels, trajectory context and original record preview
through the reusable engine.

## Visual Claim

Windowed process records form a finite metric space whose anomaly, density and
regime structure can be inspected as a process-state scene. The viewer should
see normal, drift/fault and recovery structure without reading a side panel.

## Input Evidence

Use only:

```text
docs/examples/assets/condition-monitoring/metric.visual.json
```

Do not add `visual/examples/condition-monitoring-hero/evidence.json`. Do not
compute anomaly, density, regime or time-series values in JavaScript.

## Required Engine Grammar

Primary:

- `PropertyFieldView`
- `ProcessCurveSceneView`
- record preview from exported payloads

Supporting:

- metric-space points only as spatial context
- labels that belong to the 3D scene
- ground projection or field with semantic value

## Acceptance

- The scene is not a generic colored point cloud.
- Healthy/drift/fault/recovery regions are visible.
- The floor or field carries exported anomaly, density or regime evidence.
- Hover/selection exposes original process-window payloads through the shared
  preview system.
- The screenshot can be reviewed without debug UI.
- The report states whether this is still review-pending or manually accepted.

## Validation

Run:

```bash
node visual/tools/check-visual-document.mjs docs/examples/assets/condition-monitoring/metric.visual.json
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-grae10-golden.mjs
```

The report must include the local URL used, the screenshot path, native evidence
path, primary grammar, record count, and any remaining blocker.
