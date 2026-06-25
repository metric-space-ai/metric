# Agent Task: Process-Curve Descriptor Factory Quarantine

## Owner Scope

This task owns only:

- `visual/src/views/ProcessCurveSceneView.js`
- `visual/src/views/index.js`
- `visual/examples/miniature-hero-frame/index.html`
- `visual/examples/miniature-look-gallery/index.html`
- optionally one new checker under `visual/tools/`
- `docs/visual/reports/process-curve-descriptor-factory-quarantine.md`

Do not edit `visual/src/metric-visual.js`; the command-surface wiring belongs
to another task. Do not edit GRAE10, generated evidence assets, C++ exporters,
project pages, or public hero manifests.

## Objective

Stop public and example pages from reaching directly into
`createProcessCurveMiniatureLayerDescriptors()`. Process-curve rendering must
be expressed as a semantic `ProcessCurveSceneView` or through
`createMetricVisual().showProcessCurves(...)`.

The descriptor factory may remain only as private implementation detail behind
`ProcessCurveSceneView.toLayerDescriptors()` or as an explicitly dev-only
compatibility helper with a checker that prevents public/example use.

## Required Code Shape

1. Convert `visual/examples/miniature-hero-frame/index.html` away from direct
   descriptor factory calls. Prefer a thin public-command shell:
   `createMetricVisual(...).showProcessCurves(...)`.
2. Convert `visual/examples/miniature-look-gallery/index.html` so it does not
   construct process-curve descriptors directly from a page. If the look atlas
   still needs descriptor bundles, create them from `ProcessCurveSceneView`
   rather than from the old factory export.
3. Keep any descriptor assembly helper private to
   `visual/src/views/ProcessCurveSceneView.js` unless a deliberate dev-only
   export remains with a warning and a gate.
4. Add a checker that fails if `visual/examples/*` imports or calls
   `createProcessCurveMiniatureLayerDescriptors`.
5. Keep `ProcessCurveSceneView` as the semantic owner of:
   - field
   - ground projection
   - record-order trajectory
   - skyline/record-volume glyphs
   - morph layer
   - optional neighborhood graph
   - optional relation matrix

## Forbidden

- Do not touch `visual/examples/grae10-metric-engine/`.
- Do not create another descriptor helper with a new name in an example page.
- Do not add npm, CDN, build tooling or external dependencies.
- Do not use old `metric.evidence.v1` as the evidence source for these pages.
- Do not promote preview examples to accepted heroes.

## Acceptance

Run and report:

```bash
node --check visual/src/views/ProcessCurveSceneView.js
node visual/tools/check-process-curve-scene-view.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-views.mjs
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-grae10-golden.mjs
rg -n "createProcessCurveMiniatureLayerDescriptors" visual/examples visual/src/views/index.js visual/src/index.js
```

The final `rg` may only show an explicitly dev-only/internal export if the
report explains why it cannot yet be removed. It must not show public/example
page calls.

## Report

Write `docs/visual/reports/process-curve-descriptor-factory-quarantine.md`
with:

- old example call sites removed or quarantined
- remaining internal helper/export status
- validation commands and results
- remaining direct-runtime pages left for the direct-runtime harness task
