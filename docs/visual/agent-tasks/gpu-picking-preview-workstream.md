# Agent Task: GPU Picking And Preview Workstream

## Owner

One worker owns only:

- `visual/src/picking/*`
- picking-related sections of `visual/src/runtime/runtime.js`
- `visual/src/interaction/record-preview.js`
- new picking/preview checks under `visual/tools/check-*-picking*.mjs` or
  `visual/tools/check-*-preview*.mjs`
- optional short progress notes in `docs/visual/metric-visual-progress.md`

Do not edit relation matrix grammar, glyph/field grammar, mapping/dynamics
grammar, native C++ exporters, project page, or GRAE10. Other workers may edit
those subsystems in parallel; do not revert their changes.

## Goal

Make hover and click inspection an engine capability, not page-local behavior.
The engine must support record previews and pair previews over rendered METRIC
evidence, with GPU picking where available and a deterministic CPU fallback
where GPU picking is not wired yet.

## Required Behavior

- Record picking:
  - maps rendered pick IDs back to exported record IDs
  - exposes selected record through runtime state
  - keeps the existing CPU fallback working for views that do not yet provide
    pick buffers
- Pair picking:
  - maps selected relation cells or relation edges back to row/column record IDs
  - exposes selected pair through runtime state
  - can feed the existing preview panel with relation ID, value, row record and
    column record
- Preview rendering:
  - stays engine-level and reusable
  - can show record payload snippets without computing algorithm output in
    JavaScript
  - can show pair values and relation metadata from `metric.visual.v1`
- Diagnostics:
  - runtime state reports whether the current hover/selection came from GPU
    picking, relation matrix picking, graph picking or CPU fallback
  - tools can verify that selecting a record or pair changes runtime state

## Hard Stop Rules

- Stop if preview behavior requires custom DOM or event logic in a hero page.
- Stop if JavaScript computes metric values, nearest neighbors or derived
  algorithm results to make preview content appear.
- Stop if GPU picking requires a second render pipeline outside
  `MetricVisualRuntime`.
- Do not change the protected 60k GRAE10 reference.

## Acceptance

Run:

```bash
node visual/tools/check-views.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-visual-regression-public-examples.mjs
```

Add and run a focused picking/preview check if behavior changes are
implemented.

Report:

- which layers/views expose GPU picking
- which views still use CPU fallback
- exact runtime state keys for selected record and selected pair
- exact files changed
