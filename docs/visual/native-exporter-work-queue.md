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
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-visual-performance-large-scenes.mjs
node visual/tools/check-visual-document.mjs <exported metric.visual.json>
ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure
ctest --test-dir build/core -L 'metric_application_evidence|metric_phate_pipeline|metric_mnist|metric_visual_integrity|metric_benchmark_report' --output-on-failure
```

## Parallel Work Slots

| Slot | Owner writes | Status | Integration target |
| --- | --- | --- | --- |
| Mixed records exporter | `examples/engine/mixed_finite_records_visual_export.cpp` | integrated in CMake/CTest | `docs/examples/assets/mixed-records/metric.visual.json` |
| Cross-space exporter | `examples/engine/cross_space_dependency_visual_export.cpp` | integrated in CMake/CTest | `docs/examples/assets/cross-space-dependency/metric.visual.json` |
| Dynamics exporter | `examples/engine/finite_metric_dynamics_visual_export.cpp` | integrated in CMake/CTest | `docs/examples/assets/dynamics-noise/metric.visual.json` |
| Relation matrix exporter | `examples/engine/relation_matrix_visual_export.cpp` | integrated in CMake/CTest | `docs/examples/assets/relation-matrix/metric.visual.json` |
| Condition monitoring exporter | `examples/engine/condition_monitoring_visual_export.cpp` | integrated in CMake/CTest | `docs/examples/assets/condition-monitoring/metric.visual.json` |
| Mapping/dimensionality exporter | `examples/engine/mapping_dimensionality_visual_export.cpp` | integrated in CMake/CTest | `docs/examples/assets/mapping-dimensionality/metric.visual.json` |
| Process-curve external exporter | `examples/engine/process_curve_external_visual_export.cpp` | assigned | `docs/examples/assets/process-curve-external/metric.visual.json` |

## Integrated Exporter Tests

The current branch adds native exporter binaries and validation tests:

```bash
ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure
```

The test pair for each exporter first writes `metric.visual.json` from the C++
binary and then validates that document through
`visual/tools/check-visual-document.mjs`. Passing this gate means the exporter is
native, schema-valid, and indexable. It does not mean the corresponding public
hero visualization is finished.

Integrated native exporter coverage now includes mixed records, cross-space
dependency, finite metric dynamics, relation matrix, condition monitoring, and
mapping/dimensionality evidence.

The next exporter slot targets the existing UCR process-curve gallery evidence
under `examples/engine/assets/` and `docs/examples/assets/process-curve-external/`.
It is the next real public hero candidate because the records, query outcomes
and baseline comparison already exist outside the browser renderer.

## C++ Writer Core

Native exporter validation proves that a document is schema-valid and indexable.
It does not prove that the native export surface is production-quality.

The reusable writer core is tracked in:

```text
visual/cpp/mtrc_visual.hpp
docs/visual/agent-tasks/cpp-visual-export-core-workstream.md
```

Current status:

- `visual/cpp/mtrc_visual.hpp` provides the header-only C++17 writer foundation.
- `visual/tools/check-cpp-export.mjs` validates the standalone C++ helper path.
- All six native visual exporters now route common JSON/document/file writing
  through `mtrc::visual`.
- Exporters may still use raw `visual::object(...)` composition for
  domain-specific evidence shapes that should not be flattened into generic
  helper methods.

Future exporter changes should keep one exporter per worker and disjoint write
scopes.

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
