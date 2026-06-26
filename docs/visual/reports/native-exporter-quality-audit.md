# Native Exporter Quality Audit

Date: 2026-06-24
Branch: `codex/visual-engine-evidence-exporters`

This audit covers the native `metric.visual.v1` exporter wave. Exporters are
evidence foundations only. Public hero promotion still requires a distinct
visual grammar, browser screenshot review, and the public gallery evidence gate.

Current status note, 2026-06-26: this earlier audit was superseded by
`docs/visual/reports/native-exporter-quality-audit-2026-06-26-subagent.md` for
the newer exporter-contract checks. Those blockers have since been addressed:
Redif now has native non-synthetic provenance and a public
`metric.visual.json`, Process-Curve/Solver-Trace emit JSON to stdout by
default, and Relation-Matrix public evidence is byte-consistent with native
regeneration. The executable task files are
`docs/visual/agent-tasks/visual-exporter-cli-contract.md`,
`docs/visual/agent-tasks/redif-native-public-evidence.md` and
`docs/visual/agent-tasks/relation-matrix-reproducibility-audit.md`.

## Scope

| Exporter | Native source |
| --- | --- |
| Condition monitoring | `examples/engine/condition_monitoring_visual_export.cpp` |
| Mixed finite records | `examples/engine/mixed_finite_records_visual_export.cpp` |
| Cross-space dependency | `examples/engine/cross_space_dependency_visual_export.cpp` |
| Finite metric dynamics | `examples/engine/finite_metric_dynamics_visual_export.cpp` |
| Redif metric dynamics | `examples/engine/redif_metric_dynamics_visual_export.cpp` |
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
| Redif metric dynamics | 7 | 1 | 1 | 5 | 2 | 1 | false |
| Relation matrix | 130 | 1 | 1 | 1 | 7 | 1 | false |
| Mapping/dimensionality | 15 | 2 | 2 | 3 | 7 | 4 | false |

## Findings

No blocking issues were found in the original six-exporter foundation wave.
The later exporter-contract audit listed above added blocking follow-ups for
the newer Redif, Solver-Trace and Process-Curve paths. Those follow-ups are now
integrated; they remain normal validation gates, not accepted-hero promotion.

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

## Large Evidence Artifact Policy

Native `metric.visual.v1` evidence larger than GitHub's warning threshold must
not be treated as ordinary source churn. For production exports, use one of
these explicit routes:

- reduce or sample the checked-in public preview while keeping the full native
  export reproducible from C++;
- split the evidence into documented generated artifacts;
- publish the full export through CI/release artifacts;
- or move the large payload to Git LFS with the policy documented next to the
  exporter.

The checked-in mapping-dimensionality evidence is currently below the warning
threshold in this worktree but remains close enough to require this policy for
future native exports. Redif visual evidence is intentionally small, checked in
as public native evidence and validated through `redif_visual_export_validate`.

## Export Writer Reuse Audit

The exporter wave is evidence-valid and now shares the C++ writer foundation.

| Exporter | Current writer state |
| --- | --- |
| `condition_monitoring_visual_export.cpp` | Migrated: common JSON/document/file writing flows through `mtrc::visual`; exporter keeps domain-specific raw evidence composition |
| `mixed_finite_records_visual_export.cpp` | Migrated: common JSON/document/file writing flows through `mtrc::visual`; exporter keeps domain-specific raw evidence composition |
| `cross_space_dependency_visual_export.cpp` | Migrated: common JSON/document/file writing flows through `mtrc::visual`; exporter keeps domain-specific raw evidence composition |
| `finite_metric_dynamics_visual_export.cpp` | Migrated: common object/size/bool/array/root/event/file writing flows through `mtrc::visual`; exporter keeps domain-specific raw evidence composition |
| `redif_metric_dynamics_visual_export.cpp` | New: native Redif measure paths, entropy, and transport path lengths are computed in C++ and serialized through `mtrc::visual`; JavaScript validation checks evidence consumption only |
| `mapping_dimensionality_visual_export.cpp` | Migrated: common JSON/root/file writing flows through `mtrc::visual`; exporter keeps domain-specific raw evidence composition |
| `relation_matrix_visual_export.cpp` | Migrated: local quote/number/object/file stack removed; common writing flows through `mtrc::visual`; graph keeps raw source/target/value semantics |

The executable follow-up contract is:

```text
docs/visual/agent-tasks/cpp-visual-export-core-workstream.md
```
