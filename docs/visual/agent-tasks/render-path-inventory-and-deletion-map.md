# Agent Task: Render Path Inventory And Deletion Map

## Owner Scope

This is a read-only explorer task. It owns only:

- `docs/visual/reports/render-path-inventory-and-deletion-map.md`

Do not edit source code, examples, generated assets, GRAE10, project pages or
tests. Other workers may change code while this report is being written; report
current findings and note uncertain items.

## Objective

Identify every remaining visual render path that bypasses the intended Visual
Engine architecture:

```text
metric.visual.v1 evidence -> semantic view -> layer descriptors -> MetricVisualRuntime
```

The report must classify each finding as one of:

- keep: canonical runtime or semantic view
- wrap: usable logic that must move behind a semantic view
- delete: legacy demo/rendering path no longer acceptable
- quarantine: unclear or external fixture code that must not be used by public
  heroes

## Required Searches

Inspect at least:

```bash
rg -n "new MetricVisualRuntime|new MetricScene3D|CanvasRenderingContext2D|create.*Descriptor|LayerDescriptor|setLayerDescriptors|this.views = \\[\\]|this.views = \\[\\]|function .*Descriptor|class Metric" visual/src visual/examples docs/site
rg -n "createProcessCurveMiniatureLayerDescriptors|pairedRecordBridgeDescriptor|sharedGroundDescriptor|offsetPositionMap|MetricScene3D|MetricRecordGallery|MetricHeatmap|MetricQueryInspector|loadMetricEvidence" visual/src/metric-visual.js visual/src/views visual/examples docs/site
```

Also inspect:

- `visual/src/metric-visual.js`
- `visual/src/views/`
- `visual/examples/*/index.html`
- `docs/site/index.html`

## Required Report Format

For each finding include:

- file path and line number
- symbol/function/class
- current role
- classification: keep/wrap/delete/quarantine
- why it violates or satisfies the one-engine rule
- exact next action
- safest owner scope for a future worker
- validation command that would prove the fix

The report must include a prioritized deletion/extraction order. Put the
highest-risk `metric-visual.js` legacy exports first.

## Forbidden

- Do not edit code.
- Do not mark a legacy path safe just because tests currently pass.
- Do not propose a second renderer.
- Do not propose page-local canvas/SVG fallbacks.
- Do not touch GRAE10.

## Acceptance

The report must let the parent agent schedule the next three non-overlapping
engine cleanup tasks without another exploratory pass.

