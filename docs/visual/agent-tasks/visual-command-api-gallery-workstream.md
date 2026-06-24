# Agent Task: Visual Command API And Gallery Workstream

## Owner

One worker owns only:

- `visual/src/index.js`
- `visual/src/metric-visual.js`
- new command-level checks under `visual/tools/check-visual-command-*.mjs`
- public example shell simplifications under `visual/examples/*/index.html`
- project-page gallery wiring under `docs/site/index.html` only when the
  public evidence gate stays green

Do not edit WebGL layers, runtime internals, native C++ exporters, or GRAE10
reference code. Other workers may edit layers, views, interaction or tools in
parallel; do not revert their changes.

## Goal

Turn METRIC Visual into a small command-driven library surface. A user should
load `metric.visual.v1` evidence and call one semantic command without knowing
about descriptors, layer factories, runtime setup, matrix pickers or page DOM.

The examples and project gallery are acceptance fixtures for this API. They are
not allowed to own rendering architecture.

## Required Behavior

- Keep `createMetricVisual(options)` as the main entry point.
- Keep simple commands thin and semantic:
  - `showMetricSpace`
  - `showRelationMatrix`
  - `showNeighborhoodGraph`
  - `showSpaceProperties`
  - `showMapping`
  - `showDynamics`
  - `showConditionMonitoring`
  - `showMixedRecords`
  - `showCrossSpace`
  - `showRelationMatrixNeighborhood`
- Add command diagnostics that report:
  - evidence schema
  - record count
  - selected command/view kind
  - descriptor count
  - runtime layer count
  - whether the evidence is native, documented reference, or synthetic fixture
- Add one command-level check that fails if a public example bypasses
  `createMetricVisual` and semantic commands for rendering.
- Public examples may contain page layout, text and links. They must not build
  renderers, manually wire layer descriptors, or create a second render path.
- Project-page gallery entries may link only native evidence or documented
  reference assets. Synthetic fixtures must remain labeled as development
  fixtures and stay out of public hero claims.

## Hard Stop Rules

- Stop if an example needs custom WebGL, canvas drawing or manual descriptor
  construction to look acceptable. That means an engine/view/layer capability is
  missing and belongs outside this task.
- Stop if promoting a preview would require fake data or JavaScript-computed
  algorithm output.
- Do not change the protected GRAE10 60k reference except to verify that it
  still passes `check-grae10-golden.mjs`.

## Acceptance

Run:

```bash
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-grae10-golden.mjs
```

Browser-load the public gallery page and at least two command-backed examples.
Report the exact URLs, commands, and any files changed.
