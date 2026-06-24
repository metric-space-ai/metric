# Native Exporter Quality Audit

Date: 2026-06-24
Branch: `codex/visual-engine-evidence-exporters`

This audit covers the native `metric.visual.v1` exporter wave. Exporters are
evidence foundations only. Public hero promotion still requires a distinct
visual grammar, browser screenshot review, and the public gallery evidence gate.

## Scope

| Exporter | Native source |
| --- | --- |
| Condition monitoring | `examples/engine/condition_monitoring_visual_export.cpp` |
| Mixed finite records | `examples/engine/mixed_finite_records_visual_export.cpp` |
| Cross-space dependency | `examples/engine/cross_space_dependency_visual_export.cpp` |
| Finite metric dynamics | `examples/engine/finite_metric_dynamics_visual_export.cpp` |
| Relation matrix | `examples/engine/relation_matrix_visual_export.cpp` |
| Mapping/dimensionality | `examples/engine/mapping_dimensionality_visual_export.cpp` |

## Validation

Build and CTest gate:

```bash
cmake -S . -B build/core
cmake --build build/core --target engine_condition_monitoring_visual_export engine_mapping_dimensionality_visual_export -- -j4
ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure
```

Result: 12/12 tests passed. The test pair for each exporter writes
`metric.visual.json` from the C++ binary and validates it with
`visual/tools/check-visual-document.mjs`.

Collected document validation:

```bash
node visual/tools/check-visual-document.mjs \
  build/core/examples/engine/visual_exports/condition_monitoring/metric.visual.json \
  build/core/examples/engine/visual_exports/mixed_finite_records/metric.visual.json \
  build/core/examples/engine/visual_exports/cross_space_dependency/metric.visual.json \
  build/core/examples/engine/visual_exports/finite_metric_dynamics/metric.visual.json \
  build/core/examples/engine/visual_exports/relation_matrix/metric.visual.json \
  build/core/examples/engine/visual_exports/mapping_dimensionality/metric.visual.json
```

Result: `ok:true`, `total:6`.

## Export Summary

| Exporter | Records | Spaces | Relations | Coordinates | Properties | Diagnostics | Synthetic |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| Condition monitoring | 15 | 1 | 2 | 2 | 10 | 2 | false |
| Mixed finite records | 20 | 1 | 1 | 2 | 8 | 10 | false |
| Cross-space dependency | 48 | 2 | 2 | 2 | 6 | 3 | false |
| Finite metric dynamics | 28 | 1 | 2 | 48 | 12 | 25 | false |
| Relation matrix | 130 | 1 | 1 | 1 | 7 | 1 | false |
| Mapping/dimensionality | 15 | 2 | 2 | 3 | 7 | 4 | false |

## Findings

No blocking issues were found in this exporter wave.

- All six exporters compile as C++17 in the existing build.
- All six exporters write `metric.visual.v1`.
- All six exporters validate and index through the JavaScript document checker.
- None of the six exported documents is marked as synthetic.
- JavaScript remains a validation/index/render layer; metric evidence is
  computed in C++.

Follow-up required before public hero promotion:

- Normalize provenance field names across exporters before public asset
  generation. Current exporters consistently avoid `provenance.synthetic:true`,
  but some use additional fields such as `synthetic_js:false`.
- Keep generated `metric.visual.json` artifacts out of source until the gallery
  asset policy is explicit.
- Do not promote an exporter to a public hero until its visual grammar is
  reviewed in-browser against the GRAE10 reference look.

## Export Writer Reuse Audit

The exporter wave is evidence-valid and now shares the C++ writer foundation.

| Exporter | Current writer state |
| --- | --- |
| `condition_monitoring_visual_export.cpp` | Migrated: common JSON/document/file writing flows through `mtrc::visual`; exporter keeps domain-specific raw evidence composition |
| `mixed_finite_records_visual_export.cpp` | Migrated: common JSON/document/file writing flows through `mtrc::visual`; exporter keeps domain-specific raw evidence composition |
| `cross_space_dependency_visual_export.cpp` | Migrated: common JSON/document/file writing flows through `mtrc::visual`; exporter keeps domain-specific raw evidence composition |
| `finite_metric_dynamics_visual_export.cpp` | Migrated: common object/size/bool/array/root/event/file writing flows through `mtrc::visual`; exporter keeps domain-specific raw evidence composition |
| `mapping_dimensionality_visual_export.cpp` | Migrated: common JSON/root/file writing flows through `mtrc::visual`; exporter keeps domain-specific raw evidence composition |
| `relation_matrix_visual_export.cpp` | Migrated: local quote/number/object/file stack removed; common writing flows through `mtrc::visual`; graph keeps raw source/target/value semantics |

The executable follow-up contract is:

```text
docs/visual/agent-tasks/cpp-visual-export-core-workstream.md
```
