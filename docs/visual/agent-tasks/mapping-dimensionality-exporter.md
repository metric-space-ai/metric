# Agent Task: Mapping And Dimensionality Native Visual Exporter

## Owner

One worker owns only:

- `examples/engine/mapping_dimensionality_visual_export.cpp`

The parent coordinator owns CMake, tests, docs integration, generated assets,
gallery pages, and commits.

## Inputs

Read:

- `examples/engine/process_curve_phate_map.cpp`
- `examples/engine/process_curve_phate_gallery.cpp`
- `examples/engine/metric_space_mapping_pipeline.cpp`
- `examples/engine/phate_pipeline_builder.cpp`
- `visual/cpp/mtrc_visual.hpp`
- `docs/visual/metric-visual-v1-schema.md`

## Goal

Create a native C++ exporter that emits `metric.visual.v1` for a mapping and
dimensionality workflow. The evidence must show a finite metric space, one or
more mapped coordinate states, distortion or reconstruction properties, and
diagnostics grounded in the native mapping pipeline.

## Required Behavior

- stdout emits valid `metric.visual.v1` by default.
- `--export-dir <dir>` writes `<dir>/metric.visual.json`.
- Provenance identifies native C++ export and must not set
  `provenance.synthetic: true`.
- Include at least:
  - native records from an existing mapping example
  - one finite metric relation computed in C++
  - original/native coordinate or layout state where available
  - mapped 2D/3D coordinate states
  - scalar properties for mapping error, local distortion, or reconstruction
    residual where available
  - diagnostics showing which native mapping path produced the evidence

## Constraints

- Do not add dependencies.
- Do not train or compute mapping results in JavaScript or Python.
- Do not edit CMake.
- Do not edit shared visual engine files.
- Do not use the protected GRAE10 page as the implementation shortcut. GRAE10
  remains the visual reference, not a file to overwrite.
- Do not promote this to a public hero page. This task only creates native
  evidence.

## Acceptance

The coordinator must be able to run:

```bash
c++ -std=c++17 -I. -Ivisual/cpp examples/engine/mapping_dimensionality_visual_export.cpp -o /tmp/mapping_dimensionality_visual_export
/tmp/mapping_dimensionality_visual_export --export-dir /tmp/mapping-dimensionality-visual-export
node visual/tools/check-visual-document.mjs /tmp/mapping-dimensionality-visual-export/metric.visual.json
```
