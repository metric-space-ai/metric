# W8 Dynamics And Noise Visual Acceptance

Status date: 2026-06-26

## Status

Review-pending screenshot candidate, not manually accepted as a public hero.

The page renders finite dynamics/noise as exported metric-space movement over
the `reverse-reconstruction` timeline. It does not simulate dynamics,
transition costs, uncertainty, trajectory paths or timeline states in
JavaScript.

## Scope

- Example: `visual/examples/dynamics-noise-hero/index.html`
- Native evidence: `docs/examples/assets/dynamics-noise/metric.visual.json`
- Local URL used for frame capture: `http://127.0.0.1:58118`
- Frame URLs:
  - start: `http://127.0.0.1:58118/visual/examples/dynamics-noise-hero/index.html?flow=0&verify=1`
  - middle: `http://127.0.0.1:58118/visual/examples/dynamics-noise-hero/index.html?flow=0.5&verify=1`
  - end: `http://127.0.0.1:58118/visual/examples/dynamics-noise-hero/index.html?flow=1&verify=1`

The capture server was ephemeral and was closed after screenshot generation.

## Screenshot Evidence

- `visual/output/W8-dynamics-noise-hero-start.png` - 292142 bytes
- `visual/output/W8-dynamics-noise-hero-middle.png` - 252818 bytes
- `visual/output/W8-dynamics-noise-hero-end.png` - 252187 bytes

Browser capture probes for all three frames passed with no console errors or
page errors. The probes confirmed `CurveRibbonLayer`, `HeatFieldLayer` and
`InstancedPointLayer` descriptors, 512 field records, 512 trajectory paths and
non-algorithmic field/path metadata.

## Evidence Summary

- Native records: 512
- Native provenance: `native_export: true`, `synthetic_js: false`
- Timeline id: `reverse-reconstruction`
- Exported timeline states: 41
- Start coordinate: `coord-reverse-00`, step 0, reverse MSE 0.0153362
- Middle coordinate: `coord-reverse-20`, step 20, reverse MSE 0.0050846
- End coordinate: `coord-reverse-40`, step 40, reverse MSE 0.0103288
- Timeline field property: `reverse-mse-to-clean`
- Field source: exported `timeline_step` property samples
- Trajectory evidence: 512 paths through `TrajectoryPathView` from exported
  timeline coordinate states (`exported-timeline-states`)

## Visual Grammar

- Primary semantic view: `DynamicsView`
- Primary path primitive: `CurveRibbonLayer`
- Primary field primitive: `HeatFieldLayer`
- Supporting primitive: `InstancedPointLayer`
- Timeline UI: reusable `metric.visual.timeline_control.v1` scrubber
- Record preview: `preview: "state-history"` through the shared preview system

## Validation

Commands run from the task file:

```bash
node visual/tools/check-visual-document.mjs docs/examples/assets/dynamics-noise/metric.visual.json
node visual/tools/check-dynamics-motion-grammar.mjs
node visual/tools/check-dynamics-timeline-control.mjs
node visual/tools/check-timeline-motion-contract.mjs
node visual/tools/check-trajectory-path-view.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-grae10-golden.mjs
```

Results:

- visual document check: pass
- dynamics motion grammar: pass, 23 checks
- dynamics timeline control: pass, 14 checks
- timeline motion contract: pass, 22 checks
- trajectory path view: pass, 16 checks
- hero grammar contract: pass
- public gallery evidence: pass
- GRAE10 golden reference: pass, hash
  `464f6a90c36c1e9c6b4ec90068500dc226740d65b251918aca567f99d64d3d5e`
- public visual regression: pass, 8 examples, 0 failures, generated at
  `2026-06-26T10:11:20.603Z`

Supplemental validation:

```bash
METRIC_VISUAL_EXAMPLES=dynamics-noise-hero node visual/tools/check-visual-regression-public-examples.mjs
```

Result: pass, 1 example, 0 failures, generated at
`2026-06-26T10:10:16.415Z`. The scoped browser gate confirmed native evidence,
public-preview status, rendered canvas, pointer interaction, GPU point/field
picking and GPU curve picking for `dynamics-noise-hero`.

## Remaining Blockers

- Manual screenshot review has not accepted this as a public hero.

Status:

- loads: yes
- renders: yes
- interactive: yes
- visually accepted: no
- complete: no, review-pending
