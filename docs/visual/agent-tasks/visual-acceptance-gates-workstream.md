# Agent Task: Visual Acceptance Gates Workstream

## Purpose

Make the browser gates harder to misuse. A gate may prove that a scene loads,
renders, uses native evidence, exposes the intended grammar, and stays within
basic performance budgets. It must not call a preview hero-ready.

## Owner Scope

One worker owns only:

```text
visual/tools/check-visual-regression-*.mjs
visual/tools/check-visual-performance-*.mjs
visual/tools/check-hero-screenshot-review.mjs
docs/visual/reports/*
```

Do not edit runtime, layers, semantic views, examples, native C++ exporters,
project page, GRAE10, or generated screenshots/baselines. Other workers may
change engine behavior in parallel; adapt checks to the current public API
without reverting their work.

## Required Behavior

- Public regression must continue to distinguish:
  - accepted hero
  - public preview only
  - synthetic development fixture
  - blocked/missing native evidence
- Screenshot review must stay separate from load/render smoke.
- Performance report must include:
  - record count
  - descriptor count
  - runtime layer count
  - frame timing
  - GPU draw/buffer diagnostics
  - per-layer diagnostics when available
- Grammar rows must assert that each preview uses the intended primary visual
  grammar and not just a generic point cloud.
- Generated output must stay under `output/` and must not be committed.

## Stop Rules

- Stop if any check labels a preview as hero-accepted because it loaded or
  rendered.
- Stop if a screenshot baseline for GRAE10 would need regeneration.
- Stop if Playwright/browser dependencies are unavailable; report missing gate
  rather than passing.
- Do not edit project-page content.

## Acceptance

Run:

```bash
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-visual-performance-large-scenes.mjs
node visual/tools/check-hero-screenshot-review.mjs
```

## Report

Write or update a concise report under `docs/visual/reports/` with:

- exact commands run
- generated report timestamps
- accepted hero count
- review-pending preview count
- per-preview grammar status
- explicit statement that generated `output/` files were not committed
