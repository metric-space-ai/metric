# Agent Task: Direct Runtime Harness Quarantine

## Owner Scope

This task owns only:

- `visual/examples/native-engine-probe/index.html`
- `visual/src/index.js`
- `visual/src/miniature-field/index.js`
- `visual/src/miniature-field/instanced-box-field.js` if deletion or internal
  quarantine is required
- optionally one new checker under `visual/tools/`
- `docs/visual/reports/direct-runtime-harness-quarantine.md`

Do not edit GRAE10, process-curve example pages, `visual/src/metric-visual.js`,
`ProcessCurveSceneView`, generated evidence assets, native C++ exporters, or
project pages.

## Objective

Separate internal renderer probes from the public METRIC Visual API. Public
users should not discover standalone WebGL renderers or direct runtime harnesses
as if they were the data visualization library.

The public product surface is `createMetricVisual()` plus semantic `show*()`
commands. Internal probes may exist, but they must be clearly dev-only and must
not be exported as the normal library surface.

## Required Code Shape

1. Audit `native-engine-probe`. If it remains, mark it as an internal diagnostic
   harness in code-visible metadata and keep it out of public gallery gates.
   If it can be converted cheaply, convert it to `createMetricVisual()` while
   preserving its diagnostic purpose.
2. Remove `miniatureField` from the root public export in `visual/src/index.js`
   unless another checked public API depends on it.
3. If `InstancedBoxField` is unused outside its own module, either:
   - remove the public re-export and mark the module internal, or
   - delete it if all visual value is already covered by runtime layers.
4. Add a checker that documents which direct-runtime harnesses are allowed and
   fails if an accepted public example imports `MetricVisualRuntime`,
   `createLayerFromDescriptor`, or standalone renderer modules directly.
5. Do not weaken `MetricVisualRuntime`; it remains the one engine. The task is
   about public API boundaries, not deleting the engine.

## Forbidden

- Do not touch `visual/examples/grae10-metric-engine/`.
- Do not remove runtime/layer internals used by the actual engine path.
- Do not replace a runtime probe with a fake screenshot or static canvas.
- Do not add external dependencies or build tooling.
- Do not promote any preview to accepted hero status.

## Acceptance

Run and report:

```bash
node --check visual/src/index.js
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-runtime-picking-preview.mjs
node visual/tools/check-views.mjs
node visual/tools/check-grae10-golden.mjs
rg -n "miniatureField|InstancedBoxField|createInstancedBoxField" visual/src/index.js visual/examples docs/site
rg -n "new MetricVisualRuntime|createLayerFromDescriptor" visual/examples docs/site
```

The final direct-runtime `rg` may still show protected GRAE10 and explicitly
dev-only probes. It must not show accepted public examples bypassing the public
command API.

## Report

Write `docs/visual/reports/direct-runtime-harness-quarantine.md` with:

- which direct-runtime/probe paths remain and why
- which exports were removed or marked internal
- validation commands and results
- next cleanup needed, if any
