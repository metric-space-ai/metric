# Agent Task: Scale Mixed Records Native Evidence

## Owner Scope

You own only:

- `examples/engine/mixed_finite_records_visual_export.cpp`
- `examples/engine/mixed_finite_records_fixture.hpp`
- `examples/engine/mixed_finite_records.hpp` only if required for honest
  type-specific metric contributions
- `docs/examples/assets/mixed-records/metric.visual.json`
- optional report: `docs/visual/reports/mixed-records-scale-exporter.md`

Do not edit visual runtime, project pages, GRAE10, unrelated exporters, CMake
unless the existing target is broken. You are not alone in the worktree: do not
revert changes made by other agents, and adapt to current files.

## Objective

Turn `mixed-record-hero` from a small one-record-type preview into at least
2,000 native records across at least four honest record types. The result must
prove mixed finite records without collapsing every record into one generic
payload.

No JavaScript, Python or page code may compute distances, contributions,
clusters or embeddings.

## Required Inputs

Read:

- `docs/visual/visual-engine-masterplan.md`
- `docs/visual/metric-visual-engine-implementation-plan.md`
- `docs/visual/reports/native-hero-scale-evidence-upgrade-plan.md`
- `examples/engine/mixed_finite_records_fixture.hpp`
- `examples/engine/mixed_finite_records.hpp`
- `examples/engine/mixed_finite_records_visual_export.cpp`
- `visual/hero-visual-briefs.manifest.json`

## Required Behavior

- Export at least 2,000 records.
- Export at least four public `record_type` values backed by distinct payload
  families:
  - text/code records
  - histogram/spectrum records
  - process-curve records
  - numeric-vitals/vector records
- Preserve `provenance.native_export: true` and keep `provenance.synthetic`
  absent or false.
- Keep all metric computations native C++:
  - edit distance for text/code
  - Wasserstein/EMD or existing true metric for histograms/spectra
  - TWED or existing true metric for process curves
  - standardized Euclidean or existing true metric for numeric vitals
  - positive-weight composite metric
  - per-field contribution vectors
  - nearest-neighbor/cross-type edge selection
  - cluster/representative assignment if exported
  - metric-law diagnostics
- Export sparse cross-type nearest-neighbor graph evidence with contribution
  metadata. Do not export an enormous verbose all-pairs matrix unless the
  existing schema path and performance are intentional.
- Every record must have a type-specific preview payload.

## Acceptance Commands

```bash
cmake --build build/core --target engine_mixed_finite_records_visual_export -- -j4
ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure
node visual/tools/check-visual-document.mjs docs/examples/assets/mixed-records/metric.visual.json
node visual/tools/check-native-hero-evidence-scale.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
```

## Stop Rules

Stop and write the report if you can only satisfy the record-type gate by
renaming the current composed payload, by duplicating records without new
type-specific payloads, or by moving any metric computation to JavaScript.

## Report

Write `docs/visual/reports/mixed-records-scale-exporter.md` with:

- changed files
- final record/relation/type/coordinate/property counts
- exported public record types and payload families
- native metric computations and contribution evidence
- commands run and results
- remaining blocker if visual composition still needs human screenshot review
