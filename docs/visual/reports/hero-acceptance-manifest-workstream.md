# Hero Acceptance Manifest Workstream

Status date: 2026-06-25

## Scope

Added a checked-in hero acceptance manifest and wired the hero screenshot review
gate to read it. The gate still treats browser smoke/render/screenshot success
as review evidence only. It does not auto-promote public previews.

## Manifest Schema

Manifest path: `visual/hero-acceptance.manifest.json`

Schema version: `1`

Each accepted hero entry requires:

- `exampleId`: public example id from the browser regression report.
- `requiresNativeEvidence`: must be `true`.
- `expectedPrimaryVisualGrammar`: exact grammar label from the browser
  regression report.
- `screenshot`: generated report or screenshot contract, currently using
  `generatedReport`, `requiresPageScreenshot`, and
  `requiresCanvasScreenshot`.
- `humanAcceptance.status`: must be `accepted`.
- `humanAcceptance.date`: ISO date in `YYYY-MM-DD` form.
- `acceptanceReason`: short visual acceptance reason.

## Current Acceptance State

- Accepted hero count: `1`
- Accepted hero: `grae10-metric-engine`
- Review-pending preview count: `6`
- Review-pending previews:
  `condition-monitoring-hero`,
  `mixed-record-hero`,
  `cross-space-dependency-hero`,
  `relation-matrix-neighborhood`,
  `dynamics-noise-hero`,
  `mapping-dimensionality-hero`

No preview was promoted. GRAE10 remains accepted only through the manifest entry
plus the existing protected hash and native 60k dataset contract.

## Checks Run

- `node visual/tools/check-grae10-golden.mjs`: pass.
  GRAE10 hash:
  `464f6a90c36c1e9c6b4ec90068500dc226740d65b251918aca567f99d64d3d5e`
- `node visual/tools/check-public-gallery-evidence.mjs`: pass.
  Six public previews resolve native public assets and no public synthetic
  evidence was reported.
- `METRIC_VISUAL_OUT=/tmp/metric-visual-regression.ske9Vr node visual/tools/check-visual-regression-public-examples.mjs`:
  interrupted after the temporary browser run stopped making progress. It wrote
  only the GRAE10 page and canvas screenshots before hanging; no repository
  `output/` files were changed by this run.
- `METRIC_VISUAL_HERO_REVIEW_OUT=/tmp/metric-hero-review.* node visual/tools/check-hero-screenshot-review.mjs`:
  pass against the existing green browser regression report at
  `output/visual/check-visual-regression-public-examples/results.json`.
  The gate reported `acceptedHeroCount: 1`, `reviewPendingCount: 6`, and no
  manifest issues.
- `node --check visual/tools/check-hero-screenshot-review.mjs`: pass.

Parent integration rerun:

- `node visual/tools/check-visual-regression-public-examples.mjs`: pass,
  generated `2026-06-25T10:48:50.147Z`, 7 checked, 0 failed.
- `node visual/tools/check-hero-screenshot-review.mjs`: pass against
  `output/visual/check-visual-regression-public-examples/results.json`,
  generated `2026-06-25T10:49:26.608Z`, `acceptedHeroCount: 1`,
  accepted `grae10-metric-engine`,
  `reviewPendingCount: 6`, no manifest issues.

## Remaining Work

Before a second hero can be accepted, a reviewer must add an explicit manifest
entry with native evidence required, the expected primary visual grammar, a
screenshot or generated-report contract, human acceptance status and date, and a
short acceptance reason. The browser regression report must still be green and
the manifest contract must match its evidence, grammar, and screenshot fields.
