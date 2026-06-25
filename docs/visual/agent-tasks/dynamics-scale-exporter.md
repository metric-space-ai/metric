# Agent Task: Scale Finite Dynamics Native Evidence

## Owner Scope

You own only:

- `examples/engine/finite_metric_dynamics_visual_export.cpp`
- `examples/engine/finite_metric_dynamics_fixture.hpp` if present/needed
- `docs/examples/assets/dynamics-noise/metric.visual.json`
- optional report: `docs/visual/reports/dynamics-scale-exporter.md`

Do not edit visual runtime, project pages, GRAE10, unrelated exporters, CMake
unless the existing target is broken. You are not alone in the worktree: do not
revert changes made by other agents, and adapt to current files.

## Objective

Turn `dynamics-noise-hero` from 28 records into at least 500 deterministic
finite metric states with exported forward/reverse dynamics, propagation field,
transition evidence and state-history previews.

No JavaScript, Python or page code may compute dynamics, diffusion,
reconstruction, transition values or metrics.

## Required Inputs

Read:

- `docs/visual/visual-engine-masterplan.md`
- `docs/visual/metric-visual-engine-implementation-plan.md`
- `docs/visual/reports/native-hero-scale-evidence-upgrade-plan.md`
- `examples/engine/finite_metric_dynamics.cpp`
- `examples/engine/finite_metric_dynamics_visual_export.cpp`
- `visual/hero-visual-briefs.manifest.json`

## Required Behavior

- Export at least 500 records.
- Preserve `provenance.native_export: true` and keep `provenance.synthetic`
  absent or false.
- Grow the deterministic geometry source to a higher-resolution S-curve or
  multi-segment finite state set with deterministic noise.
- Keep all computations native C++:
  - Euclidean fixture metric or existing true finite metric
  - geometry-derived transition relation
  - transition graph
  - forward diffusion
  - reverse reconstruction
  - random-walk visit counts
  - stationary distribution
  - Dirichlet energy
  - MSE-to-clean and reconstruction errors
  - timeline coordinate states
- Export enough timeline states to prove forward diffusion and reverse flow
  without bloating the JSON with unnecessary intermediate frames.
- Every record must expose vector/state-history preview data.

## Acceptance Commands

```bash
cmake --build build/core --target engine_finite_metric_dynamics_visual_export -- -j4
ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure
node visual/tools/check-visual-document.mjs docs/examples/assets/dynamics-noise/metric.visual.json
node visual/tools/check-native-hero-evidence-scale.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
```

## Stop Rules

Stop and write the report if scaling forces browser-side dynamics, removes
state-history preview data, or creates JSON so large that the public regression
cannot load it.

## Report

Write `docs/visual/reports/dynamics-scale-exporter.md` with:

- changed files
- final record/relation/coordinate/timeline/property counts
- native dynamics computations and exported timeline policy
- commands run and results
- remaining blocker if visual composition still needs human screenshot review
