# Agent Task: Scale Mapping/Dimensionality Native Evidence

## Owner Scope

You own only:

- `examples/engine/mapping_dimensionality_visual_export.cpp`
- helper code inside `examples/engine/` only if it is specific to this exporter
- `docs/examples/assets/mapping-dimensionality/metric.visual.json`
- optional report: `docs/visual/reports/mapping-dimensionality-scale-exporter.md`

Do not edit visual runtime, project pages, GRAE10, unrelated exporters, CMake
unless the existing target is broken. You are not alone in the worktree: do not
revert changes made by other agents, and adapt to current files.

## Objective

Turn `mapping-dimensionality-hero` from 15 toy records into at least 1,000
native records with source coordinate state, target coordinate state, mapping
diagnostics, residual/preservation-error evidence and record previews.

This is not a generic point cloud. It must export evidence for `MappingView`:
coordinate morph plus residual/error evidence.

## Required Inputs

Read:

- `docs/visual/visual-engine-masterplan.md`
- `docs/visual/metric-visual-engine-implementation-plan.md`
- `docs/visual/reports/native-hero-scale-evidence-upgrade-plan.md`
- `examples/engine/mapping_dimensionality_visual_export.cpp`
- `examples/engine/relation_matrix_visual_export.cpp`
- `visual/hero-visual-briefs.manifest.json`

## Required Behavior

- Export at least 1,000 records.
- Preserve `provenance.native_export: true` and keep `provenance.synthetic`
  absent or false.
- Replace or grow the tiny fixture using a deterministic native process-curve
  source with original time-series snippets and family labels. Prefer reusing
  the relation-matrix family/variant generator pattern if no real licensed
  process-curve corpus is available in the repo.
- Keep all computations native C++:
  - aligned process-curve metric values
  - source-space metric-law diagnostics
  - diffusion-coordinate target construction
  - parametric/diffusion coordinate mapping if present in the exporter
  - latent Euclidean relation
  - local mapping distortion
  - target residuals or preservation error
  - kNN graph and query diagnostics if exported
- Export a native coordinate-morph timeline such as source -> diffusion target
  -> parametric latent.
- Every record must keep original time-series payload and mapping diagnostics.

## Acceptance Commands

```bash
cmake --build build/core --target engine_mapping_dimensionality_visual_export -- -j4
ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure
node visual/tools/check-visual-document.mjs docs/examples/assets/mapping-dimensionality/metric.visual.json
node visual/tools/check-native-hero-evidence-scale.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
```

## Stop Rules

Stop and write the report if only duplicated toy curves are available, if
embedding/residual values would have to be computed in JavaScript, or if the
visual grammar would still be just a static point cloud.

## Report

Write `docs/visual/reports/mapping-dimensionality-scale-exporter.md` with:

- changed files
- final record/relation/coordinate/timeline/property counts
- native mapping computations and residual/preservation-error evidence
- commands run and results
- remaining blocker if visual composition still needs human screenshot review
