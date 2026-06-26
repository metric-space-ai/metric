# Descriptor Injection Hardening

Status: integrated

## Owner Scope

- `visual/src/metric-visual.js`
- `visual/tools/check-visual-command-api.mjs`
- `visual/tools/check-single-render-pipeline.mjs`
- `docs/visual/reports/render-path-inventory-and-deletion-map.md`
- `docs/visual/metric-visual-progress.md`

Do not edit public example pages, runtime layer implementations, native C++ exporters,
or project-site content in this task.

## Goal

Remove raw layer-descriptor injection from the public `MetricVisualSurface`
command surface. Public users should reach the renderer through semantic
commands and semantic view objects, not through `setLayerDescriptors()` or
`addLayerDescriptors()`.

`MetricVisualRuntime.setLayerDescriptors()` remains an internal runtime method.
The task is about the public facade in `visual/src/metric-visual.js`.

## Required Change

1. Replace public `MetricVisualSurface.setLayerDescriptors()` and
   `MetricVisualSurface.addLayerDescriptors()` with a private/internal helper,
   for example `_setLayerDescriptors()`.
2. Update all semantic commands inside `MetricVisualSurface` to use the private
   helper.
3. Keep `setViews()` public only for semantic view objects. It must accept
   objects with `toLayerDescriptors()`. It must not silently accept raw
   descriptor objects as public input.
4. If an internal diagnostic path still needs raw descriptor injection, gate it
   behind an explicit internal option name and keep that out of public examples.
5. Extend checks so public examples cannot call descriptor injection methods.

## Non-Negotiables

- Do not create a second render pipeline.
- Do not add page-local descriptor builders.
- Do not weaken any GRAE10 golden/reference gate.
- Do not touch unrelated dirty C++/Python files.

## Validation

Run at minimum:

```bash
node --check visual/src/metric-visual.js
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
rg -n "setLayerDescriptors\\(|addLayerDescriptors\\(" visual/src visual/examples docs/site --glob '!visual/src/runtime/runtime.js' --glob '!visual/src/runtime/metric-webgl/**'
```

The final `rg` may show the private facade helper and internal runtime method,
but it must not show public example or project-site calls.

## Report

Update `docs/visual/reports/render-path-inventory-and-deletion-map.md` and
`docs/visual/metric-visual-progress.md` with the exact API boundary after the
change.
