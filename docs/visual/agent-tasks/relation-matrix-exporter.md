# Agent Task: Relation Matrix Native Visual Exporter

## Owner

One worker owns only:

- `examples/engine/relation_matrix_visual_export.cpp`

The parent coordinator owns CMake, tests, docs integration, and commits.

## Candidate Inputs

Read first and choose the most meaningful native source:

- `examples/engine/histogram_transport_space.cpp`
- `examples/engine/strings_edit_space.cpp`
- `examples/engine/process_curves_space.cpp`
- `examples/engine/hero_application_benchmarks.cpp`
- `visual/cpp/mtrc_visual.hpp`
- `docs/visual/metric-visual-v1-schema.md`

## Goal

Create a native C++ exporter that emits a readable `metric.visual.v1` relation
matrix foundation: records, dense or block-sampled metric relation, block/order
properties and neighborhood graph.

## Required Behavior

- stdout emits valid `metric.visual.v1` by default.
- `--export-dir <dir>` writes `<dir>/metric.visual.json`.
- Provenance identifies native C++ export and must not set
  `provenance.synthetic: true`.
- Include at least:
  - 128 or more records, unless the chosen real/native fixture is smaller and
    the limitation is documented in diagnostics
  - one finite metric relation appropriate for matrix rendering
  - categorical block/order property
  - sparse nearest-neighbor graph derived from the metric
  - coordinates only if they come from native evidence or a deterministic layout
    explicitly documented as visualization layout

## Constraints

- Do not add dependencies.
- Do not compute metric values in JavaScript.
- Do not edit CMake.
- Do not edit shared visual engine files.

## Acceptance

The coordinator must be able to run:

```bash
c++ -std=c++17 -I. -Ivisual/cpp examples/engine/relation_matrix_visual_export.cpp -o /tmp/relation_matrix_visual_export
/tmp/relation_matrix_visual_export --export-dir /tmp/relation-matrix-visual-export
node visual/tools/check-visual-document.mjs /tmp/relation-matrix-visual-export/metric.visual.json
```

