# Agent Task: Scale External Process-Curve Native Evidence

## Owner Scope

You own only:

- `examples/engine/process_curve_external_visual_export.cpp`
- process-curve CSV fixture references under `examples/engine/assets/` only if
  the files already exist and are licensed for this repo
- `docs/examples/assets/process-curve-external/metric.visual.json`
- optional report: `docs/visual/reports/process-curve-external-scale-exporter.md`

Do not edit visual runtime, project pages, GRAE10, unrelated exporters, CMake
unless the existing target is broken. You are not alone in the worktree: do not
revert changes made by other agents, and adapt to current files.

## Objective

Turn `process-curve-external-hero` from a 64-record preview into at least 500
real source windows plus query/winner evidence, using native C++ aligned metric
and baseline computations. Query records must not be used as padding to fake
the source relation scale.

No JavaScript, Python or page code may compute aligned distances, vector
baseline winners, margins or mismatch flags.

## Required Inputs

Read:

- `docs/visual/visual-engine-masterplan.md`
- `docs/visual/metric-visual-engine-implementation-plan.md`
- `docs/visual/reports/native-hero-scale-evidence-upgrade-plan.md`
- `docs/visual/reports/process-curve-external-visual-exporter.md`
- `examples/engine/process_curve_external_visual_export.cpp`
- `visual/hero-visual-briefs.manifest.json`

## Required Behavior

- Export at least 500 real source windows in the source-source metric relation.
- Preserve `provenance.native_export: true` and keep `provenance.synthetic`
  absent or false.
- Use real UCR-derived process-curve assets already referenced by the exporter,
  or stop and report the exact missing licensed data if the repo does not
  contain enough source windows.
- Keep all computations native C++:
  - aligned curve metric distances
  - padded/vector baseline distances
  - nearest-neighbor winner selection
  - metric margin
  - correctness/mismatch flags
  - dense source relation
  - kNN graph
  - landmark coordinates
  - summary diagnostics
- If query records remain outside the source-source relation, export explicit
  query-to-source assignment relation/graph evidence.
- Every source and query record must keep original time-series snippets and
  source/window indices.

## Acceptance Commands

```bash
cmake --build build/core --target engine_process_curve_external_visual_export -- -j4
ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure
node visual/tools/check-visual-document.mjs docs/examples/assets/process-curve-external/metric.visual.json
node visual/tools/check-native-hero-evidence-scale.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
```

## Stop Rules

Stop and write the report if the repo does not contain enough real licensed
source windows. Do not generate synthetic replacement curves and do not count
query-only records as source relation scale.

## Report

Write `docs/visual/reports/process-curve-external-scale-exporter.md` with:

- changed files
- final source/query/relation/coordinate/property counts
- real data sources used or exact missing data blocker
- native metric/baseline computations
- commands run and results
- remaining blocker if visual composition still needs human screenshot review
