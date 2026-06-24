# C++ Visual Export Core Workstream

## Purpose

Build and harden the reusable C++ export surface for `metric.visual.v1`.

This workstream is not a hero page task. It exists so native METRIC examples
can export records, relations, metric spaces, properties, coordinates, graphs,
timelines, views and diagnostics without each exporter inventing its own JSON
writer.

C++ computes evidence. The browser visual engine loads and renders it.

## Owner Scope

Primary write scope:

```text
visual/cpp/mtrc_visual.hpp
visual/cpp/export_example.cpp
docs/visual/agent-tasks/cpp-visual-export-core-workstream.md
docs/visual/native-exporter-work-queue.md
docs/visual/reports/native-exporter-quality-audit.md
```

Exporter refactors must be separate follow-up tasks with one exporter per
worker. Do not refactor all exporters in this workstream.

## Current Foundation

`visual/cpp/mtrc_visual.hpp` is the current header-only, dependency-free C++17
helper. It provides:

- JSON string/number/bool/array/object primitives
- `Document` builder for datasets, records, relations, spaces, properties,
  coordinates, graphs and diagnostics
- raw `view_json(...)` and `timeline_json(...)` append points
- raw append points for every top-level visual document section
- metadata slots for core document entries
- `write_file(...)` and `write_metric_visual_file(...)` convenience helpers

`visual/tools/check-cpp-export.mjs` compiles `visual/cpp/export_example.cpp`,
runs it, parses the emitted JSON, validates it as `metric.visual.v1`, and
builds a `VisualSpace`.

## Exporter Refactor Status

The six native exporter binaries are integrated, validate, and now share the
`mtrc::visual` writer foundation for common JSON/document/file writing.

| Slot | Source | Current state |
| --- | --- | --- |
| Condition monitoring | `examples/engine/condition_monitoring_visual_export.cpp` | migrated |
| Mixed records | `examples/engine/mixed_finite_records_visual_export.cpp` | migrated |
| Cross-space dependency | `examples/engine/cross_space_dependency_visual_export.cpp` | migrated |
| Finite dynamics | `examples/engine/finite_metric_dynamics_visual_export.cpp` | migrated |
| Mapping/dimensionality | `examples/engine/mapping_dimensionality_visual_export.cpp` | migrated |
| Relation matrix | `examples/engine/relation_matrix_visual_export.cpp` | migrated |

Domain-specific evidence may still be composed explicitly through
`visual::object(...)` and raw append methods. That is intentional when a generic
helper would hide algorithm-specific semantics.

## Rules For Future Exporters

- Preserve exported evidence semantics. Do not simplify records, relations,
  metadata, diagnostics, timelines or views to fit the helper.
- Add missing helper escape hatches before deleting exporter-local fields.
- Keep the helper header-only and C++17.
- Do not add dependencies.
- Do not move METRIC algorithmic computation into JavaScript or Python.
- Do not touch unrelated dirty files.
- Do not claim public hero readiness from exporter validation alone.

## Acceptance

A worker refactoring one exporter is accepted only when:

1. The exporter uses `mtrc::visual` for common JSON/document/file writing.
2. The emitted document still validates with:

   ```bash
   node visual/tools/check-visual-document.mjs <exported metric.visual.json>
   ```

3. The exporter test still passes through CTest:

   ```bash
   ctest --test-dir build/core -R '<exporter_name>_visual_(export|validate)' --output-on-failure
   ```

4. `node visual/tools/check-cpp-export.mjs` still passes.
5. `git diff --check` reports no whitespace errors for the touched files.

## Stop Conditions

Stop and report instead of patching if:

- preserving a current exported field requires a helper feature that does not
  exist yet;
- the exporter output changes in a way that affects public gallery evidence;
- a CMake file has unrelated local edits and cannot be safely touched;
- validation passes but the refactor removes evidence needed by a visual
  grammar.
