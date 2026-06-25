# Agent Task: Record And Pair Preview Presentation Workstream

## Purpose

Make record and pair inspection a reusable METRIC Visual engine capability for
real exported payloads. This is not a hero page task and not a DOM workaround.

The preview panel must help a user inspect the original evidence behind a
rendered point, glyph, curve, matrix cell or graph edge without computing any
METRIC algorithm in JavaScript.

## Owner Scope

One worker owns only:

```text
visual/src/interaction/record-preview.js
visual/src/interaction/*
visual/tools/check-record-preview*.mjs
visual/tools/check-runtime-picking-preview.mjs
docs/visual/reports/preview-presentation-workstream.md
```

Do not edit renderer layers, relation-matrix grammar, glyph/field grammar,
mapping/dynamics grammar, native C++ exporters, project page, GRAE10, or public
example page scripts. Other workers may edit those areas in parallel.

## Required Behavior

- Record preview must support exported payload families:
  - strings/text
  - process curves or time-series snippets
  - histograms/distributions
  - image-like compact arrays
  - composed/mixed records
  - generic scalar/categorical/vector properties
- Pair preview must support:
  - row and column record identity
  - relation id/name/value
  - optional pair properties
  - symmetric native pair values where exported
- Layout must be bounded, readable and production-presentable:
  - no debug tables
  - no raw JSON dump as the primary view
  - no overflowing long payloads
  - compact summary first, details after
- Preview state must remain engine-level:
  - sourced from `metric.visual.v1`
  - fed by runtime inspection/selection state
  - independent of any hero page custom DOM
- Diagnostics must expose which payload renderer was used and whether content
  was truncated, summarized or unavailable.

## Stop Rules

- Stop if the implementation needs page-local hover handlers or per-example
  preview HTML.
- Stop if JavaScript computes metric values, nearest neighbors, entropy,
  dependence or mapping results to fill the preview.
- Stop if the preview requires a new render pipeline.
- Do not touch the protected GRAE10 visual reference.

## Acceptance

Run:

```bash
node visual/tools/check-record-preview-payloads.mjs
node visual/tools/check-runtime-picking-preview.mjs
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-views.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
```

Add a focused `check-record-preview-presentation*.mjs` if new presentation
diagnostics are introduced.

## Report

Write `docs/visual/reports/preview-presentation-workstream.md` with:

- files changed
- payload families covered
- pair-preview fields covered
- checks run
- remaining payload families that still need visual renderers
- explicit statement that public hero status did not change
