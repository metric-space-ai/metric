# W7 Cross-Space Dependency Visual Acceptance

Status date: 2026-06-25

## Status

Review-pending screenshot candidate, not manually accepted as a public hero.

## Scope

- Example: `visual/examples/cross-space-dependency-hero/index.html`
- Native evidence:
  `docs/examples/assets/cross-space-dependency/metric.visual.json`
- Screenshot target: `visual/output/W7-cross-space-dependency-hero.png`
- Engine module touched: `visual/src/views/CrossSpaceView.js`
- Checks updated: `visual/tools/check-cross-space-linked-selection.mjs`,
  `visual/tools/check-linked-selection-presentation.mjs`

The page uses `createMetricVisual()` and `showCrossSpace()`. It does not
compute dependence, local contribution, bridge edges or paired neighborhoods in
JavaScript.

## Evidence Summary

- Native paired observations: 512
- Spaces: `event-log-space`, `process-curve-space`
- Coordinates: `event-log-landmark-3d`, `process-curve-landmark-3d`
- Relations: two native metric relations plus
  `cross-space-dependence-bridge-relation`
- Bridge graph: `cross-space-dependence-bridges`
- Local dependence property: `local-dependence-contribution`
- Global statistic: exported cross-space MGC diagnostic

## Visual Grammar

- Primary semantic view: `CrossSpaceView`
- Primary primitive: `RelationEdgeLayer` for dependence bridges
- Supporting primitives: two metric-space descriptor groups for the paired
  spaces
- Interaction: linked selection connects paired records and bridge evidence

## Validation

Commands run:

```bash
node visual/tools/check-cross-space-linked-selection.mjs
node visual/tools/check-linked-selection-presentation.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
```

Result:

- cross-space linked-selection check: pass
- linked-selection presentation check: pass
- public gallery evidence gate: pass
- GRAE10 golden reference: pass

The checks verified 512 paired observations, native bridge relation identity,
native bridge graph identity, source/target record channels and pair-selection
presentation.

## Remaining Blockers

- Manual screenshot review has not accepted this as a public hero.
- The visual composition still needs review for bridge legibility and whether
  the two spaces read as separate spaces at first glance.

Status:

- loads: yes by command/example checks
- renders: yes by descriptor checks
- interactive: linked record and pair selection checks pass
- visually accepted: no
- complete: no, review-pending
