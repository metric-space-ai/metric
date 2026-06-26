# MNIST Babyplots Standalone Quarantine

Status: ready to assign

## Owner Scope

- `visual/examples/mnist-dimension-reduction/index.html`
- docs references that directly mention `mnist-dimension-reduction`
- `docs/visual/reports/render-path-inventory-and-deletion-map.md`
- `docs/visual/metric-visual-progress.md`

Do not edit `visual/examples/grae10-metric-engine/index.html`. That page is the
protected 60k GRAE10 reference.

## Goal

Remove the embedded Babyplots/Babylon-style standalone MNIST render path from
the active visual examples tree. MNIST dimension reduction remains represented
by the protected METRIC-owned GRAE10 engine page, not by the old bundled
Babyplots HTML.

## Required Change

1. Delete or quarantine `visual/examples/mnist-dimension-reduction/index.html`
   so it is no longer an active visual example.
2. Remove public docs/site references to `mnist-dimension-reduction` if any
   still exist.
3. Do not replace it with another standalone renderer.
4. Do not create a placeholder that can be mistaken for a hero.
5. Update the render-path inventory and progress notes to say the Babyplots
   standalone path is gone and MNIST is represented by
   `visual/examples/grae10-metric-engine/index.html`.

## Non-Negotiables

- No Babyplots/Babylon bundle in active public visual examples.
- No `Baby`, `Baby.Plots`, `doRender()` or `bbp` render-loop artifacts in the
  active visual example path.
- Do not touch the protected GRAE10 page or its hash.
- Do not stage generated screenshots.

## Validation

Run at minimum:

```bash
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-direct-runtime-harness-quarantine.mjs
node visual/tools/check-grae10-golden.mjs
rg -n "Baby;|Baby\\.|doRender\\(|mnist-dimension-reduction|bbp\\." visual docs/site
```

The final `rg` should show only historical/report mentions if those are kept
for traceability. It must not show an active render page.

## Report

List changed paths and state whether the old file was deleted or moved to an
explicit non-public quarantine location.
