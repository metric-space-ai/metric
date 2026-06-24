# Agent Task: Mapping, Dynamics And Motion Grammar Workstream

## Owner

One worker owns only:

- `visual/src/timeline/*`
- `visual/src/curves/*`
- `visual/src/views/MappingView.js`
- `visual/src/views/DynamicsView.js`
- `visual/src/views/SolverTraceView.js`
- mapping/dynamics checks under `visual/tools/check-mapping-*.mjs`,
  `visual/tools/check-dynamics-*.mjs`, and `visual/tools/check-timeline-*.mjs`
- `visual/examples/mapping-dimensionality-hero/index.html` and
  `visual/examples/dynamics-noise-hero/index.html` only as thin acceptance
  fixtures

Do not edit command API, relation matrix code, glyph/property-field code,
native C++ exporters, project page, or GRAE10. Other workers may edit other
visual subsystems in parallel; do not revert their changes.

## Goal

Make coordinate changes, mappings, residuals, dynamics and solver traces
visible as time-aware metric-space evidence. Mapping/dimensionality and
dynamics/noise must not collapse into static colored point clouds.

## Required Behavior

- Mapping grammar:
  - consumes exported source/target coordinate states
  - supports 2D-to-3D or source-to-target morphs where record IDs match
  - renders residual/error vectors or projected distortion when exported
  - shows labels from exported label properties where available
- Dynamics grammar:
  - consumes exported timelines and trajectory/path evidence
  - renders paths, flow direction, propagation fields or state history
  - supports restrained animation that changes state without looping too fast
- Solver trace grammar:
  - consumes exported loss/residual/objective properties
  - renders them as timeline/state evidence, not as a separate charting library
- Motion must be an engine/timeline capability, not custom `setInterval` logic
  in a page.

## Hard Stop Rules

- Stop if mapping or dynamics examples are just one static point cloud.
- Stop if JavaScript computes PHATE, AE, diffusion, denoise or solver results.
- Stop if animation is page-local and not reusable through timeline/runtime
  contracts.

## Acceptance

Run:

```bash
node visual/tools/check-views.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
```

Browser-verify mapping and dynamics examples. Report:

- coordinate/timeline IDs consumed
- visible morph/path/residual layers
- animation timing behavior
- whether any required evidence is missing from native exports
