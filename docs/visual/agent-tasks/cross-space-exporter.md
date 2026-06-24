# Agent Task: Cross-Space Native Visual Exporter

## Owner

One worker owns only:

- `examples/engine/cross_space_dependency_visual_export.cpp`

The parent coordinator owns CMake, tests, docs integration, and commits.

## Inputs

Read:

- `examples/engine/cross_space_dependency.cpp`
- `examples/engine/cross_space_dependency.hpp`
- `visual/cpp/mtrc_visual.hpp`
- `docs/visual/metric-visual-v1-schema.md`

## Goal

Create a native C++ exporter that emits `metric.visual.v1` for paired
observations in two finite metric spaces.

## Required Behavior

- stdout emits valid `metric.visual.v1` by default.
- `--export-dir <dir>` writes `<dir>/metric.visual.json`.
- Provenance identifies native C++ export and must not set
  `provenance.synthetic: true`.
- Include at least:
  - paired records with stable IDs
  - two finite metric spaces
  - two metric relations or sampled relation edge sets
  - coordinates for both spaces
  - local dependence/contribution properties where available
  - global MGC/dependence diagnostics from the native workflow

## Constraints

- Do not add dependencies.
- Do not compute dependence in JavaScript.
- Do not edit CMake.
- Do not edit shared visual engine files.

## Acceptance

The coordinator must be able to run:

```bash
c++ -std=c++17 -I. -Ivisual/cpp examples/engine/cross_space_dependency_visual_export.cpp -o /tmp/cross_space_visual_export
/tmp/cross_space_visual_export --export-dir /tmp/cross-space-visual-export
node visual/tools/check-visual-document.mjs /tmp/cross-space-visual-export/metric.visual.json
```

