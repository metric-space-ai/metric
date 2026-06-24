# Native Visual Exporter Work Queue

This queue is the execution layer for the visual engine plan. It exists so
parallel agents can work on native evidence exporters without touching the same
files.

## Git Discipline

- Work happens on `codex/visual-engine-evidence-exporters`.
- Every integration point gets a commit before the next parallel wave.
- Workers own disjoint source files. The parent agent owns CMake, tests,
  project page integration, and final commits.
- Generated screenshots and runtime outputs are not committed.

## Current Integration Gates

```bash
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-views.mjs
node visual/tools/check-visual-document.mjs <exported metric.visual.json>
ctest --test-dir build/core -L 'metric_application_evidence|metric_phate_pipeline|metric_mnist|metric_visual_integrity|metric_benchmark_report' --output-on-failure
```

## Parallel Work Slots

| Slot | Owner writes | Status | Integration target |
| --- | --- | --- | --- |
| Mixed records exporter | `examples/engine/mixed_finite_records_visual_export.cpp` | in progress | `docs/examples/assets/mixed-records/metric.visual.json` |
| Cross-space exporter | `examples/engine/cross_space_dependency_visual_export.cpp` | in progress | `docs/examples/assets/cross-space-dependency/metric.visual.json` |
| Dynamics exporter | `examples/engine/finite_metric_dynamics_visual_export.cpp` | in progress | `docs/examples/assets/dynamics-noise/metric.visual.json` |
| Relation matrix exporter | not assigned | planned | `docs/examples/assets/relation-matrix/metric.visual.json` |
| Condition monitoring exporter | not assigned | planned | `docs/examples/assets/condition-monitoring/metric.visual.json` |
| Mapping/dimensionality exporter | not assigned | planned | `docs/examples/assets/mapping-dimensionality/metric.visual.json` |

## Acceptance For A Native Exporter

An exporter is integration-ready when:

- it compiles without adding runtime dependencies
- it emits `metric.visual.v1`
- `provenance.synthetic` is absent or false
- `node visual/tools/check-visual-document.mjs` accepts the exported JSON
- C++ computes all metric values, properties, diagnostics and coordinates
- JavaScript only validates, indexes and renders the exported result

An exporter is not a public hero yet. Public hero status requires a distinct
visual grammar, browser screenshot review, and the public gallery evidence gate.
