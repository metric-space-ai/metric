# W6 Mapping And Dimensionality Visual Acceptance

Status date: 2026-06-25

## Status

Review-pending screenshot candidate, not manually accepted as a public hero.

## Scope

- Example: `visual/examples/mapping-dimensionality-hero/index.html`
- Native evidence: `docs/examples/assets/mapping-dimensionality/metric.visual.json`
- Screenshots:
  - `visual/output/W6-mapping-dimensionality-hero-source.png`
  - `visual/output/W6-mapping-dimensionality-hero-transition.png`
  - `visual/output/W6-mapping-dimensionality-hero-target.png`
- Engine module touched: `visual/src/views/MappingView.js`
- Check added: `visual/tools/check-mapping-frame-screenshots.mjs`

The page uses exported source and target coordinate states plus exported
residual/error evidence. It does not compute mapping coordinates or residuals
in JavaScript.

## Evidence Summary

- Native records: 1,000 process-curve records
- Coordinate states: `source-coordinate-layout-3d`,
  `parametric-coordinate-latent-2d`
- Relations: source and latent metric relations
- Residual property: `local-mapping-distortion`
- Timeline/morph id: `mapping-coordinate-morph`
- Morph timing: source hold 1600 ms, quick transition 720 ms, target hold
  1700 ms

## Visual Grammar

- Primary semantic view: `MappingView`
- Primary primitives: `InstancedPointLayer`, `RelationEdgeLayer`
- Supporting primitives: `GroundPlaneLayer`, `GroundProjectionLayer`,
  `BillboardLabelLayer`
- Required visual behavior: source, transition and target frames are separate
  deterministic states, not one slow constant morph.

## Validation

Commands run:

```bash
node visual/tools/check-mapping-motion-grammar.mjs
node visual/tools/check-mapping-frame-screenshots.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
```

Result:

- mapping motion grammar check: pass
- source/transition/target screenshot capture check: pass
- public gallery evidence gate: pass
- GRAE10 golden reference: pass

The screenshot check opened the three deterministic frame URLs and verified
1,000 records, five runtime descriptors, residual vector layer presence and no
console/page errors.

## Remaining Blockers

- Manual screenshot review has not accepted this as a public hero.
- The current evidence is a native preview; further work should improve visual
  clarity of residual vectors before project-page promotion as an accepted hero.

Status:

- loads: yes
- renders: yes
- interactive: shared record preview path available, manual browser review still
  pending
- visually accepted: no
- complete: no, review-pending
