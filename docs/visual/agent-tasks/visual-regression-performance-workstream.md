# Agent Task: Visual Regression And Performance Workstream

## Owner

One worker owns only:

- new or updated checks under `visual/tools/check-visual-regression-*.mjs`
- new or updated checks under `visual/tools/check-visual-performance-*.mjs`
- reports under `docs/visual/reports/`
- optional generated local artifacts under `output/playwright/` or
  `output/visual/` that must not be committed unless explicitly documented as
  baselines

Do not edit runtime, layers, semantic views, examples, native C++ exporters,
project page, or GRAE10 reference code. Other workers may implement engine
features in parallel; do not revert their changes.

## Goal

Add objective gates that prevent future visual regressions from being called
done. Render success is not hero acceptance. The tools must distinguish:

- loads
- renders
- uses native evidence
- uses the intended grammar
- preserves protected GRAE10
- meets basic interaction contracts
- meets large-scene performance budgets

## Required Behavior

- Provide a browser-backed visual smoke harness for public examples.
- Capture current screenshots to `output/` for local review without committing
  transient files.
- Add a performance probe that records:
  - record count
  - descriptor count
  - runtime layer count
  - frame timing sample
  - GPU buffer or draw-count diagnostics when available
- Add a report that lists each public preview and whether it is:
  - hero-accepted
  - public preview only
  - synthetic development fixture
  - blocked by missing native evidence
- Keep `visual/regression-baselines/grae10-metric-engine.sha256` authoritative
  and do not regenerate it unless explicitly instructed by the parent agent.

## Hard Stop Rules

- Stop if a tool says a hero is accepted from a mere load/render check.
- Stop if screenshots or generated data would pollute the repo.
- Stop if Playwright/browser dependencies are unavailable; report that as a
  missing gate, not as a pass.

## Acceptance

Run:

```bash
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-single-render-pipeline.mjs
```

Then run the new regression/performance tools and write a concise report under
`docs/visual/reports/`.
