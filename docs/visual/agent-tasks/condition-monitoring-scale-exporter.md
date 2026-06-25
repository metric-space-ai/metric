# Agent Task: Scale Condition Monitoring Native Evidence

## Owner Scope

You own only:

- `examples/engine/condition_monitoring_visual_export.cpp`
- `docs/examples/assets/condition-monitoring/metric.visual.json`
- optional report: `docs/visual/reports/condition-monitoring-scale-exporter.md`

Do not edit visual runtime, project pages, GRAE10, unrelated exporters, CMake
unless the existing target is broken. You are not alone in the worktree: do not
revert changes made by other agents, and adapt to current files.

## Objective

Turn `condition-monitoring-hero` from a 15-record native preview into at least
500 native process-window records while keeping the same finite metric-space
meaning: C++ computes process-window relations, density/anomaly properties,
trajectory state, metric diagnostics and time-series preview payloads.

No JavaScript, Python or page code may compute METRIC values.

## Required Inputs

Read:

- `docs/visual/visual-engine-masterplan.md`
- `docs/visual/metric-visual-engine-implementation-plan.md`
- `docs/visual/reports/native-hero-scale-evidence-upgrade-plan.md`
- `examples/engine/condition_monitoring.cpp`
- `examples/engine/condition_monitoring_visual_export.cpp`
- `visual/hero-visual-briefs.manifest.json`

## Required Behavior

- Export at least 500 records.
- Preserve `provenance.native_export: true` and keep `provenance.synthetic`
  absent or false.
- Generate ordered process windows across healthy baseline, slow drift, abrupt
  fault, recovery and recurring signature regimes.
- Keep all metric computations native C++:
  - TWED process-window distances
  - native metric relation or declared sparse relation
  - transition/path relation
  - nearest healthy/catalog scores
  - density or entropy property
  - anomaly/severity scores
  - outlier flags
  - landmark/trajectory coordinates
  - metric-law diagnostics
- Every record must keep a usable original time-series preview payload with
  sample rate, truth regime and run phase.
- Add or grow a timeline/state descriptor if the exporter already has enough
  evidence to express healthy -> drift -> fault -> recovery progression.

## Acceptance Commands

```bash
cmake --build build/core --target engine_condition_monitoring_visual_export -- -j4
ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure
node visual/tools/check-visual-document.mjs docs/examples/assets/condition-monitoring/metric.visual.json
node visual/tools/check-native-hero-evidence-scale.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
```

## Stop Rules

Stop and write the report if scaling would require browser-side metric
computation, fake records, duplicated query padding, or lowering the visual
brief threshold.

## Report

Write `docs/visual/reports/condition-monitoring-scale-exporter.md` with:

- changed files
- final record/relation/coordinate/property counts
- which C++ computations produce each exported evidence family
- commands run and results
- remaining blocker if visual composition still needs human screenshot review
