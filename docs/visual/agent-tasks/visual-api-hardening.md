# Agent Task: Visual API Hardening

## Owner

One worker owns only:

- `visual/src/metric-visual.js`
- `visual/tools/check-visual-document.mjs`
- optionally one new tool under `visual/tools/`

Do not edit C++ files, example pages, or docs except a short note if needed.

## Goal

Make the public JavaScript API harder to misuse. It should be obvious how to
load native evidence and call semantic visual commands without manually wiring
runtime internals.

## Required Behavior

- Keep `createMetricVisual(options)` as the main entry point.
- Add or improve command-level diagnostics for:
  - loaded schema
  - record count
  - selected view kind
  - layer descriptor count
  - runtime layer count
- If a caller passes synthetic evidence to a public-gallery-like context,
  surface a warning in diagnostics. Do not block engine development fixtures.
- Ensure `showRelationMatrix`, `showMetricSpace`, `showMapping`, `showDynamics`
  keep going through semantic views and `MetricVisualRuntime`.

## Constraints

- Do not change the protected GRAE10 page.
- Do not compute metrics or algorithm results in JavaScript.
- Do not create a new renderer path.

## Acceptance

Run:

```bash
node visual/tools/check-views.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-public-gallery-evidence.mjs
```

