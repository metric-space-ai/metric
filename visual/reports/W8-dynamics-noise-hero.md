# W8 Dynamics And Noise Visual Acceptance

Status date: 2026-06-25

## Status

Review-pending screenshot candidate, not manually accepted as a public hero.

## Scope

- Example: `visual/examples/dynamics-noise-hero/index.html`
- Native evidence: `docs/examples/assets/dynamics-noise/metric.visual.json`
- Screenshot targets:
  - `visual/output/W8-dynamics-noise-hero-start.png`
  - `visual/output/W8-dynamics-noise-hero-middle.png`
  - `visual/output/W8-dynamics-noise-hero-end.png`
- Checks updated: `visual/tools/check-dynamics-motion-grammar.mjs`,
  `visual/tools/check-dynamics-timeline-control.mjs`,
  `visual/tools/check-timeline-control-widget.mjs`

The page uses `showDynamics()` over exported timeline states and trajectory
evidence. It does not simulate dynamics, transition cost, uncertainty or
timeline states in JavaScript.

## Evidence Summary

- Native records: 512
- Timeline id: `reverse-reconstruction`
- Exported timeline states: 41
- Start/middle/end coordinates: `coord-reverse-00`, `coord-reverse-20`,
  `coord-reverse-40`
- Timeline field property: `reverse-mse-to-clean`
- Active field property after scrub: `best-reconstruction-error`
- Trajectory paths: 512

## Visual Grammar

- Primary semantic view: `DynamicsView`
- Primary path primitive: `CurveRibbonLayer`
- Primary field primitive: `HeatFieldLayer`
- Supporting primitive: `InstancedPointLayer`
- UI: reusable timeline-control widget attached to the visual surface

## Validation

Commands run:

```bash
node visual/tools/check-dynamics-motion-grammar.mjs
node visual/tools/check-dynamics-timeline-control.mjs
node visual/tools/check-timeline-control-widget.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
```

Result:

- dynamics motion grammar check: pass
- dynamics timeline-control check: pass
- timeline widget check: pass
- public gallery evidence gate: pass
- GRAE10 golden reference: pass

The checks verified 512 records, 41 exported states, 512 trajectory paths,
state-changing field samples and timeline-widget scrubbing over exported
evidence.

## Remaining Blockers

- Manual screenshot review has not accepted this as a public hero.
- Start, middle and end screenshots still need human review for composition and
  legibility.

Status:

- loads: yes by command/example checks
- renders: yes by descriptor checks
- interactive: timeline-widget checks pass
- visually accepted: no
- complete: no, review-pending
