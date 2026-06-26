# W6 Mapping And Dimensionality Visual Acceptance

Status date: 2026-06-26

## Status

Review-pending screenshot candidate, not manually accepted as a public hero.

## Scope

- Example: `visual/examples/mapping-dimensionality-hero/index.html`
- Native evidence: `docs/examples/assets/mapping-dimensionality/metric.visual.json`
- Screenshots:
  - `visual/output/W6-mapping-dimensionality-hero-source.png`
  - `visual/output/W6-mapping-dimensionality-hero-transition.png`
  - `visual/output/W6-mapping-dimensionality-hero-target.png`
- Local screenshot URL base: `http://127.0.0.1:58408`
- Engine modules touched in this pass: none
- Mapping check used for screenshots: `visual/tools/check-mapping-frame-screenshots.mjs`

The page uses exported source and target coordinate states plus exported
residual/error evidence. It does not compute mapping coordinates or residuals
in JavaScript.

## Evidence Summary

- Native records: 1,000 process-curve records
- Coordinate states: `source-coordinate-layout-3d`,
  `parametric-coordinate-latent-2d`
- Relations: source and latent metric relations
- Residual property: `local-mapping-distortion`
- Residual channel name: `residual`
- Timeline/morph id: `mapping-coordinate-morph`
- Morph timing: source hold 1600 ms, quick transition 720 ms, target hold
  1700 ms, reset hold 360 ms, total 5200 ms

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
node visual/tools/check-visual-document.mjs docs/examples/assets/mapping-dimensionality/metric.visual.json
node visual/tools/check-mapping-motion-grammar.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-grae10-golden.mjs
```

Result:

- visual document check: pass (`metric.visual.v1`, 1,000 records, 3
  coordinates, 9 properties, native export)
- mapping motion grammar check: pass
- hero grammar contract: pass
- public gallery evidence gate: pass
- public examples regression gate: pass in the main worktree; 8 examples,
  0 failures
- GRAE10 golden reference: pass

Additional screenshot capture:

```bash
node visual/tools/check-mapping-frame-screenshots.mjs
```

Result: pass. The check opened the three deterministic frame URLs at
`http://127.0.0.1:58408`, verified 1,000 records, five runtime descriptors,
residual vector layer presence, morph progress `0.000`, `0.500`, `1.000`, and
no console/page errors.

## Remaining Blockers

- Manual screenshot review has not accepted this as a public hero.
- The native evidence remains a review-pending preview: its provenance does not
  mark it as public-hero-ready.

Status:

- loads: yes
- renders: yes
- interactive: yes; hover/selection exposes local mapping evidence including
  `local-mapping-distortion`, linked source/target coordinates and record
  preview data
- visually accepted: no
- complete: no, review-pending
