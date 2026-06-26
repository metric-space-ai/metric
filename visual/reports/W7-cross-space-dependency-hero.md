# W7 Cross-Space Dependency Visual Acceptance

Status date: 2026-06-26

## Status

Review-pending screenshot candidate, not manually accepted as a public hero.

## Scope

- Example: `visual/examples/cross-space-dependency-hero/index.html`
- Native evidence:
  `docs/examples/assets/cross-space-dependency/metric.visual.json`
- Screenshot target: `visual/output/W7-cross-space-dependency-hero.png`
- Check updated: `visual/tools/check-cross-space-linked-selection.mjs`

The page uses `createMetricVisual()` and `showCrossSpace()` against the native
metric visual export. It does not compute dependence, local contribution, bridge
edges or paired neighborhoods in JavaScript.

## Review Capture

- Local URL used:
  `http://127.0.0.1:58140/visual/examples/cross-space-dependency-hero/index.html?acceptance=1`
- Screenshot:
  `visual/output/W7-cross-space-dependency-hero.png`
- Native evidence path:
  `docs/examples/assets/cross-space-dependency/metric.visual.json`
- Paired records: 512
- Selected pair: `obs-000 <-> obs-000`
- Selected local contribution value: `1.159109`
- Runtime selection presentation: 1 paired bridge and 2 paired-space record
  features; shared pair preview visible.

## Evidence Summary

- Spaces: `event-log-space`, `process-curve-space`
- Coordinates: `event-log-landmark-3d`, `process-curve-landmark-3d`
- Relations: two native metric relations plus
  `cross-space-dependence-bridge-relation`
- Bridge graph: `cross-space-dependence-bridges`
- Local dependence property: `local-dependence-contribution`
- Global statistic: exported cross-space MGC diagnostic
  `0.955685202924308`
- Native bridge contributors: 96 selected bridge contributors out of 512
  candidate pairs

## Visual Grammar

- Primary semantic view: `CrossSpaceView`
- Primary layout: two offset finite metric spaces, with visible source/target
  bands and separate metric labels
- Primary primitive: `RelationEdgeLayer` for exported dependence bridges
- Supporting primitives: paired metric-space descriptor groups with exported
  local contribution color
- Interaction: linked selection connects paired records and bridge evidence
  through the shared preview system

## Validation

Commands run:

```bash
node visual/tools/check-visual-document.mjs docs/examples/assets/cross-space-dependency/metric.visual.json
node visual/tools/check-cross-space-linked-selection.mjs
node visual/tools/check-linked-selection-presentation.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-grae10-golden.mjs
```

Results:

- `check-visual-document`: pass; native schema-valid export with 512 records, 2
  spaces, 3 relations, 2 coordinates and 6 properties.
- `check-cross-space-linked-selection`: pass after updating the mock surface to
  support the current `_setLayerDescriptors()` surface method.
- `check-linked-selection-presentation`: pass.
- `check-hero-grammar-contract`: pass.
- `check-public-gallery-evidence`: pass; GRAE10 hash unchanged and public
  preview examples load native assets.
- `check-visual-regression-public-examples`: pass in the main worktree; 8
  examples, 0 failures.
- `check-grae10-golden`: pass; hash
  `464f6a90c36c1e9c6b4ec90068500dc226740d65b251918aca567f99d64d3d5e`.

Additional browser capture:

- A separate headless Chromium capture wrote
  `visual/output/W7-cross-space-dependency-hero.png`.
- It verified `metricCrossSpaceHero=ready`, 512 records, selected pair
  `obs-000:obs-000`, local contribution `1.159109`, 4 runtime layers, visible
  shared preview, 1 paired bridge and 2 paired record features.

## Remaining Blockers

- Manual screenshot review has not accepted the composition as a public hero.

Status:

- loads: yes by document and browser capture checks
- renders: yes by browser screenshot capture
- interactive: linked record and pair selection checks pass
- visually accepted: no
- complete: no, review-pending
