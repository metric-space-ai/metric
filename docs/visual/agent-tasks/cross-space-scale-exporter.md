# Agent Task: Scale Cross-Space Dependency Native Evidence

## Owner Scope

You own only:

- `examples/engine/cross_space_dependency_visual_export.cpp`
- `examples/engine/cross_space_dependency.hpp` only if the existing generator
  needs a narrow scale parameter or bridge evidence helper
- `docs/examples/assets/cross-space-dependency/metric.visual.json`
- optional report: `docs/visual/reports/cross-space-scale-exporter.md`

Do not edit visual runtime, project pages, GRAE10, unrelated exporters, CMake
unless the existing target is broken. You are not alone in the worktree: do not
revert changes made by other agents, and adapt to current files.

## Objective

Turn `cross-space-dependency-hero` from 48 paired records into at least 500
paired observations with two native metric spaces, dependence diagnostics and
visible bridge evidence.

No JavaScript, Python or page code may compute MGC, correlations, distance
matrices, local contributions or bridge selection.

## Required Inputs

Read:

- `docs/visual/visual-engine-masterplan.md`
- `docs/visual/metric-visual-engine-implementation-plan.md`
- `docs/visual/reports/native-hero-scale-evidence-upgrade-plan.md`
- `examples/engine/cross_space_dependency.hpp`
- `examples/engine/cross_space_dependency_visual_export.cpp`
- `visual/hero-visual-briefs.manifest.json`

## Required Behavior

- Export at least 500 paired observations.
- Preserve `provenance.native_export: true` and keep `provenance.synthetic`
  absent or false.
- Use the deterministic native paired-observation generator with coupled
  event-log and process-curve observations.
- Keep all metric/dependence computations native C++:
  - event-log edit distance
  - process-curve TWED distance
  - native distance matrices
  - MGC statistic through the existing METRIC correlation path
  - permutation significance if present and tractable
  - local dependence contribution
  - local densities and distance-profile alignment
- Export explicit bridge/edge evidence for high-contribution paired
  observations or paired-neighborhood references so the paired-space view has
  real bridge geometry.
- Keep composed preview payloads for each paired observation.

## Acceptance Commands

```bash
cmake --build build/core --target engine_cross_space_dependency_visual_export -- -j4
ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure
node visual/tools/check-visual-document.mjs docs/examples/assets/cross-space-dependency/metric.visual.json
node visual/tools/check-native-hero-evidence-scale.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
```

## Stop Rules

Stop and write the report if permutation cost makes the exporter impractical
without changing the algorithm contract, or if bridge evidence would have to be
invented in JavaScript.

## Report

Write `docs/visual/reports/cross-space-scale-exporter.md` with:

- changed files
- final paired record/relation/coordinate/property counts
- native MGC/dependence computations and any cost controls
- bridge evidence shape
- commands run and results
- remaining blocker if visual composition still needs human screenshot review
